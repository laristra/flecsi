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
  virtual void
  add_future_to_single_task_launcher(Legion::TaskLauncher &launcher) const = 0;

  virtual void
  add_future_to_index_task_launcher(Legion::IndexLauncher &launcher) const = 0;
};

/*!
  Base legion future  type.

  @tparam RETURN The return type of the task.
  @tparam FUTURE The Legion runtime future type.

  @ingroup legion-execution
*/

template <typename RETURN, typename FUTURE> struct legion_future__ {};

/*! Partial specialization for the Legion:Future

  @tparam RETURN The return type of the task.

  @ingroup legion-execution
 */

template <typename RETURN>
struct legion_future__<RETURN, Legion::Future> : public future_base_t {
  /*!
      Construct a future from a Legion future.

      @param legion_future The Legion future instance.
     */

  legion_future__(const Legion::Future &legion_future)
      : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) { legion_future_.wait(); } // wait

  /*!
    Get a task result.

    @param index The index of the task.

    @remark This method only applies to indexed task invocations.
   */

  RETURN
  get(size_t index = 0, bool silence_warnings = false) {
    return legion_future_.template get_result<RETURN>(silence_warnings);
  } // get

  void
  defer_dynamic_collective_arrival(Legion::Runtime *runtime,
                                   Legion::Context ctx,
                                   Legion::DynamicCollective &dc_reduction) {
    runtime->defer_dynamic_collective_arrival(ctx, dc_reduction,
                                              legion_future_);
  } // defer_dynamic_collective_arrival

  /*!
    Add Legion Future to the task launcher
   */
  void
  add_future_to_single_task_launcher(Legion::TaskLauncher &launcher) const {
    launcher.add_future(legion_future_);
  }

  void
  add_future_to_index_task_launcher(Legion::IndexLauncher &launcher) const {
    launcher.add_future(legion_future_);
  }

private:
  Legion::Future legion_future_;
}; // legion_future

/*! void specialization for the Legion:Future

  @ingroup legion-execution
 */

template <>
struct legion_future__<void, Legion::Future> : public future_base_t {

  /*!
    Construct a future from a Legion future.

    @param legion_future The Legion future instance.
   */

  legion_future__(const Legion::Future &legion_future)
      : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.get_void_result(silence_warnings);
  } // wait

  void
  defer_dynamic_collective_arrival(Legion::Runtime *runtime,
                                   Legion::Context ctx,
                                   Legion::DynamicCollective &dc_reduction) {
    // reduction of a void is still void
  }

  /*!
    Add Legion Future to the task launcher
   */
  void
  add_future_to_single_task_launcher(Legion::TaskLauncher &launcher) const {
    launcher.add_future(legion_future_);
  }

  void
  add_future_to_index_task_launcher(Legion::IndexLauncher &launcher) const {
    launcher.add_future(legion_future_);
  }

private:
  Legion::Future legion_future_;
}; // legion_future

/*! Partial specialization for the Legion:FutureMap

  @tparam RETURN The return type of the task.

  @ingroup legion-execution
 */

template <typename RETURN>
struct legion_future__<RETURN, Legion::FutureMap> : public future_base_t {

  /*!
    Construct a future from a Legion future map.

    @param legion_future The Legion future instance.
   */

  legion_future__(const Legion::FutureMap &legion_future)
      : legion_future_(legion_future) {}

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
  get(size_t index = 0, bool silence_warnings = false) {
    return legion_future_.get_result<RETURN>(
        Legion::DomainPoint::from_point<1>(
            LegionRuntime::Arrays::Point<1>(index)),
        silence_warnings);
  } // get

  void
  defer_dynamic_collective_arrival(Legion::Runtime *runtime,
                                   Legion::Context ctx,
                                   Legion::DynamicCollective &dc_reduction) {
    // Not sure what reducing a map with other maps would mean
  }

  /*!
    Add Legion Future to the task launcher
   */
  void
  add_future_to_single_task_launcher(Legion::TaskLauncher &launcher) const {
    assert(false && "you can't pass future from index task to any task");
  }

  void
  add_future_to_index_task_launcher(Legion::IndexLauncher &launcher) const {
    assert(false && "you can't pass future handle from index task to any task");
  }

private:
  Legion::FutureMap legion_future_;
}; // legion_future

/*!
  Explicit specialization for index launch FutureMap and void.
 */

template <>
struct legion_future__<void, Legion::FutureMap> : public future_base_t {

  /*!
      Construct a future from a Legion future map.

      @param legion_future The Legion future instance.
     */

  legion_future__(const Legion::FutureMap &legion_future)
      : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Add Legion Future to the task launcher
   */
  void
  add_future_to_single_task_launcher(Legion::TaskLauncher &launcher) const {
    assert(false && "you can't pass future from index task to any task");
  }

  void
  add_future_to_index_task_launcher(Legion::IndexLauncher &launcher) const {
    assert(false && "you can't pass future handle from index task to any task");
  }

private:
  Legion::FutureMap legion_future_;

}; // legion_future

#if 0
template<typename RETURN, typename FUTURE>

struct legion_future_model__ : public legion_future_concept__<RETURN> {

  /*!
    Construct a future from a Legion future.
  
    @param legion_future The Legion future instance.
   */

  legion_future_model__(const FUTURE & legion_future)
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

  void defer_dynamic_collective_arrival(
      Legion::Runtime * runtime,
      Legion::Context ctx,
      Legion::DynamicCollective & dc_reduction) {
    runtime->defer_dynamic_collective_arrival(
        ctx, dc_reduction, legion_future_);
  } // defer_dynamic_collective_arrival

private:
  FUTURE legion_future_;

}; // struct legion_future_model__

/*!
  Partial specialization for void.

  @tparam FUTURE The Legion runtime future type.

  @ingroup legion-execution
 */

template<typename FUTURE>
struct legion_future_model__<void, FUTURE>
    : public legion_future_concept__<void> {

  /*!
    Construct a future from a Legion future.
  
    @param legion_future The Legion future instance.
   */

  legion_future_model__(const FUTURE & legion_future)
      : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.get_void_result(silence_warnings);
  } // wait

  void defer_dynamic_collective_arrival(
      Legion::Runtime * runtime,
      Legion::Context ctx,
      Legion::DynamicCollective & dc_reduction) {
    // reduction of a void is still void
  }

private:
  FUTURE legion_future_;

}; // struct legion_future_model__


/*!
  Partial specialization for index launch FutureMap.
 */

template<typename RETURN>
struct legion_future_model__<RETURN, Legion::FutureMap>
    : public legion_future_concept__<RETURN> {

  /*!
    Construct a future from a Legion future map.
  
    @param legion_future The Legion future instance.
   */

  legion_future_model__(const Legion::FutureMap & legion_future)
      : legion_future_(legion_future) {}

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
  get(size_t index = 0, bool silence_warnings = false) {
    return legion_future_.get_result<RETURN>(
        Legion::DomainPoint::from_point<1>(
            LegionRuntime::Arrays::Point<1>(index)),
        silence_warnings);
  } // get

  void defer_dynamic_collective_arrival(
      Legion::Runtime * runtime,
      Legion::Context ctx,
      Legion::DynamicCollective & dc_reduction) {
    // Not sure what reducing a map with other maps would mean
  }

private:
  Legion::FutureMap legion_future_;

}; // struct legion_future_model__

/*!
 Explicit specialization for index launch FutureMap and void.
*/

template<>
struct legion_future_model__<void, Legion::FutureMap>
    : public legion_future_concept__<void> {

  /*!
    Construct a future from a Legion future map.
  
    @param legion_future The Legion future instance.
   */

  legion_future_model__(const Legion::FutureMap & legion_future)
      : legion_future_(legion_future) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

private:
  Legion::FutureMap legion_future_;

}; // struct legion_future_model__
//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//
/*!
//! Legion future type.
//!
//! @tparam RETURN The return type of the task.
//!
//! @ingroup legion-execution
 */
template<typename RETURN>
struct legion_future__ {

  /*!
   Construct a future from a Legion future map.
  
   @tparam FUTURE
  
   @param future
   */

  template<typename FUTURE>
  legion_future__(const FUTURE & future)
      : state_(new legion_future_model__<RETURN, FUTURE>(future)) {
  } // legion_future__

  /*!
   Copy constructor.
  
   @param lf The legion_future__ to use to set our state.
   */

  legion_future__(const legion_future__ & lf) : state_(lf.state_) {}

  /*!
   Assignment operator.
  
   @param lf The legion_future__ to use to set our state.
   */

  legion_future__ & operator=(const legion_future__ & lf) {
    state_ = lf.state_;
  } // operator =

  /*!
   Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    state_->wait(silence_warnings);
  } // wait

  /*!
    Get a task result.
  
    @param index The index of the task.
  
    @remark This method only applies to indexed task invocations.
   */

  RETURN
  get(size_t index = 0, bool silence_warnings = false) {
    return state_->get(index, silence_warnings);
  } // get

  void defer_dynamic_collective_arrival(
      Legion::Runtime * runtime,
      Legion::Context ctx,
      Legion::DynamicCollective & dc_reduction) {
    state_->defer_dynamic_collective_arrival(runtime, ctx, dc_reduction);
  } // defer_dynamic_collective_arrival

private:
  // Needed to satisfy static check.
  void set() {}

  std::shared_ptr<legion_future_concept__<RETURN>> state_;

}; // struct legion_future__

/*!
  Legion future type. Explicit specialization for void.

  @ingroup legion-execution
 */

template<>
struct legion_future__<void> {

  /*!
    Construct a future from a Legion future map.
  
    @tparam FUTURE
  
    @param future
   */

  template<typename FUTURE>
  legion_future__(const FUTURE & future)
      : state_(new legion_future_model__<void, FUTURE>(future)) {}

  /*!
    Wait on a task result.
   */

  void wait(bool silence_warnings = false) {
    state_->wait(silence_warnings);
  } // wait

  std::shared_ptr<legion_future_concept__<void>> state_;

}; // struct legion_future__
#endif
} // namespace execution
} // namespace flecsi
