/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_data_h
#define flecsi_data_data_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include "flecsi/data/common/privilege.h"
#include "flecsi/data/storage.h"

#define flecsi_register_data(client, nspace, name, data_type,                  \
  storage_type, ...)                                                           \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::data::storage_t::instance().register_data<                           \
    flecsi::data::storage_type, data_type,                                     \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
      client, EXPAND_AND_STRINGIFY(name), ##__VA_ARGS__)

//----------------------------------------------------------------------------//
//! @def flecsi_new_register_data
//!
//! This macro registers data with a data_client_t type. Data registration
//! creates a data attribute for the given client type. This call does
//! not necessarily cause memory to be allocated. It's primary function
//! is to describe the data to the runtime. Memory allocation will likely be
//! deferred.
//!
//! @param client_type  The \ref data_client_t type.
//! @param nspace       The namespace to use to register the variable.
//! @param name         The name of the data variable to register.
//! @param data_type    The data type to store, e.g., double or my_type_t.
//! @param storage_type The storage type for the data \ref storage_type_t.
//! @param versions     The number of versions of the data to register. This
//!                     parameter can be used to manage multiple data versions,
//!                     e.g., for new and old state.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_new_register_data(client_type, nspace, name, data_type,         \
  storage_type, index_space, versions)                                         \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the storage policy to register the data */                           \
  bool client_type ## _ ## nspace ## _ ## name ## _data_registered =           \
    flecsi::data::storage_t::instance().new_register_data<                     \
      client_type, flecsi::data::storage_type, data_type,                      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash(),        \
      index_space, versions>()

//----------------------------------------------------------------------------//
//! @def flecsi_get_handle
//!
//! This macro registers a data client with the FleCSI runtime. This call
//! does not necessarily cause memory to be allocated. It's primary function
//! is to describe the data to the runtime. Memory allocation will likely be
//! deferred. 
//!
//! @param client_type  The \ref data_client_t type.
//! @param nspace       The namespace to use to register the variable.
//! @param name         The name of the data variable to register.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_register_data_client(client_type, nspace, name)                 \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the storage policy to register the data */                           \
  bool client_type ## _ ## nspace ## _ ## name ## _data_client_registered =    \
    flecsi::data::storage_t::instance().register_data_client<                  \
      client_type,                                                             \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()>()

//----------------------------------------------------------------------------//
//! @def flecsi_get_handle
//!
//! Access data with a data_client_t instance.
//!
//! @param client       The data_client_t instance with which to access
//!                     the data.
//! @param nspace       The namespace to use to access the variable.
//! @param name         The name of the data variable to access.
//! @param data_type    The data type to access, e.g., double or my_type_t.
//! @param storage_type The storage type for the data \ref storage_type_t.
//! @param version      The version number of the data to access. This
//!                     parameter can be used to manage multiple data versions,
//!                     e.g., for new and old state.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_get_handle(client, nspace, name, data_type, storage_type,       \
  version)                                                                     \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the storage policy to get a handle to the data */                    \
  flecsi::data::storage_t::instance().get_handle<flecsi::data::storage_type,   \
    data_type,                                                                 \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),        \
    decltype(client)>(                                                         \
      client, EXPAND_AND_STRINGIFY(name), version)

//----------------------------------------------------------------------------//
//! @def flecsi_get_handles
//!
//! Get a list of all accessors in the namespace of a certain type.
//!
//! @param client       The data_client_t instance with which to access
//!                     the data.
//! @param nspace       The namespace to use to access the variables.
//! @param data_type    The data type to access, e.g., double or my_type_t.
//! @param storage_type The storage type for the data \ref storage_type_t.
//! @param version      The version number of the data to access. This
//!                     parameter can be used to manage multiple data versions,
//!                     e.g., for new and old state.
//! @param predicate    The predicate function to test an accessor against to
//!                     figure out whether it get added to the returned list.
//!
//! @remark  This version is confined to search only within a namespace.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_get_handles(client, nspace, data_type, storage_type,            \
  version, ...)                                                                \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the storage policy to get the handles to the data */                 \
  flecsi::data::storage_t::instance().get_handles<                             \
    flecsi::data::storage_type, data_type,                                     \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
    client, version, ## __VA_ARGS__ )

//----------------------------------------------------------------------------//
//! @def flecsi_get_handles_all
//!
//! Get a list of all accessors in the namespace of a certain type.
//!
//! @param client       The data_client_t instance with which to access
//!                     the data from.
//! @param data_type    The data type to access, e.g., double or my_type_t.
//! @param storage_type The storage type for the data \ref storage_type_t.
//! @param version      The version number of the data to access. This
//!                     parameter can be used to manage multiple data versions,
//!                     e.g., for new and old state.
//! @param predicate    The predicate function to test an accessor against to
//!                     figure out whether it get added to the returned list.
//!
//! @remark This version searches all namespaces.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_get_handles_all(                                                \
    client, data_type, storage_type, version, ...)                             \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the storage policy to get the handles to the data */                 \
  flecsi::data::storage_t::instance().get_handles<flecsi::data::storage_type,  \
    data_type>( client, version, ## __VA_ARGS__ )

//----------------------------------------------------------------------------//
//! @def flecsi_is_at
//!
//! Select state variables in the given virtual index space. This macro
//! defines a predicate function to select the virtual index space.
//!
//! @param index_space State data must be registered in this index space
//!                    to meet the predicate criterium. Valid index spaces
//!                    depend on the particular specialization in use.
//!
//! @return True if the state is registered in the specified
//!         virtual index space, false, otherwise.
//!
//! @remark The index_space can't match anything in the underlying
//!         storage container.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_is_at(index_space)                                              \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Define a lambda function predicate to test the index space */             \
  [](const auto & a) {                                                         \
    return a.index_space() == index_space;                                     \
  }

//----------------------------------------------------------------------------//
//! @def flecsi_has_attribute_at
//!
//! Select specific variables in an index space with an attribute.
//!
//! Predicate function to select state variables that have been tagged with
//! some attribute AND are defined in a specific virtual index space.
//!
//! @param attribute The attribute to search.
//!
//! @param index_space State data must be registered in this index space
//!                    to meet the predicate criterium. Valid index spaces
//!                    depend on the particular specialization in use.
//!
//! @return True if the state is persistent and is registered in
//!
//! @remark The index_space can't match anything in the
//!         underlying storage container.
//!
//! @return True if the state is persistent and is registered in
//!         the specified virtual index space, false, otherwise.
//!
//! @remark The index_space can't match anything in the underlying
//!         storage container.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_has_attribute_at(attribute, index_space)                        \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Define a lambda function predicate to select the index space */           \
  [](const auto & a) {                                                         \
    return a.attributes().test(attribute) &&                                   \
           a.index_space() == index_space;                                     \
  }

//----------------------------------------------------------------------------//
//! @def flecsi_has_attribute
//!
//! Test variables for a particular attribute.
//!
//! Predicate function to select state variables that have been tagged with
//! some attribute AND are defined in a specific virtual index space.
//!
//! @param attribute The attribute to search.
//!
//! @return True if the state is persistent and is registered in
//!         the specified virtual index space, false, otherwise.
//!
//! @remark The index_space can't match anything in the underlying
//!         storage container.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

#define flecsi_has_attribute(attribute)                                        \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Define a lambda predicate function to test the attribute */               \
  [](const auto & a) {                                                         \
    return a.attributes().test(attribute);                                     \
  }

#define flecsi_get_mutator(client, nspace, name, data_type, storage_type,      \
  version, slots)                                                              \
                                                                               \
  flecsi::data::storage_t::instance().get_mutator<flecsi::data::storage_type,  \
    data_type,                                                                 \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
    client, EXPAND_AND_STRINGIFY(name), slots, version)

#define flecsi_get_all_handles(client, storage_type, handles,                  \
  hashes, namespaces, versions)                                                \
                                                                               \
  flecsi::data::storage_t::instance().get_all_handles<                         \
    flecsi::data::storage_type>(client, handles, hashes, namespaces, versions)

#define flecsi_put_all_handles(client, storage_type, num_handles, handles,     \
  hashes, namespaces, versions)                                                \
                                                                               \
  flecsi::data::storage_t::instance().put_all_handles<                         \
    flecsi::data::storage_type>(client, num_handles, handles, hashes,          \
    namespaces, versions)

#endif // flecsi_data_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
