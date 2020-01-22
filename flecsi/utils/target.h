/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

//----------------------------------------------------------------------------//
// Pickup Kokkos defines if enabled.
//----------------------------------------------------------------------------//

#if defined(FLECSI_ENABLE_KOKKOS)
#include <Kokkos_Core.hpp>

#include <Kokkos_Vector.hpp>

namespace flecsi {
template<typename T>
using vector = Kokkos::vector<T>;
} // namespace flecsi

#define FLECSI_TARGET KOKKOS_FUNCTION
#define FLECSI_INLINE_TARGET KOKKOS_INLINE_FUNCTION

#else
#include <vector>

namespace flecsi {
template<typename T>
using vector = std::vector<T>;
} // namespace flecsi

#endif // FLECSI_ENABLE_KOKKOS

//----------------------------------------------------------------------------//
// Defaults.
//----------------------------------------------------------------------------//

#if !defined(FLECSI_TARGET)
#define FLECSI_TARGET
#endif

#if !defined(FLECSI_INLINE_TARGET)
#define FLECSI_INLINE_TARGET inline
#endif
