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

#include "flecsi/exec/fold.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/demangle.hh"
#include <flecsi/flog.hh>

#include <legion.h>

namespace flecsi {

inline log::devel_tag reduction_wrapper_tag("reduction_wrapper");

namespace exec {

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
  (run::context::instance().register_init(detail::register_reduction<R>),
    ++detail::reduction_id);

template<class TYPE>
void
detail::register_reduction() {
  {
    log::devel_guard guard(reduction_wrapper_tag);
    flog_devel(info) << "registering reduction operation " << util::type<TYPE>()
                     << std::endl;
  }

  // Register the operation with the Legion runtime
  Legion::Runtime::register_reduction_op<TYPE>(reduction_op<TYPE>);
}

} // namespace exec
} // namespace flecsi
