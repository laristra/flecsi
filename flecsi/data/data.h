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
#include <flecsi/data/common/data_types.h>
#include <flecsi/data/field.h>

/*!
  @def flecsi_register_data_client

  This macro registers a data client with the FleCSI runtime. This call
  does not necessarily cause memory to be allocated. It's primary function
  is to describe the data to the runtime. Memory allocation will likely be
  deferred.

  @param client_type  The \ref data_client_t type.
  @param nspace       The namespace to use to register the variable.
  @param name         The name of the data variable to register.

  @ingroup data
 */

#define flecsi_register_data_client(client_type, nspace, name)                 \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  bool client_type##_##nspace##_##name##_data_client_registered =              \
      flecsi::data::data_client_interface_t::register_data_client<             \
          client_type,                                                         \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),  \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()>(   \
          {EXPAND_AND_STRINGIFY(name)})

/*!
  @def flecsi_register_field

  This macro registers field data with a data_client_t type. Data
  registration creates a data attribute for the given client type.
  This call does not necessarily cause memory to be allocated. It's
  primary function is to describe the field data to the runtime.
  Memory allocation will likely be deferred.

  @param client_type   The \ref data_client_t type.
  @param nspace        The namespace to use to register the variable.
  @param name          The name of the data variable to register.
  @param data_type     The data type to store, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param versions      The number of versions of the data to register. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_register_field(                                                 \
    client_type, nspace, name, data_type, storage_class, versions, ...)        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  bool client_type##_##nspace##_##name##_data_registered =                     \
      flecsi::data::field_interface_t::register_field<                         \
          client_type, flecsi::data::storage_class, data_type,                 \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),  \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),    \
          versions, ##__VA_ARGS__>({EXPAND_AND_STRINGIFY(name)})

/*!
  @def flecsi_register_global

  This macro registers global field data.
  This call does not necessarily cause memory to be allocated. It's
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

#define flecsi_register_global(nspace, name, data_type, versions, ...)         \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to register the data */                           \
  bool client_type##_##nspace##_##name##_data_registered =                     \
      flecsi::data::field_interface_t::register_field<                         \
          flecsi::data::global_data_client_t, flecsi::data::global, data_type, \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),  \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),    \
          versions, ##__VA_ARGS__>({EXPAND_AND_STRINGIFY(name)})

/*!
  @def flecsi_get_handle

  Access data with a data_client_t instance.

  @param client        The data_client_t instance with which to access
                       the data.
  @param nspace        The namespace to use to access the variable.
  @param name          The name of the data variable to access.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.

  @ingroup data
 */

#define flecsi_get_handle(                                                     \
    client_handle, nspace, name, data_type, storage_class, version)            \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the data */                    \
  flecsi::data::field_interface_t::get_handle<                                 \
      typename flecsi::data_client_type__<decltype(client_handle)>::type,      \
      flecsi::data::storage_class, data_type,                                  \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),        \
      version>(client_handle)

/*!
  @def flecsi_get_global

  Access global data

  @param nspace        The namespace to use to access the variable.
  @param name          The name of the data variable to access.
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
  flecsi_get_handle(                                                           \
      flecsi_get_client_handle(                                                \
          flecsi::data::global_data_client_t, nspace, name),                   \
      nspace, name, data_type, global, version)

/*!
  @def flecsi_get_client_handle

  Access a data client.

  @param client       The data_client_t type.
  @param nspace       The namespace to use to access the variable.
  @param name         The name of the data variable to access.

  @ingroup data
 */

#define flecsi_get_client_handle(client_type, nspace, name)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the data client */             \
  flecsi::data::data_client_interface_t::get_client_handle<                    \
      client_type,                                                             \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()>()

/*!
  @def flecsi_get_handles

  Get a list of all accessors in the namespace of a certain type.

  @param client        The data_client_t instance with which to access
                       the data.
  @param nspace        The namespace to use to access the variables.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.
  @param predicate     The predicate function to test an accessor against to
                       figure out whether it get added to the returned list.

  @remark  This version is confined to search only within a namespace.

  @ingroup data
 */

#define flecsi_get_handles(                                                    \
    client, nspace, data_type, storage_class, version, ...)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get the handles to the data */                 \
  flecsi::data::field_interface_t::get_handles<                                \
      flecsi::data::storage_class, data_type,                                  \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(     \
      client, version, ##__VA_ARGS__)

/*!
  @def flecsi_get_handles_all

  Get a list of all accessors in the namespace of a certain type.

  @param client        The data_client_t instance with which to access
                       the data from.
  @param data_type     The data type to access, e.g., double or my_type_t.
  @param storage_class The storage type for the data \ref storage_class_t.
  @param version       The version number of the data to access. This
                       parameter can be used to manage multiple data versions,
                       e.g., for new and old state.
  @param predicate     The predicate function to test an accessor against to
                       figure out whether it get added to the returned list.

  @remark This version searches all namespaces.

  @ingroup data
 */

#define flecsi_get_handles_all(client, data_type, storage_class, version, ...) \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get the handles to the data */                 \
  flecsi::data::field_interface_t::get_handles<                                \
      flecsi::data::storage_class, data_type>(client, version, ##__VA_ARGS__)

/*!
  @def flecsi_is_at

  Select state variables in the given virtual index space. This macro
  defines a predicate function to select the virtual index space.

  @param index_space State data must be registered in this index space
                     to meet the predicate criterium. Valid index spaces
                     depend on the particular specialization in use.

  @return True if the state is registered in the specified
          virtual index space, false, otherwise.

  @remark The index_space can't match anything in the underlying
          storage container.

  @ingroup data
 */

#define flecsi_is_at(index_space)                                              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a lambda function predicate to test the index space */             \
  [](const auto & a) { return a.index_space() == index_space; }

/*!
  @def flecsi_has_attribute_at

  Select specific variables in an index space with an attribute.

  Predicate function to select state variables that have been tagged with
  some attribute AND are defined in a specific virtual index space.

  @param attribute The attribute to search.
  @param index_space State data must be registered in this index space
                     to meet the predicate criterium. Valid index spaces
                     depend on the particular specialization in use.

  @return True if the state is persistent and is registered in
          the specified virtual index space, false, otherwise.

  @remark The index_space can't match anything in the underlying
          storage container.

  @ingroup data
 */

#define flecsi_has_attribute_at(attribute, index_space)                        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a lambda function predicate to select the index space */           \
  [](const auto & a) {                                                         \
    return a.attributes().test(attribute) && a.index_space() == index_space;   \
  }

/*!
  @def flecsi_has_attribute

  Test variables for a particular attribute.

  Predicate function to select state variables that have been tagged with
  some attribute AND are defined in a specific virtual index space.

  @param attribute The attribute to search.

  @return True if the state is persistent and is registered in
          the specified virtual index space, false, otherwise.

  @remark The index_space can't match anything in the underlying
          storage container.

  @ingroup data
 */

#define flecsi_has_attribute(attribute)                                        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a lambda predicate function to test the attribute */               \
  [](const auto & a) { return a.attributes().test(attribute); }

/*!
  @def flecsi_get_mutator

  Get a mutator handle for sparse data. A mutator allows for the sparse
  allocation of new entries associated with an index as well as for
  erasing existing entries.

  @param client_handle the data client handle
  @param nspace data namespace
  @param name data names
  @param data_type data type e.g. float, long -- 
    any data type so long as it is trivially copyable
  @param storage_class storage class, e.g. dense, sparse
  @param version version number
  @param slots number of slots to use for data commit -- 
    for optimal performance this should roughly be set to the expected
    number of entries that will be inserted per index although, it is fine
    if the number of inserted entries exceeds this value.

  @return a mutator handle

  @ingroup data
 */

#define flecsi_get_mutator(                                                    \
    client_handle, nspace, name, data_type, storage_class, version, slots)     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the storage policy to get a handle to the data */                    \
  flecsi::data::field_interface_t::get_mutator<                                \
      typename flecsi::data_client_type__<decltype(client_handle)>::type,      \
      flecsi::data::storage_class, data_type,                                  \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),        \
      version>(client_handle, slots)
      
/*!
 FIXME documentation
*/
#define flecsi_get_all_handles(                                                \
    client, storage_class, handles, hashes, namespaces, versions)              \
                                                                               \
  flecsi::data::field_interface_t::get_all_handles<                            \
      flecsi::data::storage_class>(                                            \
      client, handles, hashes, namespaces, versions)

/*!
  FIXME documentation
*/
#define flecsi_put_all_handles(                                                \
    client, storage_class, num_handles, handles, hashes, namespaces, versions) \
                                                                               \
  flecsi::data::field_interface_t::put_all_handles<                            \
      flecsi::data::storage_class>(                                            \
      client, num_handles, handles, hashes, namespaces, versions)
