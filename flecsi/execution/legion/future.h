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

#ifndef flecsi_execution_legion_future_h
#define flecsi_execution_legion_future_h

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Nov 15, 2015
//----------------------------------------------------------------------------//

#include <functional>
#include <legion.h>
#include <memory>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! Abstract interface type for Legion futures.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN
>
struct legion_future_concept__
{

  virtual ~legion_future_concept__() {}

  //--------------------------------------------------------------------------//
  //! Abstract interface to wait on a task result.
  //--------------------------------------------------------------------------//

  virtual void wait() = 0;

  //--------------------------------------------------------------------------//
  //! Abstract interface to get a task result.
  //--------------------------------------------------------------------------//

  virtual RETURN get(size_t index = 0) = 0;

}; // struct legion_future_concept__

//----------------------------------------------------------------------------//
//! Explicit specialization for void.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<>
struct legion_future_concept__<void>
{

  virtual ~legion_future_concept__() {}

  //--------------------------------------------------------------------------//
  //! Abstract interface to wait on a task result.
  //--------------------------------------------------------------------------//

  virtual void wait() = 0;

}; // struct legion_future_concept__

//----------------------------------------------------------------------------//
// Future model.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! Base future model type.
//!
//! @tparam RETURN The return type of the task.
//! @tparam FUTURE The Legion runtime future type.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  typename FUTURE
>
struct legion_future_model__ : public legion_future_concept__<RETURN>
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future.
  //!
  //! @param legion_future The Legion future instance.
  //--------------------------------------------------------------------------//

  legion_future_model__(const FUTURE & legion_future)
    : legion_future_(legion_future) {}

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void
  wait()
  {
  } // wait

  //--------------------------------------------------------------------------//
  //! Get a task result.
  //!
  //! @param index The index of the task.
  //!
  //! @remark This method only applies to indexed task invocations.
  //--------------------------------------------------------------------------//

  RETURN
  get(
    size_t index = 0
  )
  {
    return legion_future_.template get_result<RETURN>();
  } // get

private:

  FUTURE legion_future_;

}; // struct legion_future_model__

//----------------------------------------------------------------------------//
//! Partial specialization for void.
//!
//! @tparam FUTURE The Legion runtime future type.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<typename FUTURE>
struct legion_future_model__<void, FUTURE>
  : public legion_future_concept__<void>
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future.
  //!
  //! @param legion_future The Legion future instance.
  //--------------------------------------------------------------------------//

  legion_future_model__(const FUTURE & legion_future)
    : legion_future_(legion_future) {}

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void
  wait()
  {
    legion_future_.get_void_result();
  } // wait

private:

  FUTURE legion_future_;

}; // struct legion_future_model__

//----------------------------------------------------------------------------//
//! Partial specialization for index launch FutureMap.
//----------------------------------------------------------------------------//

template<typename RETURN>
struct legion_future_model__<RETURN, LegionRuntime::HighLevel::FutureMap>
  : public legion_future_concept__<RETURN>
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future map.
  //!
  //! @param legion_future The Legion future instance.
  //--------------------------------------------------------------------------//

  legion_future_model__(
    const LegionRuntime::HighLevel::FutureMap & legion_future
  )
    : legion_future_(legion_future) {}

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void
  wait()
  {
  } // wait

  //--------------------------------------------------------------------------//
  //! Get a task result.
  //!
  //! @param index The index of the task.
  //!
  //! @remark This method only applies to indexed task invocations.
  //--------------------------------------------------------------------------//

  RETURN
  get(
    size_t index = 0
  )
  {
    // FIXME: Need implementation
    return 0;
  } // get

private:

  LegionRuntime::HighLevel::FutureMap legion_future_;

}; // struct legion_future_model__

//----------------------------------------------------------------------------//
//! Explicit specialization for index launch FutureMap and void.
//----------------------------------------------------------------------------//

template<>
struct legion_future_model__<void, LegionRuntime::HighLevel::FutureMap>
  : public legion_future_concept__<void>
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future map.
  //!
  //! @param legion_future The Legion future instance.
  //--------------------------------------------------------------------------//

  legion_future_model__(
    const LegionRuntime::HighLevel::FutureMap & legion_future
  )
    : legion_future_(legion_future) {}

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void
  wait()
  {
  } // wait

private:

  LegionRuntime::HighLevel::FutureMap legion_future_;

}; // struct legion_future_model__

//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! Legion future type.
//!
//! @tparam RETURN The return type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN
>
struct legion_future__
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future map.
  //!
  //! @tparam FUTURE
  //!
  //! @param future
  //--------------------------------------------------------------------------//

  template<
    typename FUTURE
  >
  legion_future__(
    const FUTURE & future
  )
  :
    state_(new legion_future_model__<RETURN, FUTURE>(future))
  {
  } // legion_future__

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //!
  //! @param lf The legion_future__ to use to set our state.
  //--------------------------------------------------------------------------//

  legion_future__(const legion_future__ & lf)
    : state_(lf.state_) {}

  //--------------------------------------------------------------------------//
  //! Assignment operator.
  //!
  //! @param lf The legion_future__ to use to set our state.
  //--------------------------------------------------------------------------//

  legion_future__ &
  operator = (
    const legion_future__ & lf
  )
  {
    state_ = lf.state_;
  } // operator =

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void
  wait()
  {
    state_->wait();
  } // wait

  //--------------------------------------------------------------------------//
  //! Get a task result.
  //!
  //! @param index The index of the task.
  //!
  //! @remark This method only applies to indexed task invocations.
  //--------------------------------------------------------------------------//

  RETURN
  get(
    size_t index = 0
  )
  {
    return state_->get(index);
  } // get

private:

  // Needed to satisfy static check.
  void set() {}

  std::shared_ptr<legion_future_concept__<RETURN>> state_;

}; // struct legion_future__

//----------------------------------------------------------------------------//
//! Legion future type. Explicit specialization for void.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<>
struct legion_future__<void>
{

  //--------------------------------------------------------------------------//
  //! Construct a future from a Legion future map.
  //!
  //! @tparam FUTURE
  //!
  //! @param future
  //--------------------------------------------------------------------------//

  template<typename FUTURE>
  legion_future__(const FUTURE & future)
    : state_(new legion_future_model__<void, FUTURE>(future)) {}

  //--------------------------------------------------------------------------//
  //! Wait on a task result.
  //--------------------------------------------------------------------------//

  void wait()
  {
  } // wait

  std::shared_ptr<legion_future_concept__<void>> state_;

}; // struct legion_future__

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_future_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
