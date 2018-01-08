/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi/topology/set_storage.h>
#include <flecsi/topology/set_types.h>
#include <flecsi/topology/set_utils.h>

namespace flecsi {
namespace topology {

template<typename SET_TYPE>
class set_topology__ : public set_topology_base__<set_storage__<SET_TYPE>> {
public:
  static const size_t num_index_spaces =
      std::tuple_size<typename SET_TYPE::entity_types>::value;

  using storage_t = set_storage__<SET_TYPE>;

  using base_t = set_topology_base__<storage_t>;

  using id_t = utils::id_t;

  using type_identifier_t = set_topology__;

  // used to find the entity type for a given index space
  template<size_t INDEX_SPACE>
  using entity_type = typename find_set_entity_<SET_TYPE, INDEX_SPACE>::type;

  set_topology__() {}

  set_topology__(const set_topology__ & s) : base_t(s) {}

  void initialize_storage() {}

  template<size_t INDEX_SPACE>
  auto entities() {
    using etype = entity_type<INDEX_SPACE>;
    return base_t::ss_->index_spaces[INDEX_SPACE].template slice<etype *>();
  } // entities
};

} // namespace topology
} // namespace flecsi
