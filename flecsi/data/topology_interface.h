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
  a specialization of the \em topology_interface_u type on the backend
  runtime policy that is selected at compile time.
 */

#include <flecsi/data/common/topology_registration.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/base_topology_types.h>
#include <flecsi/utils/flog.h>
#include <flecsi/utils/hash.h>

#include <string>

namespace flecsi {
namespace data {

/*!
  The topology_interface_u type defines a high-level topology
  interface that is implemented by the given data policy.

  @tparam DATA_POLICY The backend runtime policy.

  @ingroup data
 */

template<typename DATA_POLICY>
struct topology_interface_u {

  template<typename TOPOLOGY_TYPE>
  using topology_handle_u =
    typename DATA_POLICY::template topology_handle_u<TOPOLOGY_TYPE>;

  /*!
    Register a topology with the FleCSI runtime.

    @tparam TOPOLOGY_TYPE The topology type.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
   */

  template<typename TOPOLOGY_TYPE, size_t NAMESPACE, size_t NAME>
  static bool register_topology(std::string const & name) {
    static_assert(sizeof(TOPOLOGY_TYPE) ==
                    sizeof(typename TOPOLOGY_TYPE::type_identifier_t),
      "Topologies may not add data members");

    using registration_t =
      topology_registration_u<typename TOPOLOGY_TYPE::type_identifier_t,
        NAMESPACE,
        NAME>;

    flog(internal)
      << "Registering topology" << std::endl
      << "\tname: " << name << std::endl
      << "\ttype: "
      << utils::demangle(
           typeid(typename TOPOLOGY_TYPE::type_identifier_t).name())
      << std::endl;

    return registration_t::register_fields();
  } // register_topology

  /*!
    Return a handle to a topology.

    @tparam TOPOLOGY_TYPE The topology type.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
   */

  template<typename TOPOLOGY_TYPE, size_t NAMESPACE, size_t NAME>
  static decltype(auto) get_topology() {

    using topology_t = typename DATA_POLICY::template topology_u<TOPOLOGY_TYPE>;

    return topology_t::template get_reference<NAMESPACE, NAME>();
  } // get_topology

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

  template<typename TOPOLOGY_TYPE,
    size_t NAMESPACE,
    size_t NAME>
  void set_coloring(std::string const & name,
    typename TOPOLOGY_TYPE::coloring_t & coloring) {

    auto & context_ = execution::context_t::instance();

    constexpr size_t identifier = utils::hash::topology_hash<NAMESPACE, NAME>();

    context_.set_coloring<TOPOLOGY_TYPE, identifier>(coloring);
  } // add_coloring

  /*!
    Remove a coloring.

    @tparam TOPOLOGY_TYPE The topology type, which must be derived from
                          one of the FleCSI core topology types, e.g.,
                          unstructure_mesh_topology_base_t,
                          structured_topology_base_t,
                          ntree_topology_base_t, or set_topology_base_t.
    @tparam NAMESPACE     The namespace of the topology instance for which
                          to add a coloring.
    @tparam NAME          The name of the topology instance for which
                          to add a coloring.
    @tparam COLORING_NAME The name of the coloring to add.

    @param name     The unhashed name of the coloring for debugging purposes.
                    In general, this interface is not meant to be called
                    directly.
   */

  template<typename TOPOLOGY_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t COLORING_NAME>
  void remove_coloring(std::string const & name) {

    auto & context_ = execution::context_t::instance();

    constexpr size_t identifier = utils::hash::topology_hash<NAMESPACE, NAME>();

    context_.remove_coloring<TOPOLOGY_TYPE, identifier, COLORING_NAME>();
  } // add_coloring

}; // struct topology_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/data_policy.h>

namespace flecsi {
namespace data {

/*!
  The topology_interface_t type is the high-level interface to the FleCSI
  toplogy model.

  @ingroup data
 */

using topology_interface_t = topology_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi
