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
  size_t PERMISSIONS
>
struct set_topology_handle_base__ :
public data_client_handle__<DATA_CLIENT_TYPE, PERMISSIONS>
{
  set_topology_buffer_t buffer;
  bool pack;
  bool migrate;
};

template<
  typename DATA_CLIENT_TYPE,
  size_t PERMISSIONS,
  typename INDEPENDENT_DATA_CLIENT_TYPE
>
struct set_topology_handle__ :
public set_topology_handle_base__<DATA_CLIENT_TYPE, PERMISSIONS, PACK, MIGRATE>
{
  using independent_data_client_handle_t =
    data_client_handle__<INDEPENDENT_DATA_CLIENT_TYPE, PERMISSIONS>;

  independent_data_client_handle_t independent_handle;

};

template<
  typename DATA_CLIENT_TYPE,
  size_t PERMISSIONS
>
struct set_topology_handle__<DATA_CLIENT_TYPE, PERMISSIONS, void> :
public set_topology_handle_base__<DATA_CLIENT_TYPE, PERMISSIONS>
{

};

} // namespace flecsi
