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
// Color topology.
//----------------------------------------------------------------------------//

class color_topology_base_t {};

class color_topology_base__ : public data::data_client_t,
                               public color_topology_base_t {
public:
  using id_t = utils::id_t;
};


/*!
  color_topology__ 

 @ingroup color-topology
 */
class color_topology__
    : public color_topology_base__ {

public: 
  using type_identifier_t = color_topology__;


#if 0
  //! don't allow mesh to be assigned
  global_topology__ & operator=(const global_topology__ &) = delete;

  //! Allow move operations
  global_topology__(global_topology__ && o) = default;

  //! override default move assignement
  global_topology__ & operator=(global_topology__ && o) = default;
#endif

  //! Constructor, takes as input a mesh storage or storage can later be set
  color_topology__() {
  } // color_topology__()

  //! Copy constructor: alias another color topology
  color_topology__(const color_topology__ & m) {}

}; // class color_topology__

} // namespace topology
} // namespace flecsi
