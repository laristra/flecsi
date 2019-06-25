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

#include <flecsi/data/common/privilege.hh>
#include <flecsi/data/field_interface.hh>
#include <flecsi/data/topology_interface.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/utils/const_string.hh>

/*----------------------------------------------------------------------------*
  General Topology Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_topology_reference

  Declare a variable of type \em type with namespace \em nspace
  and name \em name.

  This macro returns a reference to a topology instance. This call does not
  necessarily cause memory to be allocated. It's primary function is to
  describe the topology type to the runtime. Memory allocation will likely be
  deferred.

  @param type   The topology type.
  @param nspace The string namespace to use to register the variable.
  @param name   The string name to use to register the variable.

  @ingroup data
 */

#define flecsi_topology_reference(type, nspace, name)                          \
  flecsi::data::topology_interface_t::topology_reference<type,                 \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name)>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_topology_instance

  Access a topology.

  @param type   The topology type.
  @param nspace The string namespace to use to access the variable.
  @param name   The string name of the data variable to access.

  @ingroup data
 */

#define flecsi_topology_instance(type, nspace, name)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the topology */                \
  flecsi::data::topology_interface_t::topology_instance<type,                  \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name)>()

/*----------------------------------------------------------------------------*
  Field Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_add_field

  This macro registers field data with a topology type. Data registration
  creates a data attribute for the given topology type.  This call does not
  necessarily cause memory to be allocated. Its primary function is to describe
  the field data to the runtime.  Memory allocation will likely be deferred,
  and will only be done if the field is mapped into a task.

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

#define flecsi_add_field(                                                      \
  topology_type, nspace, name, data_type, storage_class, versions)             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  inline bool flecsi_internal_unique_name(field) =                             \
    flecsi::data::field_interface_t::add_field<topology_type,                  \
      flecsi::data::storage_class,                                             \
      data_type,                                                               \
      flecsi_internal_string_hash(nspace),                                     \
      flecsi_internal_string_hash(name),                                       \
      versions>({flecsi_internal_stringify(name)})

/*!
  @def flecsi_field_instance

  Access data with a topology instance.

  @param topology      The topology instance with which to access
                       the data.
  @param nspace        The string namespace to use to access the variable.
  @param name          The string name of the data variable to access.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_field_instance(                                                 \
  topology, nspace, name, storage_class, version)                              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the data */                    \
  flecsi::data::field_interface_t::field_instance<decltype(                    \
                                                    topology)::topology_t,     \
    flecsi::data::storage_class,                                               \
    flecsi_internal_string_hash(nspace),                                       \
    flecsi_internal_string_hash(name),                                         \
    version>(topology)

/*----------------------------------------------------------------------------*
  Global Topology Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_add_global_field

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

#define flecsi_add_global_field(nspace, name, data_type, versions)             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_add_field(::flecsi::topology::global_topology_t,nspace,name,          \
    data_type,global,versions)

/// The global topology.
#define flecsi_global_topology                                    \
  flecsi_topology_reference(flecsi::topology::global_topology_t,  \
                            "internal","global_topology")

/*!
  @def flecsi_global_field_instance

  Access global data

  @param nspace        The string namespace to use to access the variable.
  @param name          The string of the data variable to access.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_global_field_instance(nspace, name, version)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* WARNING: This macro returns a handle. Don't add terminations! */          \
  flecsi_field_instance(                                                       \
    flecsi_global_topology,                                                    \
    nspace,                                                                    \
    name,                                                                      \
    global,                                                                    \
    version)

/*----------------------------------------------------------------------------*
  Default Index Topology Interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_register_index_field

  This macro registers index field data.
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

#define flecsi_add_index_field(nspace, name, data_type, versions)              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_add_field(::flecsi::topology::index_topology_t,nspace,name,           \
    data_type,index,versions)

/// The default index topology.
#define flecsi_index_topology                                   \
  flecsi_topology_reference(flecsi::topology::index_topology_t, \
                            "internal","index_topology")

/*!
  @def flecsi_index_field_instance

  Access index data

  @param nspace        The string namespace to use to access the variable.
  @param name          The string of the data variable to access.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_index_field_instance(nspace, name, version)                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* WARNING: This macro returns a handle. Don't add terminations! */          \
  flecsi_field_instance(                                                       \
    flecsi_index_topology,                                                     \
    nspace,                                                                    \
    name,                                                                      \
    index,                                                                     \
    version)
