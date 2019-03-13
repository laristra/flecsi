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

#if !defined(__FLECSI_PRIVATE__)
#error Do not inlcude this file directly!
#else
  #include <flecsi/utils/tuple_walker.h>
#endif

flog_register_tag(prologue);

namespace flecsi {
namespace execution {

/*!

 @ingroup execution
 */

struct task_prologue_t : public flecsi::utils::tuple_walker_u<task_prolog_t> {

  /*!
   Construct a task_prolog_t instance.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  task_prolog_t(Legion::Runtime * runtime,
    Legion::Context & context,
    Legion::Domain & color_domain)
    : runtime_(runtime), context_(context), color_domain_(color_domain) {
  } // task_prolog_t

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit() {
  } // visit

private:

  Legion::Runtime * runtime_;
  Legion::Context & context_;
  Legion::Domain & color_domain_;

}; // task_prologue_t

} // namespace execution
} // namespace flecsi
