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

#include "../topology/common/core.hh" // id
#include "flecsi/runtime/backend.hh"
#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/storage_classes.hh>
#include <flecsi/data/common/storage_label.hh>
#include <flecsi/runtime/types.hh>

namespace flecsi {
namespace data {

/*!
  The field_member type provides a mechanism to define and register
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
struct field_member {

  using topology_reference_t = topology_reference<TOPOLOGY_TYPE>;

  template<size_t... PRIVILEGES>
  using accessor = typename storage_class<STORAGE_CLASS, TOPOLOGY_TYPE>::
    template accessor<DATA_TYPE, privilege_pack<PRIVILEGES...>::value>;

  field_member() : fid_(unique_fid_t::instance().next()) {

    runtime::context_t::instance().add_field_info(topology::id<TOPOLOGY_TYPE>(),
      STORAGE_CLASS,
      {fid_, INDEX_SPACE, sizeof(DATA_TYPE)},
      fid_);
  }

  /*!
    Return a reference to the field instance associated with the given topology
    reference.

    @param topology_reference A reference to a valid topology instance.
   */

  field_reference<DATA_TYPE> operator()(
    topology_reference_t const & topology_reference) const {

    return {fid_, topology_reference.identifier()};
  } // operator()

private:
  field_id_t fid_;

}; // struct field_member

} // namespace data
} // namespace flecsi
