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

#include "flecsi/exec/task_attributes.hh"
#include <flecsi-config.h>

#include <cstddef>

namespace flecsi {
/*!
  Execute a task.

  @tparam TASK          The user task.
    Its parameters may be of any default-constructible,
    trivially-move-assignable, non-pointer type, any type that supports the
    Legion return-value serialization interface, or any of several standard
    containers of such types.
    If \a ATTRIBUTES specifies an MPI task, parameters need merely be movable.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS The user-specified task arguments, implicitly converted to the
    parameter types for \a TASK.
    Certain FleCSI-defined parameter types accept particular, different
    argument types that serve as selectors for information stored by the
    backend; each type involved documents the correspondence.

  \note Additional types may be supported by defining appropriate
    specializations of \c util::serial or \c util::serial_convert.  Avoid
    passing large objects to tasks repeatedly; use global variables (and,
    perhaps, pass keys to select from them) or fields.
 */

template<auto & TASK,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
auto execute(ARGS &&...);
} // namespace flecsi

//----------------------------------------------------------------------------//
// This section works with the build system to select the correct runtime
// implemenation for the task model. If you add to the possible runtimes,
// remember to edit config/packages.cmake to include a definition using
// the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
//----------------------------------------------------------------------------//

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include "flecsi/exec/leg/policy.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include "flecsi/exec/mpi/policy.hh"

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

#include "flecsi/exec/hpx/policy.hh"

#endif // FLECSI_RUNTIME_MODEL
