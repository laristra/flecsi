/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Apr 11, 2017
//----------------------------------------------------------------------------//

#include "flecsi/execution/legion/legion_tasks.h"

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/utils/common.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! Register the top-level SMPD task.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(spmd_task, processor_type_t::loc, index | inner);

//----------------------------------------------------------------------------//
//! Register task to handoff to the MPI runtime.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(handoff_to_mpi_task, processor_type_t::loc, index | leaf);

//----------------------------------------------------------------------------//
//! Register task to wait on the MPI runtime.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(wait_on_mpi_task, processor_type_t::loc, index | leaf);

//----------------------------------------------------------------------------//
//! Register task to unset the active state for the MPI runtime.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(unset_call_mpi_task, processor_type_t::loc, index | leaf);

//----------------------------------------------------------------------------//
//! Register compaction task.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(compaction_task, processor_type_t::loc, index | leaf);

//----------------------------------------------------------------------------//
//! Register fix_ghost_refs task.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(fix_ghost_refs_task, processor_type_t::loc, index | leaf);

//----------------------------------------------------------------------------//
//! Register ghost_copy task.
//!
//! \remark The translation unit that contains this call will not be
//!         necessary with C++17, as it will be possible to move this call
//!         into the header file using inline variables.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_register_legion_task(ghost_copy_task, processor_type_t::loc, index | leaf);

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
