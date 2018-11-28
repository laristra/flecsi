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

#include <flecsi/data/data_client.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Global topology.
//----------------------------------------------------------------------------//

class global_topology_base_u : public data::data_client_t
{
public:
  using id_t = utils::id_t;
};

/*!
  global_topology_u

 @ingroup global-topology
 */
class global_topology_u : public global_topology_base_u
{

public:
  using type_identifier_t = global_topology_u;

  //! Constructor
  global_topology_u() {} // global_topology_u()

  //! Copy constructor
  global_topology_u(const global_topology_u & m) {}

}; // class global_topology_u

} // namespace topology
} // namespace flecsi
