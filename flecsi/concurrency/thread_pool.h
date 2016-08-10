/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_thread_pool_h
#define flecsi_thread_pool_h

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>

#include "flecsi/concurrency/virtual_semaphore.h"

/*!
 * \file thread_pool.h
 * \authors nickm
 * \date Initial file creation: May 12, 2016
 */

namespace flecsi
{
  
  class thread_pool{
  public:
    using function_t = std::function<void(void)>;

    thread_pool(){
      done_ = false;
    }

    ~thread_pool(){
      join();
    }

    void run_(){
      for(;;){
        sem_.acquire();

        mutex_.lock();

        if(done_){
          mutex_.unlock();
          return;
        }

        auto f = queue_.front();
        queue_.pop();
        mutex_.unlock();
        f();
      }
    }

    template <typename FT, typename... ARGS>
    void queue(FT f, ARGS... args){
      mutex_.lock();
      queue_.emplace(std::bind(f, std::forward<ARGS>(args)...));
      mutex_.unlock();
      sem_.release();
    }

    void start(size_t num_threads){
      assert(threads_.empty() && "thread pool already started");

      for(size_t i = 0; i < num_threads; ++i){
        auto t = new std::thread(&thread_pool::run_, this);
        threads_.push_back(t);
      }
    }

    void join(){
      if(done_){
        return;
      }

      done_ = true;
      sem_.interrupt();

      for(auto t : threads_){
        t->join();
        delete t;
      }
    }

    size_t num_threads() const{
      return threads_.size();
    }

  private:
    std::mutex mutex_;
    std::queue<function_t> queue_;
    std::vector<std::thread*> threads_;
    virtual_semaphore sem_;
    std::atomic_bool done_;    
  };

} // namespace flecsi

#endif // flecsi_thread_pool_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
