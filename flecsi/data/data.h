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

#include <flecsi/data/client.h>
#include <flecsi/data/field.h>
#include <flecsi/data/storage_class.h>
#include <flecsi/topology/internal.h>
#include <flecsi/utils/const_string.h>

/*!
  @def flecsi_register_topology

  Declare a variable of type \em type with namespace \em nspace
  and name \em name.

  This macro registers a data client with the FleCSI runtime. This call
  does not necessarily cause memory to be allocated. It's primary function
  is to describe the client type to the runtime. Memory allocation will
  likely be deferred.

  @param type   The \ref topology type.
  @param nspace The namespace to use to register the variable.
  @param name   The name to use to register the variable.

  @ingroup data
 */

#define flecsi_register_topology(type, nspace, name)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the client interface to register the data */                         \
  inline bool client_type##_##nspace##_##name##_client_registered =            \
    flecsi::data::client_interface_t::register_client<type,                    \
      flecsi_internal_hash(nspace), flecsi_internal_hash(name)>(               \
      {EXPAND_AND_STRINGIFY(name)})

namespace flecsi {
namespace topology {

flecsi_register_topology(global_topology_t, global_client, global_client);
flecsi_register_topology(color_topology_t, color_client, color_client);

} // namespace topology
} // namespace flecsi

/*!
  @def flecsi_register_global

  This macro registers global field data.
  This call does not necessarily cause memory to be allocated. Its
  primary function is to describe the field data to the runtime.
  Memory allocation will likely be deferred.

  @param nspace        The namespace to use to register the variable.
  @param name          The name of the data variable to register.
  @param data_type     The data type to store, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param versions      The number of versions of the data to register. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_register_global(nspace, name, data_type, versions)              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  inline bool client_type##_##nspace##_##name##_data_registered =              \
    flecsi::data::field_interface_t::register_field<                           \
      flecsi::topology::global_topology_t, flecsi::data::global, data_type,    \
      flecsi_internal_hash(nspace), flecsi_internal_hash(name), versions,      \
      flecsi::topology::global_index_space>({EXPAND_AND_STRINGIFY(name)})

/*!
  @def flecsi_register_color

  This macro registers color field data.
  This call does not necessarily cause memory to be allocated. Its
  primary function is to describe the field data to the runtime.
  Memory allocation will likely be deferred.

  @param nspace        The namespace to use to register the variable.
  @param name          The name of the data variable to register.
  @param data_type     The data type to store, e.g., double or my_type_t.
  @param versions      The number of versions of the data to register. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_register_color(nspace, name, data_type, versions)               \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  inline bool client_type##_##nspace##_##name##_data_registered =              \
    flecsi::data::field_interface_t::register_field<                           \
      flecsi::topology::color_topology_t, flecsi::data::color, data_type,      \
      flecsi_internal_hash(nspace), flecsi_internal_hash(name), versions,      \
      flecsi::topology::color_index_space>({EXPAND_AND_STRINGIFY(name)})
