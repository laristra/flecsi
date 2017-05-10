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
  //! Register a task variant with the runtime.
  //!
  //! @param key       The task hash key.
  //! @param variant   The processor variant of the task.
  //! @param name      The task name string.
  //! @param call_back The registration call back function.
  //--------------------------------------------------------------------------//

  bool
  register_task(
    task_hash_key_t & key,
    processor_type_t variant,
    std::string & name,
    const registration_function_t & call_back
  )
  {
    // Get the task entry. It is ok to create a new entry, and to have
    // multiple variants for each entry, i.e., we don't need to check
    // that the entry is empty.
    auto task_entry = task_registry_[key];

    // Add the variant only if it has not been defined.
    if(task_entry.find(variant) == task_entry.end()) {
      task_registry_[key][variant] = 
        std::make_tuple(unique_tid_t::instance().next(), call_back, name);
      return true;
    }

    return false;
  } // register_task

  //--------------------------------------------------------------------------//
  //! Return the task id for the task identified by \em key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  task_id_t
  task_id(
    task_hash_key_t key
  )
  {
    {
    clog_tag_guard(context);
    clog(info) << "Returning task id: " << key << std::endl;
    }

    // There is only one task variant set.
    clog_assert(key.processor().count() == 1,
      "multiple task variants given: " << key.processor());

    // The key exists.
    auto task_entry = task_registry_.find(key);
    clog_assert(task_entry != task_registry_.end(),
      "task key does not exist: " << key);

    auto mask = static_cast<processor_mask_t>(key.processor().to_ulong());
    auto variant = task_entry->second.find(mask_to_type(mask));

    clog_assert(variant != task_entry->second.end(),
      "task variant does not exist: " << key);
    
    return std::get<0>(variant->second);
  } // task_id

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  template<typename T>
  bool
  register_function(
    const utils::const_string_t & key,
    T & function
  )
  {
    size_t h = key.hash();
    if(function_registry_.find(h) == function_registry_.end()) {
      function_registry_[h] =
        reinterpret_cast<std::function<void(void)> *>(&function);
      return true;
    } // if

    return false;
  } // register_function
  
  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  std::function<void(void)> *
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

  struct task_value_hash_t
  {

    std::size_t
    operator () (
      const processor_type_t & key
    )
    const
    {
      return size_t(key);
    } // operator ()

  }; // struct task_value_hash_t

  struct task_value_equal_t
  {

    bool
    operator () (
      const processor_type_t & key1,
      const processor_type_t & key2
    )
    const
    {
      return size_t(key1) == size_t(key2);
    } // operator ()

  };

  // Define the value type for task map.
  using task_value_t =
    std::unordered_map<
      processor_type_t,
      std::tuple<
        task_id_t,
        registration_function_t,
        std::string
      >,
      task_value_hash_t,
      task_value_equal_t
    >;

  // Define the map type using the task_hash_t hash function.
  std::unordered_map<
    task_hash_t::key_t, // key
    task_value_t,       // value
    task_hash_t,        // hash function
    task_hash_t         // equivalence operator
  > task_registry_;

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

}; // class mpi_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
