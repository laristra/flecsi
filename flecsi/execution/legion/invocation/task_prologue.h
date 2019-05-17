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

flog_register_tag(prologue);

namespace flecsi {
namespace execution {
namespace legion {

using namespace flecsi::data::legion;

/*!

 @ingroup execution
 */

struct task_prologue_t : public flecsi::utils::tuple_walker_u<task_prologue_t> {

  /*!
   Construct a task_prologue_t instance.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  task_prologue_t(Legion::Runtime * runtime,
    Legion::Context & context,
    Legion::Domain & color_domain)
    : runtime_(runtime), context_(context), color_domain_(color_domain) {
  } // task_prologue_t

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
    Color Topology
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(index_topology::accessor_u<DATA_TYPE, PRIVILEGES> & accessor) {
  } // visit

  /*--------------------------------------------------------------------------*
    Unstructured Mesh Topology
   *--------------------------------------------------------------------------*/

#if 0
  template<typename DATA_TYPE, size_t PRIVILEGES>
  using dense_unstructured_mesh_accessor_u =
    data::legion::unstructured_mesh_topology::dense_accessor_u<
      DATA_TYPE, PRIVILEGES>;

  template<typename DATA_TYPE, size_t PRIVILEGES>
  void visit(
      dense_unstructured_mesh_accessor_u<DATA_TYPE, PRIVILEGES> & accessor) {
  } // visit
#endif

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

  /*--------------------------------------------------------------------------*
    Launch Copies
   *--------------------------------------------------------------------------*/

  void update_state() {} // update

private:
  Legion::Runtime * runtime_;
  Legion::Context & context_;
  Legion::Domain & color_domain_;

}; // task_prologue_t

} // namespace legion
} // namespace execution
} // namespace flecsi
