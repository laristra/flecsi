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

#include <flecsi/data/data_client_handle.h>

namespace flecsi {

enum class set_topology_buffer_t
{
  main,
  active
};

template<
  typename DATA_CLIENT_TYPE,
  size_t PERMISSIONS,
  typename DEPENDENT_DATA_CLIENT_TYPE,
  bool PACK,
  bool MIGRATE,
>
struct set_topology_handle__ :
public data_client_handle__<DATA_CLIENT_TYPE, PERMISSIONS>
{
  using dependent_data_client_handle_t =
    data_client_handle__<DEPENDENT_DATA_CLIENT_TYPE, PERMISSIONS>;

  dependent_data_client_handle_t dependent_handle;

  set_topology_buffer_t buffer;
};

} // namespace flecsi
