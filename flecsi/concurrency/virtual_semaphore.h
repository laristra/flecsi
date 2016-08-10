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

#ifndef flecsi_virtual_semaphore_h
#define flecsi_virtual_semaphore_h

#include <mutex>
#include <atomic>
#include <condition_variable>

/*!
 * \file virtual_semaphore.h
 * \authors nickm
 * \date Initial file creation: May 12, 2016
 */

namespace flecsi
{
  
  class virtual_semaphore{
  public:
    using lock_t = std::unique_lock<std::mutex>;

    virtual_semaphore(int count=0)
    : count_(count),
    max_count_(0){
      done_ = false;
    }
    
    virtual_semaphore(int count, int max_count)
    : count_(count),
    max_count_(max_count){
      done_ = false;
    }
    
    ~virtual_semaphore(){}
    
    bool acquire(){
      if(done_){
        return false;
      }

      lock_t lock(mutex_);
      
      while(count_ <= 0){
        cond_.wait(lock);
        if(done_){
          return false;
        }
      }
      
      --count_;
      
      return true;
    }
    
    bool try_acquire(){
      if(done_){
        return false;
      }

      lock_t lock(mutex_);

      if(count_ > 0){
        --count_;
        return true;
      }

      return false;
    }
    
    void release(){
      lock_t lock(mutex_);

      if(max_count_ == 0 || count_ < max_count_){
        ++count_;
      }

      cond_.notify_one();
    }

    void interrupt(){
      done_ = true;
      cond_.notify_all();
    }
    
    virtual_semaphore& operator=(const virtual_semaphore&) = delete;
    
    virtual_semaphore(const virtual_semaphore&) = delete;

  private:
    std::mutex mutex_;
    std::condition_variable cond_;

    int count_;
    int max_count_;
    std::atomic_bool done_;
  };

} // namespace flecsi

#endif // flecsi_virtual_semaphore_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
