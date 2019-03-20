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

/*!
  @file

 */

#include <flecsi/data/common/data_reference.h>

#ifndef POLICY_NAMESPACE
#error You must define a data policy namespace before including this file.
#endif

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

/*!
  Base topology type for topology-specific specializations. Specializations of
  this type must implement a get_topology_handle method.
 */

template<typename TOPOLOGY_TYPE>
struct topology_u {};

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
