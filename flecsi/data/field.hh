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
#include <flecsi/data/common/storage_classes.hh>
#include <flecsi/data/common/storage_label.hh>
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

template<typename DATA_TYPE,
  storage_label_t STORAGE_CLASS,
  typename TOPOLOGY_TYPE,
  size_t INDEX_SPACE>
struct field_member_u {

  using topology_reference_t = topology_reference_u<TOPOLOGY_TYPE>;

  template<size_t ... PRIVILEGES>
  using accessor = typename storage_class_u<STORAGE_CLASS, TOPOLOGY_TYPE>::
    template accessor<DATA_TYPE, privilege_pack_u<PRIVILEGES ...>::value>;

  field_member_u()
    : fid_(register_field()) {}

  /*!
    Return a reference to the field instance associated with the given topology
    reference.

    @param topology_reference A reference to a valid topology instance.
   */

  field_reference_t operator()(topology_reference_t const & topology_reference) const {

    return {fid_, topology_reference.identifier()};
  } // operator()

private:
  /*!
    Register this field definition with the runtime.
   */

  field_id_t register_field() const {

    field_id_t fid = unique_fid_t::instance().next();

    execution::context_t::instance().add_field_info(
      TOPOLOGY_TYPE::type_identifier_hash,
      STORAGE_CLASS,
      {fid, INDEX_SPACE, sizeof(DATA_TYPE)},
      fid);

    return fid;
  } // register_field

  field_id_t fid_;

}; // struct field_member_u

} // namespace data
} // namespace flecsi
