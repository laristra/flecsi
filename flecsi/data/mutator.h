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

#include <flecsi/data/data_constants.h>

namespace flecsi {

// this empty base class which is the base of all accessors is used by the
// handle tuple walkers for type checking
struct mutator_base_t {};

template<data::storage_label_type_t, typename T>
struct mutator__ : public mutator_base_t {};

template<typename T>
struct mutator__<data::base, T> {}; // struct mutator__

} // namespace flecsi
