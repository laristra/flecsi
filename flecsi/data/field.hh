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

/*! @file */

// This can move into the includes below as soon as storage_class_u is removed.
#include <flecsi/runtime/data_policy.hh>

#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/storage_class.hh>
#include <flecsi/execution/context.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/hash.hh>

namespace flecsi {
namespace data {

/*!
  The field_member_u type provides a mechanism to define and register
  fundamental field types with the FleCSI runtime.

  @tparam DATA_TYPE     A compact type that will be defined at each index of
                        the associated topological index space. FleCSI defines
                        a compact type as either a P.O.D. type, or a
                        user-defined type that does not have external
                        references, i.e., sizeof for a compact type is equal to
                        the logical serialization of that type.
  @tparam STORAGE_CLASS The label of the storage class for the field member.
  @tparam TOPOLOGY_TYPE A specialization of a core FleCSI topology type.
  @tparam INDEX_SPACE   The id of the index space on which to define the field.
 */

  // topology_field
template<typename DATA_TYPE,
  storage_label_t STORAGE_CLASS,
  typename TOPOLOGY_TYPE,
  size_t INDEX_SPACE>
struct field_member_u {

  using topology_reference_t = topology_reference_u<TOPOLOGY_TYPE>;

  field_member_u(size_t versions = 1)
    : versions_(versions), fid_(register_field()) {}

  /*!
    Return a reference to the field instance associated with the given topology
    reference.

    @param topology_reference A reference to a valid topology instance.
    @param version            The field version.
   */

  field_reference_t operator()(topology_reference_t const & topology_reference,
    size_t version = 0) const {

    flog_assert(version < versions_,
      "no such version #" << version << " in " << versions_ << " versions");

    return {fid_ + version, topology_reference.identifier()};
  } // operator()

private:
  /*!
    Register this field definition with the runtime.
   */

  field_id_t register_field() const {

    constexpr auto max = utils::hash::field_max_versions;

    flog_assert(versions_ <= max,
      "can't have " << versions_ << '>' << max << " versions");

    field_id_t fid;

    for(size_t v{0}; v < versions_; ++v) {
      const auto unique = unique_fid_t::instance().next();

      if(v) {
        flog_assert(unique == fid_ + v,
          "version id " << unique << " is not consecutive to fid " << fid_
                        << " with versions " << versions_);
      }
      else {
        fid = unique;
      } // if

      execution::context_t::instance().add_field_info(
        TOPOLOGY_TYPE::type_identifier_hash,
        STORAGE_CLASS,
        {fid, INDEX_SPACE, versions_, sizeof(DATA_TYPE)},
        fid);
    } // for
  } // register_field

  size_t versions_;
  field_id_t fid_;

}; // struct field_member_u

} // namespace data
} // namespace flecsi
