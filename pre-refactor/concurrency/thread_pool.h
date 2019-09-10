/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 12, 2016
//----------------------------------------------------------------------------//

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <flecsi/concurrency/virtual_semaphore.h>

namespace flecsi {

//------------------------------------------------------------------------//
//! This class provides a thread pool mechanism by which callable objects
//! and associated arguments can be executed by a pool of worker threads.
//!
//! @ingroup concurrency
//------------------------------------------------------------------------//
class thread_pool
{
public:
  //! signature of internally queued function
  using function_t = std::function<void(void)>;

  //---------------------------------------------------------------------//
  //! Constructor
  //---------------------------------------------------------------------//
  thread_pool() {
    done_ = false;
  }

  //---------------------------------------------------------------------//
  //! Destructor
  //---------------------------------------------------------------------//
  ~thread_pool() {
    join();
  }

  //---------------------------------------------------------------------//
  //! Internal run method. Do not call directly.
  //---------------------------------------------------------------------//
  void run_() {
    for(;;) {
      sem_.acquire();

      mutex_.lock();

      if(done_) {
        mutex_.unlock();
        return;
      }

      auto f = queue_.front();
      queue_.pop();
      mutex_.unlock();
      f();
    }
  }

  //---------------------------------------------------------------------//
  //! Queue a callable object and associated arguments to the thread pool.
  //---------------------------------------------------------------------//
  template<typename FT, typename... ARGS>
  void queue(FT f, ARGS... args) {
    mutex_.lock();
    queue_.emplace(std::bind(f, std::forward<ARGS>(args)...));
    mutex_.unlock();
    sem_.release();
  }

  //---------------------------------------------------------------------//
  //! The constructor does not start the thread pool until this method is
  //! called.
  //!
  //! @param num_threads Number of workers threads
  //---------------------------------------------------------------------//
  void start(size_t num_threads) {
    assert(threads_.empty() && "thread pool already started");

    for(size_t i = 0; i < num_threads; ++i) {
      auto t = new std::thread(&thread_pool::run_, this);
      threads_.push_back(t);
    }
  }

  //---------------------------------------------------------------------//
  //! Interrupt the thread pool and wait for all threads to finish.
  //---------------------------------------------------------------------//
  void join() {
    if(done_) {
      return;
    }

    done_ = true;
    sem_.interrupt();

    for(auto t : threads_) {
      t->join();
      delete t;
    }
  }

  //---------------------------------------------------------------------//
  //! Return the number of worker threads
  //---------------------------------------------------------------------//
  size_t num_threads() const {
    return threads_.size();
  }

private:
  std::mutex mutex_;
  std::queue<function_t> queue_;
  std::vector<std::thread *> threads_;
  virtual_semaphore sem_;
  std::atomic_bool done_;
};

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
