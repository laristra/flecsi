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

#ifndef flecsi_default_scalar_h
#define flecsi_default_scalar_h

#include "flecsi/utils/const_string.h"
#include "flecsi/data/default/default_storage_type.h"
#include "flecsi/data/data_client.h"

/*!
 * \file default_scalar.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi
{
namespace data_model
{
namespace default_storage_policy
{

/*----------------------------------------------------------------------------*
 * Scalar accessor.
 *----------------------------------------------------------------------------*/

template<typename T, typename MD>
struct scalar_accessor_t {

	/*--------------------------------------------------------------------------*
	 * Type definitions.
	 *--------------------------------------------------------------------------*/

	using meta_data_t = MD;
	using user_meta_data_t = typename meta_data_t::user_meta_data_t;

	/*--------------------------------------------------------------------------*
	 * Constructors.
	 *--------------------------------------------------------------------------*/

	scalar_accessor_t() {}

	scalar_accessor_t(const std::string & label, T * data,
		const user_meta_data_t & meta_data)
		: label_(label), data_(data), meta_data_(meta_data) {}

	const T * operator -> () const {
		return data_;
	} // operator ->

	T * operator -> () {
		return data_;
	} // operator ->

	/*!
		\brief Test to see if this accessor is empty.
	
		\return true if registered.
	 */
	operator bool() const {
		return data_ != nullptr;
	} // operator bool

private:

	std::string label_ = "";
	T * data_ = nullptr;
	const user_meta_data_t & meta_data_ = {};

}; // struct scalar_accessor_t

/*----------------------------------------------------------------------------*
 * Scalar handle.
 *----------------------------------------------------------------------------*/

template<typename T>
struct scalar_handle_t {
}; // struct scalar_handle_t

/*----------------------------------------------------------------------------*
 * Scalar storage type.
 *----------------------------------------------------------------------------*/

/*!
  FIXME: Scalar storage type.
 */
template<typename DS, typename MD>
struct storage_type_t<scalar, DS, MD> {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = scalar_accessor_t<T, MD>;

  template<typename T>
  using handle_t = scalar_handle_t<T>;

  /*--------------------------------------------------------------------------*
   * Data registration.
   *--------------------------------------------------------------------------*/

  /*!
    \tparam T Data type to register.
    \tparam NS Namespace.
    \tparam Args Variadic arguments that are passed to
      metadata initialization.

    \param data_store A reference for accessing the low-level data.
    \param key A const string instance containing the variable name.
    \param runtime_namespace The runtime namespace to be used.
    \param The number of variable versions for this datum.
   */
  template<typename T, size_t NS, typename ... Args>
  static handle_t<T> register_data(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key,
    size_t versions, Args && ... args) {

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

    // map is unused for this storage type
    data_store[NS][h].map.resize(0);

    return {};    
  } // register_data

  /*--------------------------------------------------------------------------*
   * Data accessors.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T, size_t NS>
  static accessor_t<T> get_accessor(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key,
		size_t version) {
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
				meta_data.user_data };
		} // if
  } // get_accessor

  /*--------------------------------------------------------------------------*
   * Data handles.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T, size_t NS>
  static handle_t<T> get_handle(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key) {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace default_storage_policy
} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_scalar_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
