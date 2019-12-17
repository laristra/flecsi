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
#include <flecsi/utils/flog.hh>

#include <legion.h>

flog_register_tag(reduction_wrapper);

namespace flecsi {
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
inline const Legion::ReductionOpID
  reduction_op = (runtime::context::instance().register_reduction_operation(
                    detail::register_reduction<R>),
    ++detail::reduction_id);

template<class TYPE>
void
detail::register_reduction() {
  {
    flog_tag_guard(reduction_wrapper);
    flog_devel(info) << "registering reduction operation "
                     << utils::type<TYPE>() << std::endl;
  }

  // Register the operation with the Legion runtime
  Legion::Runtime::register_reduction_op<TYPE>(reduction_op<TYPE>);
}

} // namespace execution
} // namespace flecsi
