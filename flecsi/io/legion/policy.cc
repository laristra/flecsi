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
/*! @file */

#include <flecsi/io/legion/policy.h>

#include <cinchlog.h>

#include <flecsi/execution/legion/internal_task.h>
#include <flecsi/utils/common.h>

namespace flecsi {
namespace io {

/*!
  Register the tasks to write checkpoint files.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-io
 */

flecsi_internal_register_legion_task(checkpoint_with_attach_task,
  processor_type_t::loc,
  inner);

flecsi_internal_register_legion_task(checkpoint_without_attach_task,
  processor_type_t::loc,
  leaf);

/*!
  Register the tasks to restart from checkpoint files.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-io
 */

flecsi_internal_register_legion_task(recover_with_attach_task,
  processor_type_t::loc,
  inner);

flecsi_internal_register_legion_task(recover_without_attach_task,
  processor_type_t::loc,
  leaf);

} // namespace io
} // namespace flecsi
