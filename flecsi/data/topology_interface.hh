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

/*!
  @file

  This file contains the C++ data model interface \em topology_interface_t
  for FleCSI topology data structures. The \em topology_interface_t type is
  a specialization of the \em topology_interface type on the backend
  runtime policy that is selected at compile time.
 */

#include <flecsi/runtime/data_policy.hh>

#include "../topology/common/core.hh"
#include <flecsi/data/common/topology_registration.hh>
#include <flecsi/execution/context.hh>
#include <flecsi/topology/base_topology_types.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/hash.hh>

#include <string>

namespace flecsi {
namespace data {

/*!
  The topology_interface type defines a high-level topology
  interface that is implemented by the given data policy.

  @tparam DATA_POLICY The backend runtime policy.

  @ingroup data
 */

struct topology_interface_t {

  /*!
    Return a topology reference.

    @tparam TOPOLOGY_TYPE The topology type.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of topology instance names to avoid collisions.
    @tparam NAME          The topology instance name.
   */

  template<typename TOPOLOGY_TYPE, size_t NAMESPACE, size_t NAME>
  static decltype(auto) reference(std::string const & name) {
    using Core = topology::core_t<TOPOLOGY_TYPE>;
    static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(Core),
      "Topologies may not add data members");

    using registration_t = topology_registration<Core, NAMESPACE, NAME>;

    using topology_reference_t = topology_reference<Core>;

    registration_t::register_fields();

    return topology_reference_t(utils::hash::topology_hash<NAMESPACE, NAME>());
  } // reference

  /*!
    Set the coloring for an index topology instance.

    @tparam TOPOLOGY_TYPE The topology type, which must be derived from
                          one of the FleCSI core topology types, e.g.,
                          unstructure_mesh_topology_base_t,
                          structured_topology_base_t,
                          ntree_topology_base_t, or set_topology_base_t.
    @tparam NAMESPACE     The namespace of the topology instance for which
                          to add a coloring.
    @tparam NAME          The name of the topology instance for which
                          to add a coloring.

    @param name     The unhashed name of the coloring for debugging purposes.
                    In general, this interface is not meant to be called
                    directly.
    @param coloring A valid coloring instance for the given topology type.
   */

  template<typename TOPOLOGY_TYPE>
  void create(topology_reference<TOPOLOGY_TYPE> const & topology_reference,
    typename TOPOLOGY_TYPE::coloring_t const & coloring,
    std::string const & name) {

    data_policy_t::create<TOPOLOGY_TYPE>(topology_reference, coloring);

  } // add_coloring

  template<typename TOPOLOGY_TYPE>
  void destroy(topology_reference<TOPOLOGY_TYPE> const & topology_reference,
    std::string const & name) {

    data_policy_t::destroy<TOPOLOGY_TYPE>(topology_reference);

  } // destroy_coloring

}; // struct topology_interface

} // namespace data
} // namespace flecsi
