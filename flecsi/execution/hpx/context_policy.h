/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_hpx_context_policy_h
#define flecsi_execution_hpx_context_policy_h

#if !defined(ENABLE_HPX)
#error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/lcos/local/condition_variable.hpp>
#include <hpx/lcos/local/spinlock.hpp>
#include <hpx/runtime_fwd.hpp>

#include <functional>
#include <map>
#include <mutex>

#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/hpx/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/export_definitions.h"

///
/// \file hpx/execution_policy.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
struct hpx_context_policy_t {
  //------------------------------------------------------------------------//
  // Initialization.
  //------------------------------------------------------------------------//

  FLECSI_EXPORT hpx_context_policy_t();

  ///
  /// Initialize the context runtime. The arguments to this method should
  /// be passed from the main function.
  ///
  /// \param argc The number of command-line arguments.
  /// \param argv The array of command-line arguments.
  ///
  /// \return Zero upon clean initialization, non-zero otherwise.
  ///
  int initialize(int argc, char * argv[]) {
    // start HPX runtime system, execute driver code in the context of HPX
    return start_hpx(&hpx_runtime_driver, argc, argv);
  } // hpx_context_policy_t::initialize

protected:
  // Helper function for HPX start-up and shutdown
  FLECSI_EXPORT int
  hpx_main(int (*driver)(int, char * []), int argc, char * argv[]);

  // Start the HPX runtime system,
  FLECSI_EXPORT int
  start_hpx(int (*driver)(int, char * []), int argc, char * argv[]);
}; // struct hpx_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
