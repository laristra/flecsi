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

//#include <algorithm>
//#include <array>
//#include <cassert>
//#include <cstring>
//#include <functional>
//#include <iostream>
//#include <map>
//#include <memory>
//#include <type_traits>
//#include <unordered_map>
//#include <vector>

#include <flecsi/execution/context.h>
#include <flecsi/data/data_client.h>
//#include <flecsi/topology/mesh_storage.h>
//#include <flecsi/topology/mesh_types.h>
//#include <flecsi/topology/partition.h>
//#include <flecsi/utils/common.h>
//#include <flecsi/utils/set_intersection.h>
//#include <flecsi/utils/static_verify.h>

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

#if 0
  internal_topology_base__ & operator=(const internal_topology_base__ &)
    = delete;

  /// Allow move operations
  internal_topology_base__(internal_topology_base__ &&) = default;
#endif
};


/*!
  global_topology__ 

 @ingroup global-topology
 */
class global_topology__
    : public global_topology_base__ {

public: 
  using type_identifier_t = global_topology__;

  // Don't allow the mesh to be copied or copy constructed

  //! don't allow mesh to be assigned
  global_topology__ & operator=(const global_topology__ &) = delete;

  //! Allow move operations
  global_topology__(global_topology__ && o) = default;

  //! override default move assignement
  global_topology__ & operator=(global_topology__ && o) = default;

  //! Constructor, takes as input a mesh storage or storage can later be set
  global_topology__() {
  } // global_topology__()

  //! Copy constructor: alias another mesh
  global_topology__(const global_topology__ & m) {}
#if 0
  // The mesh retains ownership of the entities and deletes them
  // upon mesh destruction
  virtual ~global_topology__() {}
#endif

}; // class mesh_topology__

} // namespace topology
} // namespace flecsi
