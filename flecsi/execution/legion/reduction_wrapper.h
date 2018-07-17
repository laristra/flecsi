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

#include <cinchlog.h>

#include <flecsi/execution/context.h>
#include <flecsi/utils/common.h>

#include <legion.h>

clog_register_tag(reduction_wrapper);

namespace flecsi {
namespace execution {

template<size_t NAME, typename OPERATION>
struct reduction_wrapper__ {

  using rhs_t = typename OPERATION::RHS;
  using lhs_t = typename OPERATION::LHS;

  struct unique {};

  using oid_t = flecsi::utils::unique_id_t<unique>;

  /*!
    Register the user-defined reduction operator with the runtime.
   */

  static void registration_callback() {
    // Get the map of registered operations
    auto reduction_ops = context_t::instance().reduction_ops();

    clog_assert(reduction_ops.find(NAME) == reduction_ops.end(),
      typeid(OPERATION).name()
        << " has already been registered with this name");

    // Offset by 1 because 0 is reserved
    const size_t id = oid_t::instance().next() + 1;

    // Register the operation with the Legion runtime
    Legion::Runtime::register_reduction_op<OPERATION>(id);

    // Save the id for invocation
    reduction_ops[NAME] = id;
  } // registration_callback

}; // struct reduction_wrapper__

} // namespace execution
} // namespace flecsi
