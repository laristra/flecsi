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

///
// \file serial/global.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
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
// \brief global_accessor_t provides logically array-based access to data
//        variables that have been registered in the data model.
//
// \tparam T The type of the data variable. If this type is not
//           consistent with the type used to register the data, bad things
//           can happen. However, it can be useful to reinterpret the type,
//           e.g., when writing raw bytes. This class is part of the
//           low-level \e flecsi interface, so it is assumed that you
//           know what you are doing...
// \tparam MD The meta data type.
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

  global_accessor_t() {}

  ///
  // Constructor.
  //
  // \param label The c_str() version of the const_string_t used for
  //              this data variable's hash.
  // \param size The size of the associated index space.
  // \param data A pointer to the raw data.
  // \param user_meta_data A reference to the user-defined meta data.
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
    user_meta_data_(user_meta_data),
    user_attributes_(user_attributes)
  {}

	///
  // Copy constructor.
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
	///
  // \brief Return the user meta data for this data variable.
	///
  const user_meta_data_t &
  meta_data() const
  {
    return user_meta_data_;
  } // meta_data

  ///
  //
  ///
  bitset_t &
  attributes()
  {
    return user_attributes_;
  } // attributes
  ///
  //
  ///
  const T *
  operator -> () const
  {
    return data_;
  } // operator ->

  ///
  //
  ///
  T *
  operator -> ()
  {
    return data_;
  } // operator ->

  ///
  // \brief Test to see if this accessor is empty.
  //
  // \return true if registered.
  ///
  operator bool() const
  {
    return data_ != nullptr;
  } // operator bool

private:

  std::string label_ = "";
  T * data_ = nullptr;
  const user_meta_data_t & user_meta_data_ =
    *(std::make_unique<user_meta_data_t>());
  bitset_t & user_attributes_ =
    *(std::make_unique<bitset_t>());

}; // struct global_accessor_t

//----------------------------------------------------------------------------//
// Scalar handle.
//----------------------------------------------------------------------------//

template<typename T>
struct global_handle_t : public data_handle_t
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

  template<typename T>
  using handle_t = global_handle_t<T>;

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  // \tparam T Data type to register.
  // \tparam NS Namespace.
  // \tparam Args Variadic arguments that are passed to
  //              metadata initialization.
  //
  // \param data_store A reference for accessing the low-level data.
  // \param key A const string instance containing the variable name.
  // \param runtime_namespace The runtime namespace to be used.
  // \param The number of variable versions for this datum.
  ///
  template< 
    typename T,
    size_t NS,
    typename ... Args
  >
  static
  handle_t<T>
  register_data(
    data_client_t & data_client,
    data_store_t & data_store,
    const const_string_t & key,
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

  ///
  //
  ///
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    data_client_t & data_client,
    data_store_t & data_store,
    const const_string_t & key,
    size_t version
  )
  {
    const size_t h = key.hash() ^ data_client.runtime_id();
    auto search = data_store[NS].find(h);

    if(search == data_store[NS].end()) {
      return {};
    }
    else {
      auto & meta_data = search->second;
          
      // check that the requested version exists.
      assert(meta_data.versions > version && "version out-of-range");

      return { meta_data.label,
        reinterpret_cast<T *>(&meta_data.data[version][0]),
        meta_data.user_data, meta_data.attributes[version] };
    } // if
  } // get_accessor

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  template<
    typename T,
    size_t NS
  >
  static
  handle_t<T>
  get_handle(
    data_client_t & data_client,
    data_store_t & data_store,
    const const_string_t & key
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
