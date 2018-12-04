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
// Color topology.
//----------------------------------------------------------------------------//

class color_topology_base_u : public data::data_client_t
{
public:
  using id_t = utils::id_t;
};

/*!
  color_topology_u

 @ingroup color-topology
 */
class color_topology_u : public color_topology_base_u
{

public:
  using type_identifier_t = color_topology_u;

  //! Constructor
  color_topology_u() {} // color_topology_u()

  //! Copy constructor
  color_topology_u(const color_topology_u & m) {}

}; // class color_topology_u

} // namespace topology
} // namespace flecsi
