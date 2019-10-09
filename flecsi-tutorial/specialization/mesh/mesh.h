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

#include <iostream>
#include <vector>

#include <flecsi-tutorial/specialization/mesh/policy.h>
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Mesh Specialization
//----------------------------------------------------------------------------//

struct specialization_mesh_t
  : public flecsi::topology::mesh_topology_u<specialization_mesh_policy_t> {

  void print(const char * string) {
    std::cout << string << std::endl;
  } // print

  auto cells() {
    return entities<2, 0>();
  } // cells

  auto cells(partition_t p) {
    return entities<2, 0>(p);
  } // cells

#if 0
  template< typename E, size_t M>
  auto cells(flecsi::topology::domain_entity_u<M, E> & e) {
    return entities<2, 0>(e);
  } // cells
#endif

  auto vertices() {
    return entities<0, 0>();
  } // vertices

  template<typename E, size_t M>
  auto vertices(flecsi::topology::domain_entity_u<M, E> & e) {
    return entities<0, 0>(e);
  } // vertices

  using types_t = specialization_mesh_policy_t;

  static constexpr size_t num_domains = 1;

}; // specialization_mesh_t

using mesh_t = specialization_mesh_t;
flecsi_register_data_client(mesh_t, clients, mesh);

//----------------------------------------------------------------------------//
// Type Definitions
//----------------------------------------------------------------------------//

template<size_t PRIVILEGES>
using mesh = data_client_handle_u<mesh_t, PRIVILEGES>;

template<size_t SHARED_PRIVILEGES>
using field = dense_accessor<double, rw, SHARED_PRIVILEGES, rw>;

template<size_t SHARED_PRIVILEGES>
using ragged_field = ragged_accessor<double, rw, SHARED_PRIVILEGES, rw>;

using ragged_field_mutator = ragged_mutator<double>;

template<size_t SHARED_PRIVILEGES>
using sparse_field = sparse_accessor<double, rw, SHARED_PRIVILEGES, rw>;

using sparse_field_mutator = sparse_mutator<double>;

} // namespace tutorial
} // namespace flecsi
