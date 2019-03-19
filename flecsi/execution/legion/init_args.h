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
  #include <flecsi/utils/demangle.h>
  #include <flecsi/utils/tuple_walker.h>
#endif

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(init_args);

namespace flecsi {
namespace execution {
namespace legion {

using namespace flecsi::data::legion;

/*!
  The init_args_t type can be called to walk task args before the
  task launcher is created. This allows us to gather region requirements
  and to set state on the associated data handles \em before Legion gets
  the task arguments tuple.

  @ingroup execution
*/

struct init_args_t : public flecsi::utils::tuple_walker_u<init_args_t> {

  /*!
    Construct an init_args_t instance.

    @param runtime The Legion task runtime.
    @param context The Legion task runtime context.
   */

  init_args_t(Legion::Runtime * runtime, Legion::Context & context)
    : runtime_(runtime), context_(context) {}

  /*!
    Convert the template privileges to proper Legion privileges.

    @param mode privilege
   */

  static Legion::PrivilegeMode privilege_mode(size_t mode) {
#if 0
    switch(mode) {
      case size_t(reserved):
        return NO_ACCESS;
      case size_t(ro):
        return READ_ONLY;
      case size_t(wo):
        return WRITE_DISCARD;
      case size_t(rw):
        return READ_WRITE;
      default:
        clog_fatal("invalid privilege mode");
    } // switch
    // should never get here, but this is needed
    // to avoid compiler warnings
    return NO_ACCESS;
#endif
    return READ_WRITE;
  } // privilege_mode

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
    Non-FleCSI Data Types
   *--------------------------------------------------------------------------*/

  template<typename DATA_TYPE>
  static typename std::enable_if_t<
    !std::is_base_of_v<data::data_reference_base_t, DATA_TYPE>>
  visit(DATA_TYPE &) {
    {
    flog_tag_guard(init_args);
    flog(internal) << "Skipping argument with type " <<
      flecsi::utils::type<DATA_TYPE>() << std::endl;
    }
  } // visit

private:

  Legion::Runtime * runtime_;
  Legion::Context & context_;

}; // init_args_t

} // namespace legion
} // namespace execution
} // namespace flecsi
