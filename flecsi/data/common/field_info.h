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
#include <flecsi/utils/flog.h>
#include <flecsi/utils/hash.h>
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
  size_t namespace_hash = std::numeric_limits<size_t>::max();
  size_t name_hash = std::numeric_limits<size_t>::max();
  size_t type_size = std::numeric_limits<size_t>::max();
  size_t versions = std::numeric_limits<size_t>::max();
  size_t fid = std::numeric_limits<size_t>::max();
  size_t index_space = std::numeric_limits<size_t>::max();
  size_t key = std::numeric_limits<size_t>::max();
}; // struct field_info_t

struct field_info_store_t {

  void register_field_info(field_info_t const & fi) {
    data_.emplace_back(fi);
    const size_t offset = data_.size() - 1;
    fid_lookup_[fi.fid] = offset;
    key_lookup_[fi.key] = offset;
  } // register_field_info

  /*!
    Lookup field info using the field id.

    @param fid The field id.
   */

  field_info_t const & get_field_info(size_t fid) const {

    const auto ita = fid_lookup_.find(fid);
    flog_assert(ita != fid_lookup_.end(), "fid lookup failed for " << fid);

    return data_[ita->second];
  } // get_field_info

  /*!
    Lookup field info using the namespace and name.

    @tparam NAMESPACE The namespace of the field.
    @tparam NAME      The name of the field.
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
    Lookup field info using a hash key.

    @tparam NAMESPACE The namespace of the field.
    @tparam NAME      The name of the field.
   */

  template<size_t IDENTIFIER>
  field_info_t const & get_field_info() const {

    const auto ita = key_lookup_.find(IDENTIFIER);
    flog_assert(ita != key_lookup_.end(),
      "identifier lookup failed for " << IDENTIFIER);

    return data_[ita->second];
  } // get_field_info

  std::vector<field_info_t> const & data() const {
    return data_;
  } // data

private:
  std::vector<field_info_t> data_;
  std::unordered_map<size_t, size_t> fid_lookup_;
  std::unordered_map<size_t, size_t> key_lookup_;

}; // struct field_info_store_t

} // namespace data
} // namespace flecsi
