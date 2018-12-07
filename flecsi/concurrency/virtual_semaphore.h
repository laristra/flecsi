/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 12, 2016
//----------------------------------------------------------------------------//

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace flecsi {

//------------------------------------------------------------------------//
//! The virtual semaphore class provides a semaphore-like interface
//! implemented in terms of condition variables and mutexes. Unlike real
//! semaphores which consume system resources, this class can be
//! instantiated indefinitely.
//!
//! @ingroup concurrency
//------------------------------------------------------------------------//
class virtual_semaphore
{
public:
  using lock_t = std::unique_lock<std::mutex>;

  //---------------------------------------------------------------------//
  //! Constructor
  //!
  //! @param count initialize with count
  //---------------------------------------------------------------------//
  virtual_semaphore(int count = 0) : count_(count), max_count_(0) {
    done_ = false;
  }

  //---------------------------------------------------------------------//
  //! Constructor. Initialize with count and max count.
  //---------------------------------------------------------------------//
  virtual_semaphore(int count, int max_count)
    : count_(count), max_count_(max_count) {
    done_ = false;
  }

  //---------------------------------------------------------------------//
  //! Destructor
  //---------------------------------------------------------------------//
  ~virtual_semaphore() {}

  //---------------------------------------------------------------------//
  //! Block until successfully acquiring the semaphore, i.e. count > 0.
  //---------------------------------------------------------------------//
  bool acquire() {
    if(done_) {
      return false;
    }

    lock_t lock(mutex_);

    while(count_ <= 0) {
      cond_.wait(lock);
      if(done_) {
        return false;
      }
    }

    --count_;

    return true;
  }

  //---------------------------------------------------------------------//
  //! Non-blocking, attempt to acquire the semaphore and return true
  //! upon success
  //---------------------------------------------------------------------//
  bool try_acquire() {
    if(done_) {
      return false;
    }

    lock_t lock(mutex_);

    if(count_ > 0) {
      --count_;
      return true;
    }

    return false;
  }

  //---------------------------------------------------------------------//
  //! Release one count on the semaphore
  //---------------------------------------------------------------------//
  void release() {
    lock_t lock(mutex_);

    if(max_count_ == 0 || count_ < max_count_) {
      ++count_;
    }

    cond_.notify_one();
  }

  //---------------------------------------------------------------------//
  //! Disable semaphore and unblock all acquire operations.
  //---------------------------------------------------------------------//
  void interrupt() {
    done_ = true;
    cond_.notify_all();
  }

  virtual_semaphore & operator=(const virtual_semaphore &) = delete;

  virtual_semaphore(const virtual_semaphore &) = delete;

private:
  std::mutex mutex_;
  std::condition_variable cond_;

  int count_;
  int max_count_;
  std::atomic_bool done_;
};

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
