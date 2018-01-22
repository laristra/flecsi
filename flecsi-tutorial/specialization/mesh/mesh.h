#pragma once

#include <iostream>
#include <vector>

#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/ragged_accessor.h>
#include <flecsi/data/ragged_mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/data/sparse_mutator.h>
#include <flecsi/data/common/privilege.h>
#include <specialization/mesh/types.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Mesh Specialization
//----------------------------------------------------------------------------//

struct specialization_mesh_t :
  public flecsi::topology::mesh_topology__<specialization_mesh_policy_t>
{

  void print(const char * string) {
    std::cout << string << std::endl;
  } // print

  auto cells() {
    return entities<2,0>();
  } // cells

  auto cells(partition_t p) {
    return entities<2,0>(p);
  } // cells

#if 0
  template< typename E, size_t M>
  auto cells(flecsi::topology::domain_entity__<M, E> & e) {
    return entities<2, 0>(e);
  } // cells
#endif

  auto vertices() {
    return entities<0,0>();
  } // vertices

  template< typename E, size_t M>
  auto vertices(flecsi::topology::domain_entity__<M, E> & e) {
    return entities<0, 0>(e);
  } // vertices

}; // specialization_mesh_t

using mesh_t = specialization_mesh_t;

//----------------------------------------------------------------------------//
// Type Definitions
//----------------------------------------------------------------------------//

template<
  size_t PRIVILEGES>
using mesh = data_client_handle__<mesh_t, PRIVILEGES>;

template<
  size_t SHARED_PRIVILEGES>
using field = dense_accessor<double, rw, SHARED_PRIVILEGES, ro>;

template<
  size_t SHARED_PRIVILEGES>
using ragged_field = ragged_accessor<double, rw, SHARED_PRIVILEGES, ro>;

using ragged_field_mutator = ragged_mutator<double>;

template<
  size_t SHARED_PRIVILEGES>
using sparse_field = sparse_accessor<double, rw, SHARED_PRIVILEGES, ro>;

using sparse_field_mutator = sparse_mutator<double>;

} // namespace tutorial
} // namespace flecsi
