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
  Register the top-level SMPD task.

  \remark The translation unit that contains this call will not be
         necessary with C++17, as it will be possible to move this call
         into the header file using inline variables.

  @ingroup legion-execution
 */

__flecsi_internal_register_legion_task(
    spmd_task,
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
    single | leaf);

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
    single | leaf);

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

} // namespace execution
} // namespace flecsi
