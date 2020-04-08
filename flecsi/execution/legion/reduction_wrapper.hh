/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "../reduction.hh"
#include "flecsi/runtime/backend.hh"
#include "flecsi/utils/demangle.hh"
#include <flecsi/flog.hh>

#include <legion.h>

namespace flecsi {

inline flog::devel_tag reduction_wrapper_tag("reduction_wrapper");

namespace execution {

namespace detail {
/*!
  Register the user-defined reduction operator with the runtime.
*/

template<class>
void register_reduction();

inline Legion::ReductionOpID reduction_id;
} // namespace detail

// NB: 0 is reserved by Legion.
template<class R>
inline const Legion::ReductionOpID reduction_op =
  (runtime::context::instance().register_init(detail::register_reduction<R>),
    ++detail::reduction_id);

template<class TYPE>
void
detail::register_reduction() {
  {
    flog::devel_guard guard(reduction_wrapper_tag);
    flog_devel(info) << "registering reduction operation "
                     << utils::type<TYPE>() << std::endl;
  }

  // Register the operation with the Legion runtime
  Legion::Runtime::register_reduction_op<TYPE>(reduction_op<TYPE>);
}

} // namespace execution
} // namespace flecsi
