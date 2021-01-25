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

#include <hpx/include/lcos.hpp>

#include <flecsi/execution/common/launch.h>

#include <functional>
#include <memory>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

/*!
 Abstract interface type for HPX futures.

 @ingroup hpx-execution
 */

template<typename R, launch_type_t launch = launch_type_t::single>
using hpx_future_u = hpx::shared_future<R>;

template<typename RETURN, launch_type_t launch>
using flecsi_future = hpx_future_u<RETURN, launch>;

} // namespace execution
} // namespace flecsi
