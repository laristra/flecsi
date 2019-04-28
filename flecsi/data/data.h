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

/*!
  @file

  This file contains the high-level, macro interface for the FleCSI
  data model.
 */

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/field_interface.h>
#include <flecsi/data/topology_interface.h>
#include <flecsi/topology/internal/color.h>
#include <flecsi/topology/internal/global.h>
#include <flecsi/utils/const_string.h>

/*----------------------------------------------------------------------------*
  General Topology Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_register_topology

  Declare a variable of type \em type with namespace \em nspace
  and name \em name.

  This macro registers a topology with the FleCSI runtime. This call
  does not necessarily cause memory to be allocated. It's primary function
  is to describe the topology type to the runtime. Memory allocation will
  likely be deferred.

  @param type   The topology type.
  @param nspace The string namespace to use to register the variable.
  @param name   The string name to use to register the variable.

  @ingroup data
 */

#define flecsi_register_topology(type, nspace, name)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the topology interface to register the topology */                   \
  inline bool flecsi_internal_unique_name(topology_registration_) =            \
    flecsi::data::topology_interface_t::register_topology<type,                \
      flecsi_internal_string_hash(nspace),                                     \
      flecsi_internal_string_hash(name)>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_get_topology

  Access a topology.

  @param type   The topology type.
  @param nspace The string namespace to use to access the variable.
  @param name   The string name of the data variable to access.

  @ingroup data
 */

#define flecsi_get_topology(type, nspace, name)                                \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the topology */                \
  flecsi::data::topology_interface_t::get_topology<type,                       \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name)>()

/*----------------------------------------------------------------------------*
  Field Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_register_field

  This macro registers field data with a data_client_t type. Data
  registration creates a data attribute for the given topology type.
  This call does not necessarily cause memory to be allocated. Its
  primary function is to describe the field data to the runtime.
  Memory allocation will likely be deferred.

  @param topology_type The topology type.
  @param nspace        The string namespace to use to register the variable.
  @param name          The string name of the data variable to register.
  @param data_type     The data type to store, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param versions      The number of versions of the data to register. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_register_field(                                                 \
  topology_type, nspace, name, data_type, storage_class, versions, ...)        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  inline bool flecsi_internal_unique_name(field) =                             \
    flecsi::data::field_interface_t::register_field<topology_type,             \
      flecsi::data::storage_class,                                             \
      data_type,                                                               \
      flecsi_internal_string_hash(nspace),                                     \
      flecsi_internal_string_hash(name),                                       \
      versions,                                                                \
      ##__VA_ARGS__>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_get_field

  Access data with a topology instance.

  @param topology      The topology instance with which to access
                       the data.
  @param nspace        The string namespace to use to access the variable.
  @param name          The string name of the data variable to access.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_get_field(                                                      \
  topology, nspace, name, data_type, storage_class, version)                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the data */                    \
  flecsi::data::field_interface_t::get_field<decltype(                         \
                                               topology)::topology_type_t,     \
    flecsi::data::storage_class,                                               \
    data_type,                                                                 \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name),                                         \
    version>(topology)

/*----------------------------------------------------------------------------*
  Global Topology Interface.
 *----------------------------------------------------------------------------*/

namespace flecsi {
namespace topology {

flecsi_register_topology(global_topology_t, "global_client", "global_client");

} // namespace topology
} // namespace flecsi

/*!
  @def flecsi_register_global

  This macro registers global field data.
  This call does not necessarily cause memory to be allocated. Its
  primary function is to describe the field data to the runtime.
  Memory allocation will likely be deferred.

  @param nspace        The string namespace to use to register the variable.
  @param name          The string name of the data variable to register.
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
  inline bool flecsi_internal_unique_name(global_field) =                      \
    flecsi::data::field_interface_t::register_field<                           \
      flecsi::topology::global_topology_t,                                     \
      flecsi::data::global,                                                    \
      data_type,                                                               \
      flecsi_internal_string_hash(nspace),                                     \
      flecsi_internal_string_hash(name),                                       \
      versions,                                                                \
      flecsi::topology::global_index_space>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_get_global

  Access global data

  @param nspace        The string namespace to use to access the variable.
  @param name          The string of the data variable to access.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_get_global(nspace, name, data_type, version)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* WARNING: This macro returns a handle. Don't add terminations! */          \
  flecsi_get_field(                                                            \
    flecsi_get_topology(                                                       \
      flecsi::topology::global_topology_t, "global_client", "global_client"),  \
    nspace,                                                                    \
    name,                                                                      \
    data_type,                                                                 \
    global,                                                                    \
    version)

/*----------------------------------------------------------------------------*
  Color Topology Interface.
 *----------------------------------------------------------------------------*/

namespace flecsi {
namespace topology {

flecsi_register_topology(color_topology_t, "color_client", "color_client");

} // namespace topology
} // namespace flecsi

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
  inline bool flecsi_internal_unique_name(color_field) =                       \
    flecsi::data::field_interface_t::register_field<                           \
      flecsi::topology::color_topology_t,                                      \
      flecsi::data::color,                                                     \
      data_type,                                                               \
      flecsi_internal_string_hash(nspace),                                     \
      flecsi_internal_string_hash(name),                                       \
      versions,                                                                \
      flecsi::topology::color_index_space>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_get_color

  Access color data

  @param nspace        The string namespace to use to access the variable.
  @param name          The string of the data variable to access.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_get_color(nspace, name, data_type, version)                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* WARNING: This macro returns a handle. Don't add terminations! */          \
  flecsi_get_field(                                                            \
    flecsi_get_topology(                                                       \
      flecsi::topology::color_topology_t, "color_client", "color_client"),     \
    nspace,                                                                    \
    name,                                                                      \
    data_type,                                                                 \
    color,                                                                     \
    version)
