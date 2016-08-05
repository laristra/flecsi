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

#ifndef flecsi_serial_context_policy_h
#define flecsi_serial_context_policy_h

#include <unordered_map>
#include <functional>

#include "flecsi/utils/const_string.h"
#include "flecsi/execution/serial/runtime_driver.h"

/*!
 * \file serial/execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {
namespace execution {

struct serial_context_policy_t
{

  /*--------------------------------------------------------------------------*
   * Initialization.
   *--------------------------------------------------------------------------*/

  int initialize(int argc, char ** argv) {
    serial_runtime_driver(argc, argv);
    return 0;
  } // initialize

  /*--------------------------------------------------------------------------*
   * Function registraiton.
   *--------------------------------------------------------------------------*/

  template<typename T>
  bool register_function(const const_string_t & key, T & function)
  {
    size_t h = key.hash();
    if(function_registry_.find(h) == function_registry_.end()) {
      function_registry_[h] =
        reinterpret_cast<std::function<void(void)> *>(&function);
      return true;
    } // if

    return false;
  } // register_function
  
  std::function<void(void)> * function(size_t key)
  {
    return function_registry_[key];
  } // function

private:

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

}; // struct serial_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_serial_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
