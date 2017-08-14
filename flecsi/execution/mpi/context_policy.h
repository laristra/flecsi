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

#ifndef flecsi_execution_mpi_context_policy_h
#define flecsi_execution_mpi_context_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 4, 2016
//----------------------------------------------------------------------------//

#include <unordered_map>
#include <map>
#include <functional>
#include <cinchlog.h>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include "flecsi/coloring/coloring_types.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/mpi/runtime_driver.h"
#include "flecsi/runtime/types.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The mpi_context_policy_t is the backend runtime context policy for
//! MPI.
//!
//! @ingroup mpi-execution
//----------------------------------------------------------------------------//

struct mpi_context_policy_t
{
  //--------------------------------------------------------------------------//
  //! FleCSI context initialization. This method initializes the FleCSI
  //! runtime using MPI.
  //!
  //! @param argc The command-line argument count passed from main.
  //! @param argv The command-line argument values passed from main.
  //!
  //! @return An integer value with a non-zero error code upon failure,
  //!         zero otherwise.
  //--------------------------------------------------------------------------//

  int
  initialize(
    int argc,
    char ** argv
  );

  //--------------------------------------------------------------------------//
  //! Return the color for which the context was initialized.
  //--------------------------------------------------------------------------//

  size_t
  color()
  const
  {
    return color_;
  } // color

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! The registration_function_t type defines a function type for
  //! registration callbacks.
  //--------------------------------------------------------------------------//

  using registration_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;

  //--------------------------------------------------------------------------//
  //! The unique_tid_t type create a unique id generator for registering
  //! tasks.
  //--------------------------------------------------------------------------//

  using unique_tid_t = utils::unique_id_t<task_id_t>;

  //--------------------------------------------------------------------------//
  //! Register a task with the runtime.
  //!
  //! @param key       The task hash key.
  //! @param name      The task name string.
  //! @param call_back The registration call back function.
  //--------------------------------------------------------------------------//

//  bool
//  register_task(
//    size_t key,
//    processor_type_t processor,
//    launch_t launch,
//    std::string & name,
//    const registration_function_t & callback
//  )
//  {
//    clog(info) << "Registering task callback " << name << " with key " <<
//      key << std::endl;
//
//    clog_assert(task_registry_.find(key) == task_registry_.end(),
//      "task key already exists");
//
//    task_registry_[key] = std::make_tuple(unique_tid_t::instance().next(),
//      processor, launch, name, callback);
//
//    return true;
//  } // register_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  struct index_space_data_t {
    // TODO: to be defined.
  };
  auto&
  index_space_data_map()
  {
    return index_space_data_map_;
  }

  void register_field_data(field_id_t fid,
                           size_t size) {
    // TODO: VERSIONS
    field_data.insert({fid, std::vector<uint8_t>(size)});
  }

  std::map<field_id_t, std::vector<uint8_t>>&
  registered_field_data()
  {
    return field_data;
  }
  int rank;

private:

  int color_ = 0;

  // Define the map type using the task_hash_t hash function.
//  std::unordered_map<
//    task_hash_t::key_t, // key
//    task_value_t,       // value
//    task_hash_t,        // hash function
//    task_hash_t         // equivalence operator
//  > task_registry_;

  // Map to store task registration callback methods.
//  std::map<
//    size_t,
//    task_info_t
//  > task_registry_;

  std::map<field_id_t, std::vector<uint8_t>> field_data;

  std::map<size_t, index_space_data_t> index_space_data_map_;
}; // class mpi_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
