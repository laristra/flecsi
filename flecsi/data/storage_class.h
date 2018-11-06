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

#ifndef POLICY_NAMESPACE
#error "You must define a data policy namespace before including this file."
#endif

#include <flecsi/data/data_constants.h>

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

///
/// \struct storage_class_u
///
/// \tparam T Specialization parameter.
/// \tparam ST Data store type.
/// \tparam MD Metadata type.
///
template<size_t STORAGE_CLASS>
struct storage_class_u {};

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
