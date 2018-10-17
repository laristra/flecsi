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

class global_topology_base__ : public data::data_client_t
{
public:
  using id_t = utils::id_t;
};

/*!
  global_topology__

 @ingroup global-topology
 */
class global_topology__ : public global_topology_base__
{

public:
  using type_identifier_t = global_topology__;

  //! Constructor
  global_topology__() {} // global_topology__()

  //! Copy constructor
  global_topology__(const global_topology__ & m) {}

}; // class global_topology__

} // namespace topology
} // namespace flecsi
