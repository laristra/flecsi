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

#include <flecsi/execution/context.h>
#include <flecsi/data/data_client.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Global topology.
//----------------------------------------------------------------------------//

class global_topology_base_t {};

class global_topology_base__ : public data::data_client_t,
                               public global_topology_base_t {
public:
  using id_t = utils::id_t;
};


/*!
  global_topology__ 

 @ingroup global-topology
 */
class global_topology__
    : public global_topology_base__ {

public: 
  using type_identifier_t = global_topology__;

#if 0
  // Don't allow the mesh to be copied or copy constructed

  //! don't allow mesh to be assigned
  global_topology__ & operator=(const global_topology__ &) = delete;

  //! Allow move operations
  global_topology__(global_topology__ && o) = default;

  //! override default move assignement
  global_topology__ & operator=(global_topology__ && o) = default;
#endif
  //! Constructor, takes as input a mesh storage or storage can later be set
  global_topology__() {
  } // global_topology__()


  //! Copy constructor: alias another global topology
  global_topology__(const global_topology__ & m) {}

}; // class global_topology__

} // namespace topology
} // namespace flecsi
