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
#error Do not include this file directly!
#else
  #include <flecsi/data/legion/storage_classes.h>
  #include <flecsi/utils/tuple_walker.h>
#endif

flog_register_tag(epilogue);

namespace flecsi {
namespace execution {
namespace legion {

using namespace flecsi::data::legion;

/*!

 @ingroup execution
 */

struct task_epilogue_t : public flecsi::utils::tuple_walker_u<task_epilogue_t> {

  /*!
   Construct a task_epilogue_t instance.

   @param runtime      The Legion task runtime.
   @param context      The Legion task runtime context.
   @param color_domain The Legion color domain.
   */

  task_epilogue_t(Legion::Runtime * runtime,
    Legion::Context & context)
    : runtime_(runtime), context_(context) {
  } // task_epilogue_t

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on storage class and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVLEGES>
  void visit(global_topology::accessor_u<DATA_TYPE, PRIVLEGES> & accessor) {
  } // visit

private:

  Legion::Runtime * runtime_;
  Legion::Context & context_;

}; // task_epilogue_t

} // namespace legion
} // namespace execution
} // namespace flecsi
