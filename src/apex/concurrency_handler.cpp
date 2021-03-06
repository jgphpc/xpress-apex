//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#ifdef APEX_HAVE_HPX3
#include <hpx/config.hpp>
#endif

#include "concurrency_handler.hpp"
#include "apex.hpp"
#include "apex_api.hpp"
#include "apex_policies.hpp"
#include "thread_instance.hpp"
#include <iostream>
#include <map>
#include <iterator>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include "utils.hpp"

#ifdef APEX_HAVE_BFD
#include "address_resolution.hpp"
#endif

#ifdef APEX_HAVE_TAU
#define PROFILING_ON
#define TAU_DOT_H_LESS_HEADERS
#include <TAU.h>
#endif

#define MAX_FUNCTIONS_IN_CHART 5

using namespace std;

std::vector<std::mutex*> _per_thread_mutex;

namespace apex {

concurrency_handler::concurrency_handler (void) : handler() {
  _init();
}

concurrency_handler::concurrency_handler (int option) : handler(), _option(option) {
  _init();
}

concurrency_handler::concurrency_handler (unsigned int period) : handler(period) {
  _init();
}

concurrency_handler::concurrency_handler (unsigned int period, int option) : handler(period), _option(option) {
  _init();
}

bool concurrency_handler::_handler(void) {
  if (!_handler_initialized) {
      initialize_worker_thread_for_TAU();
      _handler_initialized = true;
  }
  if (_terminate) return true;
  apex* inst = apex::instance();
  if (inst == nullptr) return false; // running after finalization!
#ifdef APEX_HAVE_TAU
  if (apex_options::use_tau()) {
    TAU_START("concurrency_handler::_handler");
  }
#endif
  //cout << "HANDLER: " << endl;
  map<task_identifier, unsigned int> *counts = new(map<task_identifier, unsigned int>);
  stack<task_identifier>* tmp;
//  std::mutex* mut;
  for (unsigned int i = 0 ; i < _event_stack.size() ; i++) {
    if (_option > 1 && !thread_instance::map_id_to_worker(i)) {
      continue;
    }
    if (inst != nullptr && inst->get_state(i) == APEX_THROTTLED) { continue; }
    tmp = get_event_stack(i);
    task_identifier func;
    if (tmp != nullptr && tmp->size() > 0) {
      _per_thread_mutex[i]->lock();
      if (tmp->size() > 0) {
        func = tmp->top();
      } else {
        _per_thread_mutex[i]->unlock();
        continue;
      }
      _per_thread_mutex[i]->unlock();
      _function_mutex.lock();
      _functions.insert(func);
      _function_mutex.unlock();
      if (counts->find(func) == counts->end()) {
        (*counts)[func] = 1;
      } else {
        (*counts)[func] = (*counts)[func] + 1;
      }
    }
  }
  _states.push_back(counts);
  _thread_cap_samples.push_back(get_thread_cap());
  // TODO: FIXME multiple tuning sessions
  //for(auto param : get_tunable_params()) {
  //  _tunable_param_samples[param.first].push_back(*param.second);
  //}
  int power = current_power_high();
  _power_samples.push_back(power);
#ifdef APEX_HAVE_TAU
  if (apex_options::use_tau()) {
    TAU_STOP("concurrency_handler::_handler");
  }
#endif
  return true;
}

void concurrency_handler::_init(void) {
  add_thread(0);
  run();
  return;
}

bool concurrency_handler::on_start(task_identifier *id) {
  if (!_terminate) {
    int i = thread_instance::get_id();
    stack<task_identifier>* my_stack = get_event_stack(i);
    _per_thread_mutex[i]->lock();
    my_stack->push(*id);
    _per_thread_mutex[i]->unlock();
    return true;
  } else { 
    return false; 
  }
}

bool concurrency_handler::on_resume(task_identifier * id) {
  if (!_terminate) {
    int i = thread_instance::get_id();
    stack<task_identifier>* my_stack = get_event_stack(i);
    _per_thread_mutex[i]->lock();
    my_stack->push(*id);
    _per_thread_mutex[i]->unlock();
    return true;
  } else { 
    return false; 
  }
}

void concurrency_handler::on_stop(std::shared_ptr<profiler> &p) {
  if (!_terminate) {
    int i = thread_instance::get_id();
    stack<task_identifier>* my_stack = get_event_stack(i);
    _per_thread_mutex[i]->lock();
    if (!my_stack->empty()) {
      my_stack->pop();
    }
    _per_thread_mutex[i]->unlock();
  }
  APEX_UNUSED(p);
}

void concurrency_handler::on_yield(std::shared_ptr<profiler> &p) {
    on_stop(p);
}

void concurrency_handler::on_new_thread(new_thread_event_data &data) {
  if (!_terminate) {
        add_thread(data.thread_id);
  }
}

void concurrency_handler::on_exit_thread(event_data &data) {
  APEX_UNUSED(data);
  _terminate = true; // because there are crashes 
}

void concurrency_handler::on_shutdown(shutdown_event_data &data) {
    cancel();
    output_samples(data.node_id);
}

inline stack<task_identifier>* concurrency_handler::get_event_stack(unsigned int tid) {
  stack<task_identifier>* tmp;
  // it's possible we could get a "start" event without a "new thread" event.
  if (_event_stack.size() <= tid) {
    add_thread(tid);
  }
  tmp = this->_event_stack[tid];
  return tmp;
}

inline void concurrency_handler::add_thread(unsigned int tid) {
  _vector_mutex.lock();
  while(_event_stack.size() <= tid) {
    _event_stack.push_back(new stack<task_identifier>);
    _per_thread_mutex.push_back(new std::mutex());
  }
  _vector_mutex.unlock();
}

bool sort_functions(pair<task_identifier,int> first, pair<task_identifier,int> second) {
  if (first.second > second.second)
    return true;
  return false;
}

void concurrency_handler::output_samples(int node_id) {
  //cout << _states.size() << " samples seen:" << endl;
  ofstream myfile;
  stringstream datname;
  datname << "concurrency." << node_id << ".dat";
  myfile.open(datname.str().c_str());
  _function_mutex.lock();
  // limit ourselves to N functions.
  map<task_identifier, int> func_count;
  // initialize the map
  for (set<task_identifier>::iterator it=_functions.begin(); it!=_functions.end(); ++it) {
    func_count[*it] = 0;
  }
  // count all function instances
  for (unsigned int i = 0 ; i < _states.size() ; i++) {
    for (set<task_identifier>::iterator it=_functions.begin(); it!=_functions.end(); ++it) {
      if (_states[i]->find(*it) == _states[i]->end()) {
        continue;
      } else {
        func_count[*it] = func_count[*it] + (*(_states[i]))[*it];
      }
    }
  }
  // sort the map
  vector<pair<task_identifier,int> > my_vec(func_count.begin(), func_count.end());
  sort(my_vec.begin(),my_vec.end(),&sort_functions);
  set<task_identifier> top_x;
  for (vector<pair<task_identifier, int> >::iterator it=my_vec.begin(); it!=my_vec.end(); ++it) {
    //if (top_x.size() < 15 && (*it).first != "APEX THREAD MAIN")
    if (top_x.size() < MAX_FUNCTIONS_IN_CHART)
      top_x.insert((*it).first);
  }

  // output the header
  myfile << "\"period\"\t\"thread cap\"\t\"power\"\t";
  // output tunable parameter names
  for(auto param : _tunable_param_samples) {
    myfile << "\"" << param.first << "\"\t";
  }
  for (set<task_identifier>::iterator it=_functions.begin(); it!=_functions.end(); ++it) {
    if (top_x.find(*it) != top_x.end()) {
      //string* tmp = demangle(*it);
      task_identifier tmp_id = *it;
      string tmp = tmp_id.get_name();
#ifdef APEX_HAVE_BFD
      std::size_t pos = tmp.find("UNRESOLVED ADDR ");
      if (pos != string::npos) {
        string trimmed = tmp.substr(pos+16);
        uintptr_t function_address = std::stoul(trimmed, nullptr, 16);
        string * tmp2 = lookup_address(function_address, true);
        pos = tmp2->find(" [{");
        if (pos != string::npos) {
            trimmed = tmp2->substr(0, pos);
            myfile << "\"" << trimmed << "\"\t";
        } else {
            myfile << "\"" << *tmp2 << "\"\t";
        }
        delete (tmp2);
      } else {
        myfile << "\"" << tmp << "\"\t";
      }
#else
      myfile << "\"" << tmp << "\"\t";
#endif
    }
  }
  myfile << "\"other\"" << endl;

  size_t max_Y = 0;
  double max_Power = 0.0;
  size_t max_X = _states.size();
  int num_params = _tunable_param_samples.size();
  for (size_t i = 0 ; i < max_X ; i++) {
    myfile << i << "\t";
    myfile << _thread_cap_samples[i] << "\t";
    myfile << _power_samples[i] << "\t";
    for(auto param : _tunable_param_samples) {
      myfile << param.second[i] << "\t";
      if(param.second[i] > max_Power) max_Power = param.second[i];
    }
    unsigned int tmp_max = 0;
    int other = 0;
    for (set<task_identifier>::iterator it=_functions.begin(); it!=_functions.end(); ++it) {
      // this is the idle event.
      //if (*it == "APEX THREAD MAIN")
        //continue;
      int value = 0;
      // did we see this timer during this sample?
      if (_states[i]->find(*it) != _states[i]->end()) {
        value = (*(_states[i]))[*it];
      }
      // is this timer in the top X?
      if (top_x.find(*it) == top_x.end()) {
        other = other + value;
      } else {
        myfile << (*(_states[i]))[*it] << "\t";
        tmp_max += (*(_states[i]))[*it];
      }
    }
    myfile << other << "\t" << endl;
    tmp_max += other;
    if (tmp_max > max_Y) max_Y = tmp_max;
    if ((size_t)(_thread_cap_samples[i]) > max_Y) max_Y = _thread_cap_samples[i];
    if (_power_samples[i] > max_Power) max_Power = _power_samples[i];
  }
  _function_mutex.unlock();
  myfile.close();

  if (max_Power == 0.0) max_Power = 100;
  stringstream plotname;
  plotname << "concurrency." << node_id << ".gnuplot";
  myfile.open(plotname.str().c_str());
  myfile << "everyNth(col) = (int(column(col))%" << (int)(max_X/10) << "==0)?stringcolumn(1):\"\";" << endl;
  myfile << "set key outside bottom center invert box" << endl;
  myfile << "set xtics auto" << endl;
  myfile << "set ytics 4" << endl;
  myfile << "set y2tics auto" << endl;
  myfile << "set xrange[0:" << max_X << "]" << endl;
  myfile << "set yrange[0:" << max_Y << "]" << endl;
  myfile << "set y2range[0:" << max_Power << "]" << endl;
  myfile << "set xlabel \"Time\"" << endl;
  myfile << "set ylabel \"Concurrency\"" << endl;
  myfile << "set y2label \"Power\"" << endl;
  myfile << "# Select histogram data" << endl;
  myfile << "set style data histogram" << endl;
  myfile << "# Give the bars a plain fill pattern, and draw a solid line around them." << endl;
  myfile << "set style fill solid border" << endl;
  myfile << "set style histogram rowstacked" << endl;
  myfile << "set boxwidth 1.0 relative" << endl;
  myfile << "set palette rgb 33,13,10" << endl;
  myfile << "unset colorbox" << endl;
  myfile << "set key noenhanced" << endl; // this allows underscores in names
  myfile << "plot for [COL=" << (4+num_params) << ":" << top_x.size()+num_params+4;
  myfile << "] '" << datname.str().c_str();
  myfile << "' using COL:xticlabel(everyNth(1)) palette frac (COL-" << (3+num_params) << ")/" << top_x.size()+1;
  myfile << ". title columnheader axes x1y1, '"  << datname.str().c_str();
  myfile << "' using 2 with lines linecolor rgb \"red\" axes x1y1 title columnheader, '" << datname.str().c_str();
  myfile << "' using 3 with lines linecolor rgb \"black\" axes x1y2 title columnheader,";
  for(int p = 0; p < num_params; ++p) {
    myfile << "'" << datname.str().c_str() << "' using " << (4+p) << " with linespoints axes x1y2 title columnheader,";
  }
  myfile << endl;
  myfile.close();
}

}
