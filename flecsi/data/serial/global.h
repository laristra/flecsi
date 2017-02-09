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

#ifndef flecsi_serial_global_h
#define flecsi_serial_global_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE serial
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/utils/const_string.h"

#include <algorithm>

///
/// \file
/// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {
namespace serial {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Scalar accessor.
//----------------------------------------------------------------------------//

///
/// \brief global_accessor_t provides logically array-based access to data
///        variables that have been registered in the data model.
///
/// \tparam T The type of the data variable. If this type is not
///           consistent with the type used to register the data, bad things
///           can happen. However, it can be useful to reinterpret the type,
///           e.g., when writing raw bytes. This class is part of the
///           low-level \e flecsi interface, so it is assumed that you
///           know what you are doing...
/// \tparam MD The meta data type.
///
template<typename T, typename MD>
struct global_accessor_t
{

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  /// Default constructor.
  global_accessor_t() = default;

  ///
  /// Constructor.
  ///
  /// \param label The c_str() version of the const_string_t used for
  ///              this data variable's hash.
  /// \param size The size of the associated index space.
  /// \param data A pointer to the raw data.
  /// \param user_meta_data A reference to the user-defined meta data.
  ///
  global_accessor_t(
    const std::string & label,
    T * data,
    const user_meta_data_t & user_meta_data,
    bitset_t & user_attributes
  )
  :
    label_(label),
    data_(data),
    user_meta_data_(&user_meta_data),
    user_attributes_(&user_attributes)
  {}

	///
  /// Copy constructor.
	///
	global_accessor_t(
    const global_accessor_t & a
  )
  :
    label_(a.label_),
    data_(a.data_),
    user_meta_data_(a.user_meta_data_),
    user_attributes_(a.user_attributes_)
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
  /// \brief Return the user meta data for this data variable.
	///
  const user_meta_data_t &
  meta_data() const
  {
    return *user_meta_data_;
  } // meta_data

  ///
  ///
  ///
  bitset_t &
  attributes()
  {
    return *user_attributes_;
  } // attributes

  const bitset_t &
  attributes() const
  {
    return *user_attributes_;
  } // attributes

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  /// \brief copy operator.
  /// \param [in] a  The accessor to copy.
  /// \return A reference to the new copy.
  global_accessor_t & operator=(const global_accessor_t & a)
  {
    label_ = a.label_;
    data_ = a.data_;
    user_meta_data_ = a.user_meta_data_;
    user_attributes_ = a.user_attributes_;
    return *this;
  } // operator =

  ///
  ///
  ///
  const T *
  operator -> () const
  {
    return data_;
  } // operator ->

  ///
  ///
  ///
  T *
  operator -> ()
  {
    return data_;
  } // operator ->

  ///  \brief Provide access to the data for this data variable.  
  ///  \remark This is the const operator version.
  const T & operator*() const
  {
    return *data_;
  }

  ///  \brief Provide access to the data for this data variable.
  T & operator*()
  {
    return *data_;
  }

  ///
  /// \brief Test to see if this accessor is empty.
  ///
  /// \return true if registered.
  ///
  operator bool() const
  {
    return data_ != nullptr;
  } // operator bool

  /// \brief Implicit conversion operator.
  /// Using explicit keyword forces users to use static_cast<T>().  But
  /// if you dont use this, then it is ambiguous
  ///   accessor<int> a, b;
  ///   int c = 2;
  ///   a = c; // ok, uses assignment
  ///   b = 3; // ok, uses assignement
  ///   c = a; // ok for non-explicit cases, uses conversion operator
  ///   c = static_cast<int>(a); // works for both explicit and implicit cases
  ///   // which one do you want? accessor/accessor assignement operator or
  ///   // do you want to convert b to int, then assign int to a?
  ///   a = b;
  /// Making this explicit forces you to have to static cast for all cases,
  /// which I think is less ambiguous
  ///   a = static_cast<int>(b);
  ///   a = static_cast<int>(c);
  explicit operator T() const
  {
    return *data_;
  }

private:

  std::string label_ = "";
  T * data_ = nullptr;
  const user_meta_data_t * user_meta_data_ = nullptr;
  bitset_t * user_attributes_ = nullptr;

}; // struct global_accessor_t

//----------------------------------------------------------------------------//
// Scalar handle.
//----------------------------------------------------------------------------//

template<typename T, size_t PS>
struct global_handle_t : public data_handle__<T, PS>
{
  using type = T;
}; // struct global_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Scalar storage type.
//----------------------------------------------------------------------------//

///
// FIXME: Scalar storage type.
///
template<typename DS, typename MD>
struct storage_type_t<global, DS, MD> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = global_accessor_t<T, MD>;

  template<typename T, size_t PS>
  using handle_t = global_handle_t<T, PS>;

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  /// \tparam T Data type to register.
  /// \tparam NS Namespace.
  /// \tparam Args Variadic arguments that are passed to
  ///              metadata initialization.
  ///
  /// \param data_store A reference for accessing the low-level data.
  /// \param key A const string instance containing the variable name.
  /// \param runtime_namespace The runtime namespace to be used.
  /// \param The number of variable versions for this datum.
  ///
  template< 
    typename T,
    size_t NS,
    typename ... Args
  >
  static
  handle_t<T, 0>
  register_data(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t versions,
    Args && ... args
  )
  {
    size_t h = key.hash() ^ data_client.runtime_id();

    // Runtime assertion that this key is unique.
    assert(data_store[NS].find(h) == data_store[NS].end() &&
      "key already exists");

    data_store[NS][h].user_data.initialize(std::forward<Args>(args) ...);

    data_store[NS][h].label = key.c_str();
    data_store[NS][h].size = 1;
    data_store[NS][h].type_size = sizeof(T);
    data_store[NS][h].versions = versions;
    data_store[NS][h].rtti.reset(
      new typename meta_data_t::type_info_t(typeid(T)));

    for(size_t i=0; i<versions; ++i) {
      data_store[NS][h].data[i].resize(sizeof(T));
    } // for

    // num_materials is unused for this storage type
    data_store[NS][h].num_entries = 0;

    return {};    
  } // register_data

  //--------------------------------------------------------------------------//
  // Data accessors.
  //--------------------------------------------------------------------------//

  /// \brief Return a global_accessor_t.
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
    return { meta_data.label,
      reinterpret_cast<T *>(&meta_data.data[version][0]),
      meta_data.user_data, meta_data.attributes[version] };  
  }

  /// \brief Return a global_accessor_t.
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

  /// \brief Return a global_accessor_t.
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
    const size_t hash = key.hash() ^ data_client.runtime_id();
    return get_accessor<T,NS>(data_store, hash, version);
  } // get_accessor

  /// \brief Return a list of all accessor_t's filtered by a predicate.
  ///
  /// \param [in] data_client  The data client to restrict our search to.
  /// \param [in] data_store   The data store to search.
  /// \param [in] version   The version to select.
  /// \param [in] predicate   If \e predicate(a) returns true, add the accessor
  ///                         to the list.
  /// \param [in] sorted  If true, sort the results by label lexographically.
  ///
  /// \remark This version is confined to search within a single namespace
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


  /// \brief Return a list of all accessor_t's filtered by a predicate.
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

  /// \brief Return a list of all accessor_t's.
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


  /// \brief Return a list of all accessor_t's.
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
    const utils::const_string_t & key
  )
  {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace serial
} // namespace data
} // namespace flecsi

#endif // flecsi_serial_global_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
