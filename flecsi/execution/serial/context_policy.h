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

#ifndef flecsi_execution_serial_context_policy_h
#define flecsi_execution_serial_context_policy_h

#include <map>
#include <functional>

#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/execution/serial/runtime_driver.h"

///
/// \file serial/execution_policy.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

struct serial_context_policy_t
{
  //------------------------------------------------------------------------//
  // Initialization.
  //------------------------------------------------------------------------//

  ///
  /// Initialize the context runtime. The arguments to this method should
  /// be passed from the main function.
  ///
  /// \param argc The number of command-line arguments.
  /// \param argv The array of command-line arguments.
  ///
  /// \return Zero upon clean initialization, non-zero otherwise.
  ///
  int
  initialize(
    int argc,
    char ** argv
  )
  {
    serial_runtime_driver(argc, argv);
    return 0;
  } // initialize

  //------------------------------------------------------------------------//
  // Function registration.
  //------------------------------------------------------------------------//

  ///
  /// \tparam T The type of the function being registered.
  ///
  /// \param key A unique function identifier.
  ///
  /// \return A boolean value that is true if the registration succeeded,
  ///         false otherwise.
  ///
  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE),
    size_t KEY
  >
  bool
  register_function(
  )
  {
    clog_assert(function_registry_.find(KEY) == function_registry_.end(),
      "function has already been registered");

    clog(info) << "Registering function: " << FUNCTION << std::endl;

    function_registry_[KEY] =
      reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  ///
  /// Return the function associated with \e key.
  ///
  /// \param key The unique function identifier.
  ///
  /// \return A pointer to a std::function<void(void)> that may be cast
  ///         back to the original function type using reinterpret_cast.
  ///
  void *
  function(
    size_t key
  )
  {
    return function_registry_[key];
  } // function

private:
  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
//  std::map<
//    size_t,
//    task_info_t
//  > task_registry_;

  //------------------------------------------------------------------------//
  // Function registry
  //------------------------------------------------------------------------//

  std::unordered_map<size_t, void *>
    function_registry_;

}; // struct serial_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_serial_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
