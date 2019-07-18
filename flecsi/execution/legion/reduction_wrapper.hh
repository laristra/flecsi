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
#else
#include <flecsi/execution/context.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/flog.hh>
#endif

#include <legion.h>

flog_register_tag(reduction_wrapper);

namespace flecsi {
namespace execution {
namespace legion {

template<size_t HASH, typename TYPE>
struct reduction_wrapper_u {

  using rhs_t = typename TYPE::RHS;
  using lhs_t = typename TYPE::LHS;

  /*!
    Register the user-defined reduction operator with the runtime.
   */

  static void registration_callback() {

    // Get the map of registered operations
    auto & reduction_ops = context_t::instance().reduction_operations();

    flog_assert(reduction_ops.find(HASH) == reduction_ops.end(),
      typeid(TYPE).name() << " has already been registered with this name");

    {
      flog_tag_guard(reduction_wrapper);
      flog_devel(info) << "registering reduction operation " << HASH
                       << std::endl;
    }

    const size_t id = unique_oid_t::instance().next();

    // Register the operation with the Legion runtime
    Legion::Runtime::register_reduction_op<TYPE>(id);

    // Save the id for invocation
    reduction_ops[HASH] = id;

  } // registration_callback

}; // struct reduction_wrapper_u

} // namespace legion
} // namespace execution
} // namespace flecsi
