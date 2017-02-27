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

#include <functional>
#include <memory>

#include <legion.h>

///
/// \file legion/future.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
// Dummy type for future specialization selection.
struct mpitask_t {};
#endif // FLECSI_RUNTIME_MODEL_mpilegion

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

///
/// Interface type for Legion futures.
///
template<
  typename R
>
struct legion_future_concept__
{
  virtual ~legion_future_concept__() {}

  virtual void wait() = 0;
  virtual R get(size_t index = 0) = 0;
}; // struct legion_future_concept__

///
/// Explicit specialization for void.
///
template<>
struct legion_future_concept__<void>
{
  virtual ~legion_future_concept__() {}

  virtual void wait() = 0;
}; // struct legion_future_concept__

//----------------------------------------------------------------------------//
// Future model.
//----------------------------------------------------------------------------//

///
/// Base future model type.
///
template<typename R, typename F>
struct legion_future_model__
  : public legion_future_concept__<R>
{

  legion_future_model__(const F & legion_future)
    : legion_future_(legion_future) {}

  ///
  ///
  ///
  void
  wait()
  {
  } // wait

  ///
  /// \param index
  ///
  /// \return R
  ///
  R
  get(
    size_t index = 0
  )
  {
    return legion_future_.template get_result<R>();
  } // get

private:

  F legion_future_;

}; // struct legion_future_model__

///
/// Partial specialization for void.
///
template<typename F>
struct legion_future_model__<void, F>
  : public legion_future_concept__<void>
{

  legion_future_model__(const F & legion_future)
    : legion_future_(legion_future) {}

  void
  wait()
  {
    legion_future_.get_void_result();
  } // wait

private:

  F legion_future_;

}; // struct legion_future_model__

///
// Partial specialization for index launch FutureMap.
///
template<typename R>
struct legion_future_model__<R, LegionRuntime::HighLevel::FutureMap>
  : public legion_future_concept__<R>
{

  legion_future_model__(
    const LegionRuntime::HighLevel::FutureMap & legion_future
  )
    : legion_future_(legion_future) {}

  void
  wait()
  {
  } // wait

  R
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

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
///
/// Partial specialization for mpi task
///
template<typename R>
struct legion_future_model__<R, mpitask_t>
  : public legion_future_concept__<R>
{
  legion_future_model__(mpitask_t task) {}

  void
  wait()
  {
  } // wait

  R
  get(
    size_t index = 0
  )
  {
    return 0.0;
  } // get

}; // struct legion_future_model__
#endif // FLECSI_RUNTIME_MODEL_mpilegion

///
/// Explicit specialization for index launch FutureMap and void.
///
template<>
struct legion_future_model__<void, LegionRuntime::HighLevel::FutureMap>
  : public legion_future_concept__<void>
{

  legion_future_model__(
    const LegionRuntime::HighLevel::FutureMap & legion_future
  )
    : legion_future_(legion_future) {}

  void
  wait()
  {
  } // wait

private:

  LegionRuntime::HighLevel::FutureMap legion_future_;

}; // struct legion_future_model__

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
///
/// Explicit specialization for mpi task and void
///
template<>
struct legion_future_model__<void, mpitask_t>
  : public legion_future_concept__<void>
{
  legion_future_model__(mpitask_t task) {}

  void
  wait()
  {
  } // wait

}; // struct legion_future_model__
#endif // FLECSI_RUNTIME_MODEL_mpilegion

//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//

///
///
///
template<
  typename R
>
struct legion_future__
{
  using result_t = R;

  template<typename F>
  legion_future__(const F & future)
    : state_(new legion_future_model__<R, F>(future)) {}

  legion_future__(const legion_future__ & lf)
    : state_(lf.state_) {}

  legion_future__ &
  operator = (
    const legion_future__ & lf
  )
  {
    state_ = lf.state_;
  } // operator =

  ///
  ///
  ///
  void wait()
  {
    state_->wait();
  } // wait

  ///
  /// get operator
  ///
  result_t
  get(
    size_t index = 0
  )
  {
    return state_->get(index);
  } // get

private:

  // Needed to satisfy static check.
  void set() {}

  std::shared_ptr<legion_future_concept__<R>> state_;

}; // struct legion_future__

///
/// Explicit specialization for void.
///
template<>
struct legion_future__<void>
{

  template<typename F>
  legion_future__(const F & future)
    : state_(new legion_future_model__<void, F>(future)) {}

  ///
  ///
  ///
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
