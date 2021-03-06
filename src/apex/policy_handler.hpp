//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifdef APEX_HAVE_HPX3
#include <hpx/hpx.hpp>
#include <hpx/util/interval_timer.hpp>
#endif

#include "apex_types.h"
#include "handler.hpp"
#include "event_listener.hpp"
#include <stack>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <functional>
#include <chrono>
#include <memory>
#include <array>

#if __cplusplus > 201701L 
#include <shared_mutex>
#elif __cplusplus > 201402L
#include <shared_lock>
#else
#include <mutex>
#endif

#ifdef SIGEV_THREAD_ID
#ifndef sigev_notify_thread_id
#define sigev_notify_thread_id _sigev_un._tid
#endif /* ifndef sigev_notify_thread_id */
#endif /* ifdef SIGEV_THREAD_ID */

namespace apex
{

class policy_instance
{
public:
    int id;
    std::function<bool(apex_context const&)> func;
    policy_instance(int id_, std::function<bool(apex_context const&)> func_) : id(id_),
        func(func_) {};
};

class policy_handler : public handler, public event_listener
{
private:
#if __cplusplus > 201701L 
    typedef std::shared_mutex mutex_type;
#elif __cplusplus > 201402L
    typedef std::shared_lock mutex_type;
#else
    typedef std::mutex mutex_type;
#endif

    void _init(void);
    std::list<std::shared_ptr<policy_instance> > startup_policies;
    std::list<std::shared_ptr<policy_instance> > shutdown_policies;
    std::list<std::shared_ptr<policy_instance> > new_node_policies;
    std::list<std::shared_ptr<policy_instance> > new_thread_policies;
    std::list<std::shared_ptr<policy_instance> > exit_thread_policies;
    std::list<std::shared_ptr<policy_instance> > start_event_policies;
    std::list<std::shared_ptr<policy_instance> > stop_event_policies;
    std::list<std::shared_ptr<policy_instance> > yield_event_policies;
    std::list<std::shared_ptr<policy_instance> > resume_event_policies;
    std::list<std::shared_ptr<policy_instance> > sample_value_policies;
    std::list<std::shared_ptr<policy_instance> > periodic_policies;
    std::array<std::list<std::shared_ptr<policy_instance> >,APEX_MAX_EVENTS > custom_event_policies;
    mutex_type startup_mutex;
    mutex_type shutdown_mutex;
    mutex_type new_node_mutex;
    mutex_type new_thread_mutex;
    mutex_type exit_thread_mutex;
    mutex_type start_event_mutex;
    mutex_type stop_event_mutex;
    mutex_type yield_event_mutex;
    mutex_type resume_event_mutex;
    mutex_type sample_value_mutex;
    mutex_type custom_event_mutex;
    mutex_type periodic_mutex;
    void call_policies(
        const std::list<std::shared_ptr<policy_instance> > & policies,
        event_data &event_data);
#ifdef APEX_HAVE_HPX3
    hpx::util::interval_timer hpx_timer;
#endif
public:
    policy_handler (void);
/*
    template<Rep, Period>
    policy_handler (std::chrono::duration<Rep, Period> const& period);
*/
    policy_handler(uint64_t period_microseconds);
    ~policy_handler (void) { };
    void on_startup(startup_event_data &data);
    void on_shutdown(shutdown_event_data &data);
    void on_new_node(node_event_data &data);
    void on_new_thread(new_thread_event_data &data);
    void on_exit_thread(event_data &data);
    bool on_start(task_identifier *id);
    void on_stop(std::shared_ptr<profiler> &p);
    void on_yield(std::shared_ptr<profiler> &p);
    bool on_resume(task_identifier * id);
    void on_new_task(task_identifier * id, uint64_t task_id)
       { APEX_UNUSED(id); APEX_UNUSED(task_id); };
    void on_sample_value(sample_value_event_data &data);
    void on_custom_event(custom_event_data &data);
    void on_periodic(periodic_event_data &data);
    void on_send(message_event_data &data) { APEX_UNUSED(data); };
    void on_recv(message_event_data &data) { APEX_UNUSED(data); };

    int register_policy(const apex_event_type & when,
                        std::function<int(apex_context const&)> f);
    int deregister_policy(apex_policy_handle * handle);
    bool _handler(void);
    void _reset(void);
};

}

