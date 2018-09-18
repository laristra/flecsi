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

#include <flecsi/execution/legion/legion_tasks.h>

#include <cinchlog.h>
#include <limits>

#include <flecsi/execution/context.h>
#include <flecsi/utils/common.h>

namespace flecsi {
namespace execution {

/*!
  Register the task to setup the context for each MPI rank.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    setup_rank_context_task,
    processor_type_t::loc,
    index | inner);

/*!
  Register task to handoff to the MPI runtime.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    handoff_to_mpi_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register task to wait on the MPI runtime.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    wait_on_mpi_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register task to unset the active state for the MPI runtime.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    unset_call_mpi_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register compaction task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    owner_pos_compaction_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register fix_ghost_refs task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    owner_pos_correction_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register ghost_copy task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    ghost_copy_task,
    processor_type_t::loc,
    index | leaf);

/*!
  Register owners_subregions task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    owners_subregions_task,
    processor_type_t::loc,
    index| leaf);

/*!
  Register init mesh index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_mesh_task,
    processor_type_t::loc,
    index | leaf);
	
/*!
  Register init adjacency index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_adjacency_task,
    processor_type_t::loc,
    index | leaf);
	
/*!
  Register init vertex color index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_reduction_task(
    init_vertex_color_task,
    processor_type_t::loc,
    index | leaf,
    MinReductionPointOp::redop_id);

/*!
  Register verify vertex color index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */	
  	
__flecsi_internal_register_legion_task(
    verify_vertex_color_task,
    processor_type_t::loc,
    index | leaf);
	
/*!
  Register init offset index task.
  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.
  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_entity_offset_task,
    processor_type_t::loc,
    index | leaf);
      
/*!
  Register verify dependent partition index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    verify_dp_task,
    processor_type_t::loc,
    index | leaf);
    
/*!
  Register init cell index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_cell_task,
    processor_type_t::loc,
    index | leaf);
    
/*!
  Register init non_cell index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_non_cell_task,
    processor_type_t::loc,
    index | leaf);
    
/*!
  Register init cell_to_cell index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_cell_to_cell_task,
    processor_type_t::loc,
    index | leaf);
    
/*!
  Register init cell_to_others index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    init_cell_to_others_task,
    processor_type_t::loc,
    index | leaf);
    
/*!
  Register set entity offset index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    set_entity_offset_task,
    processor_type_t::loc,
    index | leaf);
      
/*!
  Register output_partition index task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    output_partition_task,
    processor_type_t::loc,
    index | leaf);

const double MaxReductionOp::identity = std::numeric_limits<double>::min();

template<>
void
MaxReductionOp::apply<true>(LHS & lhs, RHS rhs) {
  lhs = std::max(lhs, rhs);
}

template<>
void
MaxReductionOp::apply<false>(LHS & lhs, RHS rhs) {
  int64_t * target = (int64_t *)&lhs;
  union {
    int64_t as_int;
    double as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::max(oldval.as_T, rhs);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

template<>
void
MaxReductionOp::fold<true>(RHS & rhs1, RHS rhs2) {
  rhs1 = std::max(rhs1, rhs2);
}

template<>
void
MaxReductionOp::fold<false>(RHS & rhs1, RHS rhs2) {
  int64_t * target = (int64_t *)&rhs1;
  union {
    int64_t as_int;
    double as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::max(oldval.as_T, rhs2);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

const double MinReductionOp::identity = std::numeric_limits<double>::max();

template<>
void
MinReductionOp::apply<true>(LHS & lhs, RHS rhs) {
  lhs = std::min(lhs, rhs);
}

template<>
void
MinReductionOp::apply<false>(LHS & lhs, RHS rhs) {
  int64_t * target = (int64_t *)&lhs;
  union {
    int64_t as_int;
    double as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::min(oldval.as_T, rhs);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

template<>
void
MinReductionOp::fold<true>(RHS & rhs1, RHS rhs2) {
  rhs1 = std::min(rhs1, rhs2);
}

template<>
void
MinReductionOp::fold<false>(RHS & rhs1, RHS rhs2) {
  int64_t * target = (int64_t *)&rhs1;
  union {
    int64_t as_int;
    double as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::min(oldval.as_T, rhs2);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

// reduction point
const LegionRuntime::Arrays::Point<1> MinReductionPointOp::identity = LegionRuntime::Arrays::Point<1>(21);

template<>
void
MinReductionPointOp::apply<true>(LHS & lhs, RHS rhs) {
  lhs = std::min(lhs.x[0], rhs.x[0]);
}

template<>
void
MinReductionPointOp::apply<false>(LHS & lhs, RHS rhs) {
  int64_t * target = (int64_t *)&(lhs.x[0]);
  union {
    int64_t as_int;
    long long as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::min(oldval.as_T, rhs.x[0]);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

template<>
void
MinReductionPointOp::fold<true>(RHS & rhs1, RHS rhs2) {
  rhs1 = std::min(rhs1.x[0], rhs2.x[0]);
}

template<>
void
MinReductionPointOp::fold<false>(RHS & rhs1, RHS rhs2) {
  int64_t * target = (int64_t *)&(rhs1.x[0]);
  union {
    int64_t as_int;
    long long as_T;
  } oldval, newval;
  do {
    oldval.as_int = *target;
    newval.as_T = std::min(oldval.as_T, rhs2.x[0]);
  } while (!__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
}

} // namespace execution
} // namespace flecsi
