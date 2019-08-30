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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

//----------------------------------------------------------------------------//
// Pickup Kokkos defines if enabled.
//----------------------------------------------------------------------------//

#if defined(FLECSI_ENABLE_KOKKOS)
#include <Kokkos_Core.hpp>

#define FLECSI_TARGET KOKKOS_FUNCTION
#define FLECSI_INLINE_TARGET KOKKOS_INLINE_FUNCTION

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
