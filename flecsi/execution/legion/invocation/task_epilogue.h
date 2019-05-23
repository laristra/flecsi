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
#include <flecsi/data/legion/storage_classes.h>
#include <flecsi/utils/demangle.h>
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

  task_epilogue_t(Legion::Runtime * runtime, Legion::Context & context)
    : runtime_(runtime), context_(context) {} // task_epilogue_t

  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*
    The following methods are specializations on storage class and client
    type, potentially for every permutation thereof.
   *^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  /*--------------------------------------------------------------------------*
    Global Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(global_topology::accessor_u<DATA_TYPE, PRIVILEGES> & accessor) {
  } // visit

  /*--------------------------------------------------------------------------*
    Index Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(index_topology::accessor_u<DATA_TYPE, PRIVILEGES> & accessor) {
  } // visit

  /*--------------------------------------------------------------------------*
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::data_reference_base_t, DATA_TYPE>>
  visit(DATA_TYPE &) {
    {
      flog_tag_guard(init_args);
      flog(internal) << "Skipping argument with type "
                     << flecsi::utils::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:
  Legion::Runtime * runtime_;
  Legion::Context & context_;

}; // task_epilogue_t

} // namespace legion
} // namespace execution
} // namespace flecsi
