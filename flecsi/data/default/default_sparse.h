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

#ifndef flecsi_default_sparse_h
#define flecsi_default_sparse_h

#include "flecsi/utils/const_string.h"
#include "flecsi/data/default/default_storage_type.h"

/*!
 * \file default_sparse.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi
{
namespace data_model
{
namespace default_storage_policy
{

/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*
 * Helper type definitions.
 *+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/

/*----------------------------------------------------------------------------*
 * Sparse accessor.
 *----------------------------------------------------------------------------*/

template<typename T, typename MD>
struct sparse_accessor_t {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/
  
  using iterator_t = index_space_t::iterator_t;
  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  /*--------------------------------------------------------------------------*
   * Constructors.
   *--------------------------------------------------------------------------*/

  sparse_accessor_t() {}

  sparse_accessor_t(const std::string & label, std::vector<size_t> * map,
    T * data, const user_meta_data_t & meta_data)
    : label_(label), map_(map), data_(data), meta_data_(meta_data) {}

  T & operator [] (size_t index) {
    return data_[map_->operator[](index)];
  } // operator []

  T * data() { return data_; }

private:

  std::string label_ = "";
  std::vector<size_t> * map_;
  T * data_ = nullptr;
  const user_meta_data_t & meta_data_ = {};
  
}; // struct sparse_accessor_t

/*----------------------------------------------------------------------------*
 * Sparse handle.
 *----------------------------------------------------------------------------*/

template<typename T>
struct sparse_handle_t {
}; // struct sparse_handle_t

/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*
 * Main type definition.
 *+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/

/*!
  FIXME: Sparse storage type.
 */
template<typename DS, typename MD>
struct storage_type_t<sparse, DS, MD> {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/

	using data_store_t = DS;
	using meta_data_t = MD;

	template<typename T>
	using accessor_t = sparse_accessor_t<T, MD>;

	template<typename T>
	using handle_t = sparse_handle_t<T>;

  /*--------------------------------------------------------------------------*
   * Data registration.
   *--------------------------------------------------------------------------*/

  template<typename T, size_t NS, typename ... Args>
  static handle_t<T> register_data(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key,
    size_t versions, size_t indeces, size_t max_non_zero_entries,
		Args && ... args) {

		size_t h = key.hash() ^ runtime_namespace;

		// Runtime assertion that this key is unique
		assert(data_store[NS].find(h) == data_store[NS].end() &&
			"key already exists");

		data_store[NS][h].user_data.initialize(std::forward<Args>(args) ...);

		data_store[NS][h].label = key.c_str();
		data_store[NS][h].size = indeces;
		data_store[NS][h].type_size = sizeof(T);
		data_store[NS][h].versions = versions;
		data_store[NS][h].rtti.reset(
			new typename meta_data_t::type_info_t(typeid(T)));

    // This implementation just uses CRS
    data_store[NS][h].map.resize(indeces+1);

    // Set initial offsets
    for(size_t i(0), o(0); i<indeces+1; ++i, o+=max_non_zero_entries) {
      data_store[NS][h].map[i] = o;
    } // for

    // Allocate space for versions
		for(size_t i(0); i<versions; ++i) {
			data_store[NS][h].data[i].resize(
				indeces * max_non_zero_entries * sizeof(T));
		} // for

		return {};
  } // register_data

  /*!
   */
  template<typename T, size_t NS>
  static accessor_t<T> get_accessor(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key,
    size_t version) {
    const size_t h = key.hash() ^ runtime_namespace;
    auto search = data_store[NS].find(h);

    if(search == data_store[NS].end()) {
      return {};
    }
    else {
      auto & meta_data = search->second;

      // check that the requested version exists
      assert(meta_data.versions > version && "version out-of-range");

      return { meta_data.label, &meta_data.map,
        reinterpret_cast<T *>(&meta_data.data[version][0]),
        meta_data.user_data };
    } // if
  } // get_accessor

  /*!
   */
  template<typename T, size_t NS>
  static handle_t<T> get_handle(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key) {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace default_storage_policy
} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_sparse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
