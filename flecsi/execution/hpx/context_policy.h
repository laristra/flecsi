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

#include <cinchlog.h>
#include <flecsi-config.h>

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

  using field_id_t = size_t;

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
      size_t KEY>
  bool register_function() {
    clog_assert(
        function_registry_.find(KEY) == function_registry_.end(),
        "function has already been registered");

    clog(info) << "Registering function: " << FUNCTION << std::endl;

    function_registry_[KEY] = reinterpret_cast<void *>(FUNCTION);
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
  void * function(size_t key) {
    return function_registry_[key];
  } // function

  //--------------------------------------------------------------------
  // Data maps
  //--------------------------------------------------------------------
  struct index_space_data_t {
    // TODO: to be defined.
  };

   struct index_subspace_data_t {
    size_t capacity;
  };

  struct local_index_space_data_t{
    size_t size;
    size_t capacity;
  };

  auto&
  index_space_data_map()
  {
    return index_space_data_map_;
  }

  /*!
    Get the index subspace data map.
   */

  auto & index_subspace_data_map() {
    return index_subspace_data_map_;
  }

  auto&
  local_index_space_data_map()
  {
    return local_index_space_data_map_;
  }

  struct field_metadata_t {
    //TODO: to be defined
  };

  struct sparse_field_data_t {
    //TODO: to be defined
  };

  struct sparse_field_metadata_t {
    //TODO: to be defined
  };

protected:
  // Helper function for HPX start-up and shutdown
  FLECSI_EXPORT int
  hpx_main(int (*driver)(int, char * []), int argc, char * argv[]);

  // Start the HPX runtime system,
  FLECSI_EXPORT int
  start_hpx(int (*driver)(int, char * []), int argc, char * argv[]);

private:
  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
  //  std::map<
  //    size_t,
  //    task_info_t
  //  > task_registry_;

  // Function registry
  std::unordered_map<size_t, void *> function_registry_;

  std::map<field_id_t, std::vector<uint8_t>> field_data;
  std::map<field_id_t, field_metadata_t> field_metadata;

  std::map<size_t, index_space_data_t> index_space_data_map_;
  std::map<size_t, local_index_space_data_t> local_index_space_data_map_;
  std::map<size_t, index_subspace_data_t> index_subspace_data_map_;

  std::map<field_id_t, sparse_field_data_t> sparse_field_data;
  std::map<field_id_t, sparse_field_metadata_t> sparse_field_metadata;

}; // struct hpx_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
