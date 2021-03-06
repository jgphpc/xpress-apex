//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "apex_types.h"
#include <functional>
#include <string>

namespace apex {

class task_identifier {
public:
  apex_function_address address;
  std::string name;
  std::string _resolved_name;
  bool has_name;
  task_identifier(void) : 
      address(0L), name(""), _resolved_name(""), has_name(false) {};
  task_identifier(apex_function_address a) : 
      address(a), name(""), _resolved_name(""), has_name(false) {};
  task_identifier(std::string n) : 
      address(0L), name(n), _resolved_name(""), has_name(true) {};
	  /*
  task_identifier(profiler * p) : 
      address(0L), name(""), _resolved_name("") {
      if (p->have_name) {                                         
          name = *p->timer_name;
          has_name = true;
      } else {                                                         
          address = p->action_address;
          has_name = false;
      }            
  }
  */
  std::string get_name();
  ~task_identifier() { }
  // requried for using this class as a key in an unordered map.
  // the hash function is defined below.
  bool operator==(const task_identifier &other) const { 
    return (address == other.address && name.compare(other.name) == 0);
  }
  // required for using this class as a key in a set
  bool operator< (const task_identifier &right) const {
    if (!has_name) {
      if (!right.has_name) {
          // if both have an address, return the lower address
          return (address < right.address);
      } else {
          // if left has an address and right doesn't, return true
          return true;
      }
    } else {
      if (right.has_name) {
          // if both have a name, return the lower name
          return (name < right.name);
      }
    }
    // if right has an address and left doesn't, return false
    // (also the default)
    return false;
  }
};

}

/* This is the hash function for the task_identifier class */
namespace std {

  template <>
  struct hash<apex::task_identifier>
  {
    std::size_t operator()(const apex::task_identifier& k) const
    {
      std::size_t h1 = std::hash<int>()(k.address);
      std::size_t h2 = std::hash<std::string>()(k.name);
      return h1 ^ (h2 << 1);; // instead of boost::hash_combine
    }
  };

}


