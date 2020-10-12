/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <functional>
#include <iostream>
#include <memory>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {
namespace execution {

/*!
 Future abstract base class (need it to store pointers to legion futures
 passed to the task)
 */

struct future_base_t {
public:
  virtual void add_to_single_task_launcher(
    Legion::TaskLauncher & launcher) const = 0;

  /*!
    Abstract interface to wait on a task result.
   */

  virtual void wait(bool silence_warnings = false) = 0;

  //  /*!
  //    Abstract interface to get a task result.
  //   */

  //  virtual RETURN get(size_t index = 0, bool silence_warnings = false) = 0;

  virtual void init_future(void) = 0;

  virtual void finalize_future(void) = 0;
};

/*!
  Base legion future  type.

  @tparam RETURN The return type of the task.
  @tparam FUTURE The Legion runtime future type.

  @ingroup legion-execution
*/

template<typename RETURN, launch_type_t launch>
struct legion_future_u {};

/*! Partial specialization for the Legion:Future

  @tparam RETURN The return type of the task.

  @ingroup legion-execution
 */

template<typename RETURN>
struct legion_future_u<RETURN, launch_type_t::single> : public future_base_t {

  legion_future_u(const legion_future_u & f) {
    data_ = f.data_;
    initialized_ = f.initialized_;
    auto legion_runtime = Legion::Runtime::get_runtime();
    if(initialized_)
      legion_future_ = Legion::Future::from_value(legion_runtime, data_);
    else
      legion_future_ = f.legion_future_;
  }

  /*!
    Construct a future from a Legion future.

    @param legion_future The Legion future instance.
   */

  legion_future_u(const Legion::Future & legion_future)
    : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.wait();
  } // wait

  /*!
    Get a task result.

    @param index The index of the task.

    @remark This method only applies to indexed task invocations.
   */

  RETURN
  get(size_t index = 0, bool silence_warnings = false) {
    return legion_future_.template get_result<RETURN>(silence_warnings);
  } // get

  /*!
    Add Legion Future to the task launcher
   */
  void add_to_single_task_launcher(Legion::TaskLauncher & launcher) const {
    launcher.add_future(legion_future_);
  }

  void add_to_index_task_launcher(Legion::IndexLauncher & launcher) const {
    launcher.add_future(legion_future_);
  }

  void init_future(void) {
    initialized_ = true;
  }

  void finalize_future(void) {
    initialized_ = false;
  }

  RETURN & operator=(legion_future_u const & f) {
    return data_;
  }

  operator RETURN &() {
    return data_;
  }

  operator const RETURN &() const {
    return data_;
  }

  friend std::ostream & operator<<(std::ostream & stream,
    const legion_future_u & f) {
    stream << f.data_;
    return stream;
  } // switch

  Legion::Future & raw_future() {
    return legion_future_;
  }

  RETURN data_;

private:
  bool initialized_ = false;
  Legion::Future legion_future_;
}; // legion_future

/*! void specialization for the Legion:Future

  @ingroup legion-execution
 */

template<>
struct legion_future_u<void, launch_type_t::single> : public future_base_t {

  /*!
    Construct a future from a Legion future.

    @param legion_future The Legion future instance.
   */

  legion_future_u(const Legion::Future & legion_future)
    : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.get_void_result(silence_warnings);
  } // wait

  /*!
    Add Legion Future to the task launcher
   */
  void add_to_single_task_launcher(Legion::TaskLauncher & launcher) const {
    launcher.add_future(legion_future_);
  }

  void add_to_index_task_launcher(Legion::IndexLauncher & launcher) const {
    launcher.add_future(legion_future_);
  }

  void init_future(void) {}
  void finalize_future(void) {}

private:
  Legion::Future legion_future_;
}; // legion_future

/*! Partial specialization for the Legion:FutureMap

  @tparam RETURN The return type of the task.

  @ingroup legion-execution
 */

template<typename RETURN>
struct legion_future_u<RETURN, launch_type_t::index> : public future_base_t {

  /*!
    Construct a future from a Legion future map.

    @param legion_future The Legion FutureMap instance.
   */

  legion_future_u(const Legion::FutureMap & legion_future)
    : legion_future_(legion_future) {}

  /*!
   Construct a future from a Legion future.

   @param legion_future The Legion future instance.
  */

  legion_future_u(const Legion::Future & legion_future) {
    legion_future_[0] = legion_future;
  }

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Get a task result.

    @param index The index of the task.

    @remark This method only applies to indexed task invocations.
   */

  RETURN
  get(size_t index, bool silence_warnings = false) {
    return legion_future_.get_result<RETURN>(
      Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::Point<1>(index)),
      silence_warnings);
  } // get

  RETURN
  get(bool silence_warnings = false) {
    auto runtime = Legion::Runtime::get_runtime();
    size_t index = runtime->find_local_MPI_rank();
    return legion_future_.get_result<RETURN>(
      Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::Point<1>(index)),
      silence_warnings);
  } // get

  Legion::FutureMap & raw_future() {
    return legion_future_;
  }

  RETURN & operator=(legion_future_u const & f) {
    return data_;
  }

  operator RETURN &() {
    return data_;
  }

  operator const RETURN &() const {
    return data_;
  }

  friend std::ostream & operator<<(std::ostream & stream,
    const legion_future_u & f) {
    stream << f.data_;
    return stream;
  } // switch

  RETURN data_;

  /*!
    Add Legion Future to the task launcher
   */
  void add_to_single_task_launcher(Legion::TaskLauncher & launcher) const {
    assert(false && "you can't pass future from index task to any task");
  }

  void add_to_index_task_launcher(Legion::IndexLauncher & launcher) const {
    assert(false && "you can't pass future handle from index task to any task");
  }

  void init_future(void) {}
  void finalize_future(void) {}

private:
  Legion::FutureMap legion_future_;

}; // struct legion_future_u

/*!
 Explicit specialization for index launch FutureMap and void.
*/

/*!
  Explicit specialization for index launch FutureMap and void.
 */

template<>
struct legion_future_u<void, launch_type_t::index> : public future_base_t {

  /*!
      Construct a future from a Legion future map.

      @param legion_future The Legion future instance.
     */

  legion_future_u(const Legion::FutureMap & legion_future)
    : legion_future_(legion_future) {}

  legion_future_u(const Legion::Future & legion_future) {
    legion_future_[0] = legion_future;
  }

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Add Legion Future to the task launcher
   */
  void add_to_single_task_launcher(Legion::TaskLauncher & launcher) const {
    assert(false && "you can't pass future from index task to any task");
  }

  void add_to_index_task_launcher(Legion::IndexLauncher & launcher) const {
    assert(false && "you can't pass future handle from index task to any task");
  }

  void init_future(void) {}
  void finalize_future(void) {}

private:
  Legion::FutureMap legion_future_;

}; // legion_future

template<typename RETURN, launch_type_t launch>
using flecsi_future = legion_future_u<RETURN, launch>;

} // namespace execution
} // namespace flecsi
