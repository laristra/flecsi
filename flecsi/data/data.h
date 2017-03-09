/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_data_h
#define flecsi_data_data_h

#include "flecsi/data/storage.h"

///
/// \file
/// \date Initial file creation: Aug 01, 2016
///

///
/// \brief Register data with a data_client_t.
///
/// \param client The data_client_t instance with which to register
///               the data.
/// \param nspace The namespace to use to register the variable.
/// \param name The name of the data variable to register.
/// \param versions The number of versions of the data to register. This
///                 parameter can be used to manage multiple data versions,
///                 e.g., for new and old state.
/// \param data_type The data type to store, e.g., double or my_type_t.
/// \param storage_type The storage type for the data \ref storage_type_t.
///
/// Each storage type may have additional parameters that need to be passed
/// to this macro.
///
#define flecsi_register_data(client, nspace, name, data_type, storage_type, ...)\
	flecsi::data::storage_t::instance().register_data<                           \
    flecsi::data::storage_type, data_type,                                     \
		flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
      client, EXPAND_AND_STRINGIFY(name), ##__VA_ARGS__)

///
/// \brief Access data with a data_client_t.
///
/// \param client The data_client_t instance with which to access
///               the data from.
/// \param nspace The namespace to use to access the variable.
/// \param name The name of the data variable to access.
/// \param data_type The data type to access, e.g., double or my_type_t.
/// \param storage_type The storage type for the data \ref storage_type_t.
/// \param version  The version number of the data to access. This
///                 parameter can be used to manage multiple data versions,
///                 e.g., for new and old state.
///
#define flecsi_get_accessor(client, nspace, name, data_type, storage_type,     \
  version)                                                                     \
	flecsi::data::storage_t::instance().get_accessor<flecsi::data::storage_type, \
    data_type,                                                                 \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
      client, EXPAND_AND_STRINGIFY(name), version)


// This doesnt work because the 'nspace' is not a real type and requires 
// EXPAND_AND_STRINGIFY before the _get_accessors_select is called.
//
// #define _get_accessors_select(_1,_2,_3,_4,_5,_6,NAME,...) NAME
// #define get_accessors(...) _get_accessors_select( \
//    __VA_ARGS__, get_accesors_ns, get_all_accessors)(__VA_ARGS__)

///
/// \brief Get a list of all accessors in the namespace of a certain type.
///
/// \param client The data_client_t instance with which to access
///               the data from.
/// \param nspace The namespace to use to access the variables.
/// \param data_type The data type to access, e.g., double or my_type_t.
/// \param storage_type The storage type for the data \ref storage_type_t.
/// \param version  The version number of the data to access. This
///                 parameter can be used to manage multiple data versions,
///                 e.g., for new and old state.
/// \param predicate The predicate function to test an accessor against to
///                  figure out whether it get added to the returned list.
///
/// \remark  This version is confined to search within a namespace only.
#define flecsi_get_accessors(                                                  \
    client, nspace, data_type, storage_type, version, ...)                     \
  flecsi::data::storage_t::instance().get_accessors<flecsi::data::storage_type,\
    data_type,                                                                 \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
      client, version, ## __VA_ARGS__ )

///
/// \brief Get a list of all accessors in the namespace of a certain type.
///
/// \param client The data_client_t instance with which to access
///               the data from.
/// \param data_type The data type to access, e.g., double or my_type_t.
/// \param storage_type The storage type for the data \ref storage_type_t.
/// \param version  The version number of the data to access. This
///                 parameter can be used to manage multiple data versions,
///                 e.g., for new and old state.
/// \param predicate The predicate function to test an accessor against to
///                  figure out whether it get added to the returned list.
///
/// \remark  This version searches all namespaces.
#define flecsi_get_accessors_all(                                              \
    client, data_type, storage_type, version, ...)                             \
  flecsi::data::storage_t::instance().get_accessors<flecsi::data::storage_type,\
    data_type>( client, version, ## __VA_ARGS__ )


/// \brief Select state variables at a given attachment site.
///
/// Predicate function to select state variables that are defined at
/// a specific attachment site.
///
/// \param[in] _index_space_ State data must be registered at this site
///   to meet this predicate criterium.  Valid attachement sites are
///   documented in \ref flecsi::burton_mesh_traits_t.
///
/// \return True if the state is registered at the specified
///   attachment site, false, otherwise.
///
/// \remark The variable name _index_space_ can't match anything in the
///   underlying storage container or else things will get messy.
#define flecsi_is_at(_index_space_)                                            \
  [](const auto & a) {                                                         \
    return a.index_space() == _index_space_;                                   \
  }

/// \brief Select specific variables in an index space with an attribute.
///
/// Predicate function to select state variables that have been tagged with
/// some attribute AND are defined at a specific attachment site.
///
/// \param[in] _attribute_ The attribute to search.
///
/// \param[in] _index_space_ State data must be registered at this site
///   to meet this predicate criterium.  Valid attachement sites are
///   documented in \ref flecsi::burton_mesh_traits_t.
///
/// \return True if the state is persistent and is registered at
///   the specified attachment site, false, otherwise.
///
/// \remark The variable name _index_space_ can't match anything in the
///   underlying storage container or else things will get messy.
#define flecsi_has_attribute_at(_attribute_, _index_space_)                    \
  [](const auto & a) {                                                         \
    return a.attributes().test(_attribute_) &&                                 \
           a.index_space() == _index_space_;                                   \
  }

/// \brief Select specific variables with a particular attribute.
///
/// Predicate function to select state variables that have been tagged with
/// some attribute AND are defined at a specific attachment site.
///
/// \param[in] _attribute_ The attribute to search.
///
/// \return True if the state is persistent and is registered at
///   the specified attachment site, false, otherwise.
///
/// \remark The variable name _index_space_ can't match anything in the
///   underlying storage container or else things will get messy.
#define flecsi_has_attribute(_attribute_)                                      \
  [](const auto & a) {                                                         \
    return a.attributes().test(_attribute_);                                   \
  }


///
///
///
#define flecsi_get_mutator(client, nspace, name, data_type, storage_type,      \
  version, slots)                                                              \
  flecsi::data::storage_t::instance().get_mutator<flecsi::data::storage_type,  \
    data_type,                                                                 \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash()>(       \
    client, EXPAND_AND_STRINGIFY(name), slots, version)

///
///
///
#define flecsi_get_handle(client, nspace, name, data_type, storage_type,       \
  version, privileges)                                                         \
  flecsi::data::storage_t::instance().get_handle<flecsi::data::storage_type,   \
    data_type,                                                     \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(), \
    size_t(flecsi::data::privilege::privileges)>(       \
      client, EXPAND_AND_STRINGIFY(name), version)

#endif // flecsi_data_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
