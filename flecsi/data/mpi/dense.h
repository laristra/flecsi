/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mpi_dense_h
#define flecsi_mpi_dense_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE mpi
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/data/common/data_types.h"
#include "flecsi/data/common/privilege.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/index_space.h"

#include <algorithm>
#include <memory>

///
/// \file
/// \date Initial file creation: Apr 7, 2016
///

namespace flecsi {
namespace data {
namespace mpi {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense handle.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Dense accessor.
//----------------------------------------------------------------------------//

///
/// \brief dense_accessor_t provides logically array-based access to data
///        variables that have been registered in the data model.
///
/// \tparam T The type of the data variable. If this type is not
///           consistent with the type used to register the data, bad things
///           can happen. However, it can be useful to reinterpret the type,
///           e.g., when writing raw bytes. This class is part of the
///           low-level \e flecsi interface, so it is assumed that you
///           know what you are doing...
///
template<
  typename T,
  size_t EP,
  size_t SP,
  size_t GP
>
struct dense_handle_t : public data_handle__<T, EP, SP, GP>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using base = data_handle__<T, EP, SP, GP>;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  // FIXME: calling to base class constructor?
  ///
  /// Default constructor.
  ///
  dense_handle_t() {}

	///
  /// Copy constructor.
	///
//	dense_handle_t(
//    const dense_handle_t & a
//  )
//  :
//    label_(a.label_)
//  {}

  template<size_t EP2, size_t SP2, size_t GP2>
  dense_handle_t(const dense_handle_t<T, EP2, SP2, GP2> & h)
    : base(reinterpret_cast<const base&>(h)),
      label_(h.label_)
  {}

  //--------------------------------------------------------------------------//
  // Member data interface.
  //--------------------------------------------------------------------------//

	///
  /// \brief Return a std::string containing the label of the data variable
  ///       reference by this accessor.
	///
  const std::string &
  label() const
  {
    return label_;
  } // label

	///
  /// \brief Return the index space size of the data variable
  ///        referenced by this accessor.
	///
  size_t
  size() const
  {
    return base::combined_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  exclusive_size() const
  {
    return base::exclusive_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  shared_size() const
  {
    return base::shared_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  ghost_size() const
  {
    return base::ghost_size;
  } // size

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

	///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  const T &
  operator [] (
    E * e
  ) const
  {
    return this->operator[](e->template id<0>());
  } // operator []

	///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  T &
  operator [] (
    E * e
  )
  {
    return this->operator[](e->template id<0>());
  } // operator []

	///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  const T &
  operator () (
    E * e
  ) const
  {
    return this->operator[](e->template id<0>());
  } // operator []

	///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  T &
  operator () (
    E * e
  )
  {
    return this->operator[](e->template id<0>());
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  operator [] (
    size_t index
  ) const
  {
    assert(index < base::combined_size && "index out of range");
    return base::combined_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  operator [] (
    size_t index
  )
  {
    assert(index < base::combined_size && "index out of range");
    return base::combined_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  exclusive (
    size_t index
  ) const
  {
    assert(index < base::exclusive_size && "index out of range");
    return base::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  exclusive (
    size_t index
  )
  {
    assert(index < base::exclusive_size && "index out of range");
    return base::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  shared (
    size_t index
  ) const
  {
    assert(index < base::shared_size && "index out of range");
    return base::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  shared (
    size_t index
  )
  {
    assert(index < base::shared_size && "index out of range");
    return base::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  ghost (
    size_t index
  ) const
  {
    assert(index < base::ghost_size && "index out of range");
    return base::ghost_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  ghost (
    size_t index
  )
  {
    assert(index < base::ghost_size && "index out of range");
    return base::ghost_data[index];
  } // operator []

//  ///
//  // \brief Provide logical array-based access to the data for this
//  //        data variable.  This is the const operator version.
//  //
//  // \tparam E A complex index type.
//  //
//  // This version of the operator is provided to support use with
//  // \e flecsi mesh entity types \ref mesh_entity_base_t.
//  ///
//  template<typename E>
//  const T &
//  operator () (
//    E * e
//  ) const
//  {
//    return this->operator()(e->template id<0>());
//  } // operator ()
//
//  ///
//  // \brief Provide logical array-based access to the data for this
//  //        data variable.  This is the const operator version.
//  //
//  // \tparam E A complex index type.
//  //
//  // This version of the operator is provided to support use with
//  // \e flecsi mesh entity types \ref mesh_entity_base_t.
//  ///
//  template<typename E>
//  T &
//  operator () (
//    E * e
//  )
//  {
//    return this->operator()(e->template id<0>());
//  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  operator () (
    size_t index
  ) const
  {
    assert(index < base::combined_size && "index out of range");
    return base::combined_data[index];
  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  operator () (
    size_t index
  )
  {
    assert(index < base::combined_size && "index out of range");
    return base::combined_data[index];
  } // operator ()

	///
  /// \brief Test to see if this accessor is empty
  ///
  /// \return true if registered.
  ///
  operator bool() const
  {
    return base::primary_data != nullptr;
  } // operator bool

  template<typename, size_t, size_t, size_t>
  friend class dense_handle_t;

private:
  std::string label_ = "";
}; // struct dense_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

///
/// FIXME: Dense storage type.
///
template<>
struct storage_type__<dense>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  using handle_t = dense_handle_t<T, EP, SP, GP>;
#if 0
  //--------------------------------------------------------------------------//
  // Data accessors.
  //--------------------------------------------------------------------------//

  /// \brief Return a denst_handle_t.
  ///
  /// \param [in] meta_data  The meta data to use to build the accessor.
  /// \param [in] version   The version to select.
  ///
  /// \remark In this version, the search for meta data was already done.
  template< typename T >
  static
  accessor_t<T>
  get_accessor(
    meta_data_t & meta_data,
    size_t version
  )
  {

    // check that the requested version exists
    if ( version >= meta_data.versions ) {
      std::cerr << "version out of range" << std::endl;
      std::abort();
    }

    // construct an accessor from the meta data
    return { meta_data.label, meta_data.size,
      reinterpret_cast<T *>(&meta_data.data[version][0]),
      meta_data.user_data, meta_data.attributes[version],
      meta_data.index_space };
  }

  /// \brief Return a denst_handle_t.
  ///
  /// \param [in] data_store   The data store to search.
  /// \param [in] hash   The hash to search for.
  /// \param [in] version   The version to select.
  ///
  /// \remark In this version, the search for meta data has not been done,
  ///   but the key has already been hashed.
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    data_store_t & data_store,
    const utils::const_string_t::hash_type_t & hash,
    size_t version
  )
  {
    auto search = data_store[NS].find(hash);

    if(search == data_store[NS].end()) {
      return {};
    }
    else {
      return get_accessor<T>( search->second, version );
    } // if
  } // get_accessor

  /// \brief Return a denst_handle_t.
  ///
  /// \param [in] data_client  The data client to restrict our search to.
  /// \param [in] data_store   The data store to search.
  /// \param [in] key   The key to search for.
  /// \param [in] version   The version to select.
  ///
  /// \remark In this version, the search for meta data has not been done,
  ///   and the key is still a string.
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t version
  )
  {
    auto hash = key.hash() ^ data_client.runtime_id();
    return get_accessor<T,NS>(data_store, hash, version);
  } // get_accessor


  /// \brief Return a list of all denst_handle_t's filtered by a predicate.
  ///
  /// \param [in] data_client The data client to restrict our search to.
  /// \param [in] data_store The data store to search.
  /// \param [in] version The version to select.
  /// \param [in] predicate If \e predicate(a) returns true, add the accessor
  ///                       to the list.
  /// \param [in] sorted If true, sort the results by label lexographically.
  ///
  /// \remark This version is confined to search within a single namespace.
  template<
    typename T,
    size_t NS,
    typename Predicate
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // loop over each key pair
    for (auto & entry_pair : data_store[NS]) {
      // get the meta data key and label
      const auto & meta_data_key = entry_pair.first;
      auto & meta_data = entry_pair.second;
      // now build the hash for this label
      const auto & label = meta_data.label;
      auto key_hash = 
        utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
      auto hash = key_hash ^ runtime_id;
      // filter out the accessors for different data_clients
      if ( meta_data_key != hash ) continue;
      // if the reconstructed hash matches the meta data key,
      // then we may want this one
      auto a = get_accessor<T>( meta_data, version );
      if ( a )
        if (meta_data.rtti->type_info == typeid(T) && predicate(a))
          as.emplace_back( std::move(a) );
    } // for

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  
  } // get_accessor


  /// \brief Return a list of all denst_handle_t's filtered by a predicate.
  ///
  /// \param [in] data_client  The data client to restrict our search to.
  /// \param [in] data_store   The data store to search.
  /// \param [in] version   The version to select.
  /// \param [in] predicate   If \e predicate(a) returns true, add the accessor
  ///                         to the list.
  /// \param [in] sorted  If true, sort the results by label lexographically.
  ///
  /// \remark This version searches all namespaces.
  template<
    typename T,
    typename Predicate
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // check each namespace
    for (auto & namespace_map : data_store) {

      // the namespace data
      auto & namespace_key = namespace_map.first;
      auto & namespace_data = namespace_map.second;
      
      // loop over each key pair
      for (auto & entry_pair : namespace_data) {
        // get the meta data key and label
        const auto & meta_data_key = entry_pair.first;
        auto & meta_data = entry_pair.second;
        // now build the hash for this label
        const auto & label = meta_data.label;
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
        auto hash = key_hash ^ runtime_id;
        // filter out the accessors for different data_clients
        if ( meta_data_key != hash ) continue;
        // if the reconstructed hash matches the meta data key,
        // then we may want this one
        auto a = get_accessor<T>( meta_data, version );
        if ( a )
          if (meta_data.rtti->type_info == typeid(T) && predicate(a))
            as.emplace_back( std::move(a) );
      } // for each key pair
    } // for each namespace

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  
  } // get_accessor

  /// \brief Return a list of all denst_handle_t's.
  ///
  /// \param [in] data_client  The data client to restrict our search to.
  /// \param [in] data_store   The data store to search.
  /// \param [in] version   The version to select.
  /// \param [in] sorted  If true, sort the results by label lexographically.
  ///
  /// \remark This version is confined to search within a single namespace
  template<
    typename T,
    size_t NS
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {

    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // loop over each key pair
    for (auto & entry_pair : data_store[NS]) {
      // get the meta data key and label
      const auto & meta_data_key = entry_pair.first;
      auto & meta_data = entry_pair.second;
      // now build the hash for this label
      const auto & label = meta_data.label;
      auto key_hash = 
        utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
      auto hash = key_hash ^ runtime_id;
      // filter out the accessors for different data_clients
      if ( meta_data_key != hash ) continue;
      // if the reconstructed hash matches the meta data key,
      // then we may want this one
      auto a = get_accessor<T>( meta_data, version );
      if ( a )
        if (meta_data.rtti->type_info == typeid(T))
          as.emplace_back( std::move(a) );
    } // for

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  
  } // get_accessor


  /// \brief Return a list of all denst_handle_t's.
  ///
  /// \param [in] data_client  The data client to restrict our search to.
  /// \param [in] data_store   The data store to search.
  /// \param [in] version   The version to select.
  /// \param [in] sorted  If true, sort the results by label lexographically.
  ///
  /// \remark This version searches all namespaces.
  template<
    typename T
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {

    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // check each namespace
    for (auto & namespace_map : data_store) {

      // the namespace data
      auto & namespace_key = namespace_map.first;
      auto & namespace_data = namespace_map.second;
      
      // loop over each key pair
      for (auto & entry_pair : namespace_data) {
        // get the meta data key and label
        const auto & meta_data_key = entry_pair.first;
        auto & meta_data = entry_pair.second;
        // now build the hash for this label
        const auto & label = meta_data.label;
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
        auto hash = key_hash ^ runtime_id;
        // filter out the accessors for different data_clients
        if ( meta_data_key != hash ) continue;
        // if the reconstructed hash matches the meta data key,
        // then we may want this one
        auto a = get_accessor<T>( meta_data, version );
        if ( a )
          if (meta_data.rtti->type_info == typeid(T))
            as.emplace_back( std::move(a) );
      } // for each key pair
    } // for each namespace

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  
  } // get_accessor


  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  ///
  ///
  template<
    typename T,
    size_t NS,
    size_t PS
  >
  static
  handle_t<T, PS>
  get_handle(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t version
  )
  {
    auto hash = key.hash() ^ data_client.runtime_id();
    auto& m = data_store[NS];
    auto search = m.find(hash);
    assert(search != m.end() && "invalid hash");
    auto& md = search->second;

    assert(version < md.versions && "version out of range");

    handle_t<T, PS> h;
    h.data = &md.data[version][0];
    h.size = md.size;

    return h;
  } // get_handle
#endif


  template<
    typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION
  >
  static
  handle_t<DATA_TYPE, 0, 0, 0>
  get_handle(
    const data_client_t & data_client
  )
  {
    handle_t<DATA_TYPE, 0, 0, 0> h;

    auto& context = execution::context_t::instance();
  
    using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

    // get field_info for this data handle
    auto& field_info =
      context.get_field_info(
        typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    // get color_info for this field.
    auto& color_info = (context.coloring_info(field_info.index_space)).at(context.color());

    auto& registered_field_data = context.registered_field_data();
    auto fieldDataIter = registered_field_data.find(field_info.fid);
    if (fieldDataIter == registered_field_data.end()) {
      size_t size = field_info.size * (color_info.exclusive +
                                       color_info.shared +
                                       color_info.ghost);
      // TODO: deal with VERSION
      execution::context_t::instance().register_field_data(field_info.fid,
                                                           size);
    }

    auto data = registered_field_data[field_info.fid].data();
    // populate data member of data_handle_t
    auto &hb = dynamic_cast<data_handle__<DATA_TYPE, 0, 0, 0>&>(h);

    hb.fid = field_info.fid;
    hb.index_space = field_info.index_space;
    hb.data_client_hash = field_info.data_client_hash;

    hb.exclusive_size = color_info.exclusive;
    hb.combined_data = hb.exclusive_buf = hb.exclusive_data =
      reinterpret_cast<DATA_TYPE *>(data);
    hb.combined_size = color_info.exclusive;

    hb.shared_size = color_info.shared;
    hb.shared_data = hb.shared_buf = hb.exclusive_data + hb.exclusive_size;
    hb.combined_size += color_info.shared;

    hb.ghost_size = color_info.ghost;
    hb.ghost_data = hb.ghost_buf = hb.shared_data + hb.shared_size;
    hb.combined_size += color_info.ghost;

    return h;
  }

}; // struct storage_type_t

} // namespace mpi
} // namespace data
} // namespace flecsi

#endif // flecsi_mpi_dense_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
