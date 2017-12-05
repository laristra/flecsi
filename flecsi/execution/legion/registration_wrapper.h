/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_registration_wrapper_h
#define flecsi_execution_legion_registration_wrapper_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 14, 2017
//----------------------------------------------------------------------------//

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
  #error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The registration_wrapper__ type selects between void and non-void
//! return values for task registration.
//!
//! @tparam RETURN The return type of the task.
//! @tparam TASK   The function pointer template type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  typename RETURN,
  RETURN (*TASK)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__
{
  //--------------------------------------------------------------------------//
  //! This method registers the given task with the Legion runtime.
  //!
  //! @tparam ARGS The variadic argument pack.
  //--------------------------------------------------------------------------//

  static void register_task(
    const Legion::TaskID tid,
    const Legion::Processor::Kind &processor,
    const Legion::TaskConfigOptions config_options,
    std::string & task_name) {

    { 
      Legion::TaskVariantRegistrar registrar(tid, task_name.c_str());
      registrar.add_constraint(Legion::ProcessorConstraint(processor));
      registrar.set_leaf(config_options.leaf);
      registrar.set_inner(config_options.inner);
      registrar.set_idempotent(config_options.idempotent);
      Legion::Runtime::preregister_task_variant<RETURN, TASK>(registrar,
        task_name.c_str());
    }//scope
 
  } // register_task
}; // struct registration_wrapper__

//----------------------------------------------------------------------------//
//! Partial specialization of registration_wrapper__ for void return type.
//!
//! @tparam TASK   The function pointer template type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

template<
  void (*TASK)(
    const Legion::Task *,
    const std::vector<Legion::PhysicalRegion> &,
    Legion::Context,
    Legion::Runtime *
  )
>
struct registration_wrapper__<void, TASK>
{

  static void register_task(
    const Legion::TaskID tid,
    const Legion::Processor::Kind &processor, 
    const Legion::TaskConfigOptions config_options,
    std::string &task_name) {

    {
      Legion::TaskVariantRegistrar registrar(tid, task_name.c_str());
      registrar.add_constraint(Legion::ProcessorConstraint(processor));
      registrar.set_leaf(config_options.leaf);
      registrar.set_inner(config_options.inner);
      registrar.set_idempotent(config_options.idempotent);
      Legion::Runtime::preregister_task_variant<TASK>(registrar,
        task_name.c_str());
    }//scope

  } // register_task
}; // struct registration_wrapper__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
