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

#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/execution/mpi/runtime_driver.h"
#include "flecsi/coloring/coloring_types.h"

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

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
  // Task interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! The task_id_t type is used to uniquely identify tasks that have
  //! been registered with the runtime.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  using task_id_t = size_t;

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
  //! Adjacency triple: index space, from index space, to index space
  //--------------------------------------------------------------------------//

  using adjacency_triple_t = std::tuple<size_t, size_t, size_t>;

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

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//


  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE),
    size_t KEY
  >
  bool
  register_function()
  {
    clog_assert(function_registry_.find(KEY) == function_registry_.end(),
      "function has already been registered");

    clog(info) << "Registering function: " << FUNCTION << std::endl;

    function_registry_[KEY] =
      reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  void *
  function(
    size_t key
  )
  {
    return function_registry_[key];
  } // function

  //--------------------------------------------------------------------------//
  // Gathers info about registered data fields.
  //--------------------------------------------------------------------------//
  using field_id_t = size_t;

  struct field_info_t{
    size_t data_client_hash;
    size_t storage_type;
    size_t size;
    size_t namespace_hash;
    size_t name_hash;
    size_t versions;
    field_id_t fid;
    size_t index_space;
  }; // struct field_info_t

  //--------------------------------------------------------------------------//
  //! Register field info for index space and field id.
  //!
  //! @param index_space virtual index space
  //! @param field allocated field id
  //! @param field_info field info as registered
  //--------------------------------------------------------------------------//

  void register_field_info(field_info_t& field_info){
    field_info_vec_.emplace_back(std::move(field_info));
  }

  void register_field_data(field_info_t& field_info,
                           std::unordered_map<size_t, coloring::coloring_info_t>& infos) {
    clog(info) << "index space: " << field_info.index_space << std::endl;
    auto info = infos[rank];
    auto combined_size = 0;
    clog(info) << "number of exclusive: " << info.exclusive << std::endl;
    combined_size += info.exclusive ;

    clog(info) << "number of shared: " << info.shared << std::endl;
    combined_size += info.shared ;

    clog(info) << "number of ghost: " << info.ghost << std::endl;
    combined_size += info.ghost ;
    // TODO: VERSIONS
    field_data.insert({field_info.fid, std::vector<uint8_t>(combined_size * field_info.size)});
  }
  //--------------------------------------------------------------------------//
  //! Return registered fields
  //--------------------------------------------------------------------------//

  const std::vector<field_info_t>&
  registered_fields()
  const
  {
    return field_info_vec_;
  }

  std::vector<uint8_t> &
  registered_field_data(field_id_t fid)
  {
    return field_data[fid];
  }

  //--------------------------------------------------------------------------//
  //! Add an adjacency index space.
  //!
  //! @param index_space index space to add.
  //--------------------------------------------------------------------------//

  void
  add_adjacency_triple(const adjacency_triple_t& triple)
  {
    adjacencies_.emplace(std::get<0>(triple), triple);
  }

  //--------------------------------------------------------------------------//
  //! Return set of all adjacency index spaces.
  //--------------------------------------------------------------------------//

  auto&
  adjacencies()
  const
  {
    return adjacencies_;
  }

  int rank;

private:

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
  //--------------------------------------------------------------------------//
  // Map of adjacency triples. key: adjacency index space
  //--------------------------------------------------------------------------//

  std::map<size_t, adjacency_triple_t> adjacencies_;

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, void *>
    function_registry_;

  //--------------------------------------------------------------------------//
  // Field info vector for registered fields in TLT
  //--------------------------------------------------------------------------//

  std::vector<field_info_t> field_info_vec_;

  std::map<field_id_t, std::vector<uint8_t>> field_data;


}; // class mpi_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
