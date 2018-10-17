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

struct reduction_wrapper_unique__ {};

template<size_t HASH, typename TYPE>
struct reduction_wrapper__ {

  using rhs_t = typename TYPE::RHS;
  using lhs_t = typename TYPE::LHS;

  using oid_t = flecsi::utils::unique_id_t<reduction_wrapper_unique__>;

  /*!
    Register the user-defined reduction operator with the runtime.
   */

  static void registration_callback() {

    // Get the map of registered operations
    auto & reduction_ops = context_t::instance().reduction_operations();

    clog_assert(reduction_ops.find(HASH) == reduction_ops.end(),
      typeid(TYPE).name() << " has already been registered with this name");

    {
    clog_tag_guard(reduction_interface);
    clog(info) << "registering reduction operation " << HASH << std::endl;
    }

    const size_t id = oid_t::instance().next();

    // Register the operation with the Legion runtime
    Legion::Runtime::register_reduction_op<TYPE>(id);

    // Save the id for invocation
    reduction_ops[HASH].id = id;

    // Save the initial value for registering the collective
    // with the Legion runtime
    size_t bytes = sizeof(rhs_t);
    reduction_ops[HASH].initial.resize(bytes);
    rhs_t initial_value = TYPE::initial();
    std::memcpy(reduction_ops[HASH].initial.data(), &initial_value, bytes);

  } // registration_callback

}; // struct reduction_wrapper__

} // namespace execution
} // namespace flecsi
