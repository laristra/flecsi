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

/*!  @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/hash.hh>
#endif

#include <cstddef>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flecsi {
namespace data {

/*!

 */

struct field_info_t {
  size_t key = std::numeric_limits<size_t>::max();
  field_id_t fid = FIELD_ID_MAX;
  size_t index_space = std::numeric_limits<size_t>::max();
  size_t type_size = std::numeric_limits<size_t>::max();
}; // struct field_info_t

/*!
  The field_info_store_t type provides storage for field_info_t instances with
  an interface to lookup the field information for a particular field in a
  variety of ways.

  This type replaces the ad-hoc methods that were being used before the 2019
  refactor.
 */

struct field_info_store_t {

  void add_field_info(field_info_t const & fi) {
    flog(internal) << "Registering field info" << std::endl
                   << "\tkey: " << fi.key << std::endl
                   << "\tfid: " << fi.fid << std::endl
                   << "\tindex_space: " << fi.index_space << std::endl
                   << "\ttype_size: " << fi.type_size << std::endl;
    data_.emplace_back(fi);
    const size_t offset = data_.size() - 1;
    fid_lookup_[fi.fid] = offset;
    key_lookup_[fi.key] = offset;
  } // add_field_info

  /*!
    Lookup field info using a hash key.

    @param KEY The hash key that uniquely identifies this field.
   */

  template<size_t KEY>
  field_info_t const & get_field_info() const {

    const auto ita = key_lookup_.find(KEY);
    flog_assert(
      ita != key_lookup_.end(), "identifier lookup failed for " << KEY);

    return data_[ita->second];
  } // get_field_info

  /*!
    Lookup field info using a hash key.

    @param key The hash key that uniquely identifies this field.
   */

  field_info_t const & get_field_info(size_t key) const {

    const auto ita = key_lookup_.find(key);
    flog_assert(ita != key_lookup_.end(), "fid lookup failed for " << key);

    return data_[ita->second];
  } // get_field_info

  /*!
    Lookup field info using the namespace, name, and version.

    @tparam NAMESPACE The namespace of the field.
    @tparam NAME      The name of the field.
    @tparam VERSION   The version of the field.
   */

  template<size_t NAMESPACE, size_t NAME, size_t VERSION>
  field_info_t const & get_field_info() const {

    constexpr size_t key =
      flecsi::utils::hash::field_hash<NAMESPACE, NAME, VERSION>();
    const auto ita = key_lookup_.find(key);
    flog_assert(ita != key_lookup_.end(), "key lookup failed for " << key);

    return data_[ita->second];
  } // get_field_info

  /*!
    Return the vector of registered fields.
   */

  std::vector<field_info_t> const & field_info() const {
    return data_;
  } // data

private:
  std::vector<field_info_t> data_;
  std::unordered_map<size_t, size_t> fid_lookup_;
  std::unordered_map<size_t, size_t> key_lookup_;

}; // struct field_info_store_t

} // namespace data
} // namespace flecsi
