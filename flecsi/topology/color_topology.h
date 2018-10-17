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

class color_topology_base__ : public data::data_client_t
{
public:
  using id_t = utils::id_t;
};

/*!
  color_topology__

 @ingroup color-topology
 */
class color_topology__ : public color_topology_base__
{

public:
  using type_identifier_t = color_topology__;

  //! Constructor
  color_topology__() {} // color_topology__()

  //! Copy constructor
  color_topology__(const color_topology__ & m) {}

}; // class color_topology__

} // namespace topology
} // namespace flecsi
