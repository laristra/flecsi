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
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion_stl.h>
#include <legion_utilities.h>

#include <flecsi/execution/execution.h>
#include <flecsi/execution/legion/helper.h>
#include <flecsi/execution/legion/internal_task.h>

#define PRIMARY_PART 0
#define GHOST_PART 1
#define EXCLUSIVE_PART 0
#define SHARED_PART 1
#define SUBRECT_PART 0
#define OWNER_COLOR_TAG 1
#define EXCLUSIVE_OWNER 0
#define SHARED_OWNER 1
#define PRIMARY_ACCESS 0
#define GHOST_ACCESS 1
#define UNUSED_ACCESS 2

using legion_map = Legion::STL::map<LegionRuntime::Arrays::coord_t,
  LegionRuntime::Arrays::coord_t>;
using subrect_map = Legion::STL::map<size_t, LegionRuntime::Arrays::Rect<2>>;

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

/*!
 This is the color-specific initialization function to be defined by the
 FleCSI specialization layer. This symbol will be undefined in the compiled
 library, and is intended as a place holder for the specializations's
 initialization function that will resolve the missing symbol.

 The color-specific initialization function is the second of the two
 control points that are exposed to the specialization. This function is
 responsible for populating specialization-specific data structures.

 @param argc The number of arguments in argv (passed from the command line).
 @param argv The list of arguments (passed from the command line).

 @ingroup legion-execution
*/

#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
void specialization_spmd_init(int argc, char ** argv);
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

/*!
 @def flecsi_internal_legion_task

 This macro simplifies pure Legion task definitions by filling in the
 boiler-plate function arguments.

 @param task_name   The plain-text task name.
 @param return_type The return type of the task.

 @ingroup legion-execution
*/

#define flecsi_internal_legion_task(task_name, return_type)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Legion task template */                                                   \
  inline return_type task_name(const Legion::Task * task,                      \
    const std::vector<Legion::PhysicalRegion> & regions, Legion::Context ctx,  \
    Legion::Runtime * runtime)

/*!
 Onwer pos correction task corrects the owner position reference/pointer in
 the ghost partition by reading from old location in primary position.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(owner_pos_correction_task, void) {

  assert(task->regions.size() == 2);

  using namespace Legion;

  context_t & context_ = context_t::instance();

  auto ghost_owner_pos_fid = Legion::FieldID(internal_field::ghost_owner_pos);

  size_t is_id = *((size_t *)task->args);

  auto idx_space = context_.coloring_map().at(is_id);

  Legion::Domain sh_dom = runtime->get_index_space_domain(
    ctx, task->regions[0].region.get_index_space());
  Legion::Domain gh_dom = runtime->get_index_space_domain(
    ctx, task->regions[1].region.get_index_space());

  const FieldAccessor<READ_ONLY, Point<2>, 2> sh_acc(
    regions[0], ghost_owner_pos_fid);
  const FieldAccessor<WRITE_DISCARD, Point<2>, 2> gh_acc(
    regions[1], ghost_owner_pos_fid);

  for(PointInDomainIterator<2, coord_t> pir(gh_dom); pir(); pir++) {
    Point<2> old_location = gh_acc.read(*pir);
    Point<2> new_location = sh_acc.read(old_location);
    gh_acc.write(*pir, new_location);
  }
} // owner_pos_correction_task

/*!
 Interprocess communication to pass control to MPI runtime.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

/*!
 Interprocess communication to wait for control to pass back to the Legion
 runtime.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

/*!
 Interprocess communication to unset mpi execute state.

 @ingroup legion-execution
*/

flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

/*!
 Compaction task writes new location in old location.

 @ingroup legion-execution
 */

flecsi_internal_legion_task(owner_pos_compaction_task, void) {
  const int my_color = task->index_point.point_data[0];

  assert(task->regions.size() == 3);

  using namespace Legion;

  context_t & context_ = context_t::instance();

  auto ghost_owner_pos_fid = Legion::FieldID(internal_field::ghost_owner_pos);

  size_t is_id = *((size_t *)task->args);

  auto idx_space = context_.coloring_map().at(is_id);

  Legion::Domain sh_dom = runtime->get_index_space_domain(
    ctx, task->regions[1].region.get_index_space());
  Legion::Domain gh_dom = runtime->get_index_space_domain(
    ctx, task->regions[2].region.get_index_space());

  std::vector<PhysicalRegion> comb_regions;
  comb_regions.push_back(regions[0]);
  comb_regions.push_back(regions[1]);
  comb_regions.push_back(regions[2]);

  const Legion::MultiRegionAccessor<Legion::Point<2>, 2, Legion::coord_t,
    Realm::AffineAccessor<Legion::Point<2>, 2, Legion::coord_t>>
    acc(comb_regions, ghost_owner_pos_fid, sizeof(Legion::Point<2>));

  PointInDomainIterator<2, coord_t> sh_itr(sh_dom);
  PointInDomainIterator<2, coord_t> gh_itr(gh_dom);

  for(auto shared_itr = idx_space.shared.begin();
      shared_itr != idx_space.shared.end(); ++shared_itr) {
    const flecsi::coloring::entity_info_t shared = *shared_itr;
    const Point<2> reference(shared.rank, shared.offset);
    // reference is the old location, expanded_itr.p is the new location
    acc.write(reference, *sh_itr);
    sh_itr++;
  }

  for(auto ghost_itr = idx_space.ghost.begin();
      ghost_itr != idx_space.ghost.end(); ++ghost_itr) {
    const flecsi::coloring::entity_info_t ghost = *ghost_itr;
    const Point<2> reference(ghost.rank, ghost.offset);
    // reference is where we used to point, expanded_itr.p is where ghost
    // is now
    acc.write(*gh_itr, reference);
    gh_itr++;
  } // ghost_itr

} // owner_pos_compaction_task

/*!
 Ghost copy task writes data from shared into ghost

 @ingroup legion-execution
 */

flecsi_internal_legion_task(ghost_copy_task, void) {
  using vector_t = data::row_vector_u<uint8_t>;

  const int my_color = runtime->find_local_MPI_rank();

  context_t & context = context_t::instance();

  struct args_t {
    size_t data_client_hash;
    size_t index_space;
    bool sparse;
    size_t max_entries_per_index;
  };
  args_t args = *(args_t *)task->args;

  clog_assert(regions.size() == 3, "ghost_copy_task requires 3 regions");
  clog_assert(task->regions.size() == 3, "ghost_copy_task requires 3 regions");

  clog_assert(task->regions[1].privilege_fields.size() ==
                task->regions[0].privilege_fields.size(),
    "ghost region must be same size as owner region");
  clog_assert(task->regions[2].privilege_fields.size() == 1,
    "pos region must provide ghost_owner_pos_fid");

  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto position_ref_acc = regions[2]
                            .get_field_accessor(ghost_owner_pos_fid)
                            .typeify<LegionRuntime::Arrays::Point<2>>();

  Legion::Domain owner_domain = runtime->get_index_space_domain(
    ctx, regions[0].get_logical_region().get_index_space());
  Legion::Domain ghost_domain = runtime->get_index_space_domain(
    ctx, regions[1].get_logical_region().get_index_space());

  if(!args.sparse) {
    // For each field, copy data from shared to ghost
    for(auto fid : task->regions[0].privilege_fields) {
      // Look up field info in context
      auto iitr = context.field_info_map().find(
        {args.data_client_hash, args.index_space});
      clog_assert(
        iitr != context.field_info_map().end(), "invalid index space");
      auto fitr = iitr->second.find(fid);
      clog_assert(fitr != iitr->second.end(), "invalid fid");
      const context_t::field_info_t & field_info = fitr->second;

      const Legion::FieldAccessor<READ_ONLY, char, 2, Legion::coord_t,
        Realm::AffineAccessor<char, 2, Legion::coord_t>>
        owner_acc(regions[0], fid, field_info.size);
      const Legion::FieldAccessor<WRITE_ONLY, char, 2, Legion::coord_t,
        Realm::AffineAccessor<char, 2, Legion::coord_t>>
        ghost_acc(regions[1], fid, field_info.size);

      for(Legion::Domain::DomainPointIterator itr(ghost_domain); itr; itr++) {
        // auto ghost_ptr = Legion::DomainPoint::from_point<2>(itr.p);
        auto & ghost_ptr = itr.p;
        LegionRuntime::Arrays::Point<2> owner_location =
          position_ref_acc.read(ghost_ptr);
        auto owner_ptr = Legion::DomainPoint::from_point<2>(owner_location);

        char * ptr_ghost_acc = (char *)(ghost_acc.ptr(ghost_ptr));
        char * ptr_owner_acc = (char *)(owner_acc.ptr(owner_ptr));
        memcpy(ptr_ghost_acc, ptr_owner_acc, field_info.size);
      } // for ghost_domain
    } // for fid
  }
  else { // sparse

    for(auto fid : task->regions[0].privilege_fields) {
      // Look up field info in context
      auto iitr = context.field_info_map().find(
        {args.data_client_hash, args.index_space});
      clog_assert(
        iitr != context.field_info_map().end(), "invalid index space");
      auto fitr = iitr->second.find(fid);
      clog_assert(fitr != iitr->second.end(), "invalid fid");
      const context_t::field_info_t & field_info = fitr->second;

      const Legion::FieldAccessor<READ_ONLY, char, 2, Legion::coord_t,
        Realm::AffineAccessor<char, 2, Legion::coord_t>>
        owner_acc(regions[0], fid, sizeof(vector_t));
      const Legion::FieldAccessor<WRITE_ONLY, char, 2, Legion::coord_t,
        Realm::AffineAccessor<char, 2, Legion::coord_t>>
        ghost_acc(regions[1], fid, sizeof(vector_t));

      for(Legion::Domain::DomainPointIterator itr(ghost_domain); itr; itr++) {
        auto & ghost_ptr = itr.p;
        LegionRuntime::Arrays::Point<2> owner_location =
          position_ref_acc.read(ghost_ptr);
        auto owner_ptr = Legion::DomainPoint::from_point<2>(owner_location);
        vector_t * ptr_ghost_acc =
          reinterpret_cast<vector_t *>(ghost_acc.ptr(itr.p));
        const vector_t * ptr_owner_acc =
          reinterpret_cast<const vector_t *>(owner_acc.ptr(owner_ptr));

        const auto serdez_op = context.get_serdez(fid);
        assert(serdez_op);
        serdez_op->deep_copy(ptr_owner_acc, ptr_ghost_acc);

      } // for ghost_domain
    } // for fids

  } // if
} // ghost_copy_task

/*!
 Owners subregions task returns subrects required from every neighbor

 @ingroup legion-execution
 */

flecsi_internal_legion_task(owners_subregions_task, subrect_map) {
  const int my_color = runtime->find_local_MPI_rank();
  // clog(error) << "rank " << my_color << " owners_subregions_task" <<
  // std::endl;

  clog_assert(regions.size() == 1, "owners_subregions_task requires 1 region");
  clog_assert(
    task->regions.size() == 1, "owners_subregions_task requires 1 region");
  clog_assert(task->regions[0].privilege_fields.size() == 1,
    "owners_subregions_task only requires ghost_owner_pos_fid");

  legion_map owner_map = task->futures[0].get_result<legion_map>();

  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto position_ref_acc = regions[0]
                            .get_field_accessor(ghost_owner_pos_fid)
                            .typeify<LegionRuntime::Arrays::Point<2>>();

  Legion::Domain ghost_domain = runtime->get_index_space_domain(
    ctx, regions[0].get_logical_region().get_index_space());

  LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();
  LegionRuntime::Arrays::Rect<2> ghost_sub_rect;
  LegionRuntime::Accessor::ByteOffset byte_offset[2];

  LegionRuntime::Arrays::Point<2> * position_ref_data =
    reinterpret_cast<LegionRuntime::Arrays::Point<2> *>(
      position_ref_acc.template raw_rect_ptr<2>(
        ghost_rect, ghost_sub_rect, byte_offset));
  size_t position_max = ghost_rect.hi[1] - ghost_rect.lo[1] + 1;

  subrect_map lid_to_subrect_map;

  for(size_t ghost_pt = 0; ghost_pt < position_max; ghost_pt++) {
    LegionRuntime::Arrays::Point<2> ghost_ref = position_ref_data[ghost_pt];

    size_t lid = owner_map[ghost_ref.x[0]];

    auto itr = lid_to_subrect_map.find(lid);
    if(itr == lid_to_subrect_map.end()) {
      LegionRuntime::Arrays::Rect<2> new_rect(ghost_ref, ghost_ref);
      lid_to_subrect_map[lid] = new_rect;
    }
    else {
      if(ghost_ref.x[1] < lid_to_subrect_map[lid].lo[1]) {
        LegionRuntime::Arrays::Rect<2> new_rect(
          ghost_ref, lid_to_subrect_map[lid].hi);
        lid_to_subrect_map[lid] = new_rect;
      }
      else if(ghost_ref.x[1] > lid_to_subrect_map[lid].hi[1]) {
        LegionRuntime::Arrays::Rect<2> new_rect(
          lid_to_subrect_map[lid].lo, ghost_ref);
        lid_to_subrect_map[lid] = new_rect;
      }
    } // if itr == end

  } // for ghost_pt

  return lid_to_subrect_map;
} // owners_subregions

#undef flecsi_internal_legion_task

/*!
  FIXME DEOCUMENTATION
 */
struct MaxReductionOp {
  static const Legion::ReductionOpID redop_id = max_redop_id;

  typedef double LHS;
  typedef double RHS;
  static const double identity;

  template<bool EXCLUSIVE>
  static void apply(LHS & lhs, RHS rhs);

  template<bool EXCLUSIVE>
  static void fold(RHS & rhs1, RHS rhs2);
};

/*!
  FIXME DEOCUMENTATION
 */
struct MinReductionOp {
  static const Legion::ReductionOpID redop_id = min_redop_id;

  typedef double LHS;
  typedef double RHS;
  static const double identity;

  template<bool EXCLUSIVE>
  static void apply(LHS & lhs, RHS rhs);

  template<bool EXCLUSIVE>
  static void fold(RHS & rhs1, RHS rhs2);
};

} // namespace execution
} // namespace flecsi
