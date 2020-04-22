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
#include <flecsi/topology/types.h>
#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

class set_entity_t
{
public:
  using id_t = simple_id;

  size_t index_space_id() const {
    return 0;
  }
};

class set_topology_base_t
{
};

template<class STORAGE_TYPE>
class set_topology_base_u : public data::data_client_t,
                            public set_topology_base_t
{
public:
  STORAGE_TYPE storage;

  /*!
    This method should be called to construct and entity rather than
    calling the constructor directly. This way, the ability to have
    extra initialization behavior is reserved.
  */
  template<class T, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    return storage.template make<T>(std::forward<ARG_TYPES>(args)...);
  } // make
};

} // namespace topology
} // namespace flecsi
