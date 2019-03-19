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

/*!
  @file

  This file contains the C++ data model interface \em topology_interface_t
  for FleCSI topology data structures. The \em topology_interface_t type is
  a specialization of the \em topology_interface_u type on the backend
  runtime policy that is selected at compile time.
 */

#include <flecsi/data/common/topology_registration.h>
#include <flecsi/execution/context.h>
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
    static_assert(sizeof(TOPOLOGY_TYPE) == sizeof(typename TOPOLOGY_TYPE::type_identifier_t), "Topologies may not add data members");

    using registration_t = topology_registration_u<typename TOPOLOGY_TYPE::type_identifier_t, NAMESPACE, NAME>;

    const size_t type_key =
      typeid(typename TOPOLOGY_TYPE::type_identifier_t).hash_code();

    const size_t key = utils::hash::topology_hash<NAMESPACE, NAME>();

    flog(internal) << "Registering topology" << std::endl
                   << "\tname: " << name << std::endl
                   << "\ttype: "
                   << utils::demangle(
                    typeid(typename TOPOLOGY_TYPE::type_identifier_t).name())
                   << std::endl;

    if(!execution::context_t::instance().register_topology(
         type_key, key, registration_t::register_callback)) {
      return false;
    } // if

    return true;
  } // register_topology

  /*!
    Return a handle to a topology.

    @tparam TOPOLOGY_TYPE The topology type.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
   */

  template<typename TOPOLOGY_TYPE, size_t NAMESPACE, size_t NAME>
  static decltype(auto) get_topology_handle() {
    return DATA_POLICY::template get_topology_handle<TOPOLOGY_TYPE, NAMESPACE,
      NAME>();
  } // get_topology_handle

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
