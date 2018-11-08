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

#define flecsi_internal_legion_task(task_name, return_type)                  \
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

  {
    clog_tag_guard(legion_tasks);
    clog(trace) << "Executing owner pos correction task " << std::endl;
  }

  clog_assert((regions.size() % 2) == 0,
      "owner_pos_correction_task needs a multiple of 2 regions");
  clog_assert(
      (task->regions.size() % 2) == 0,
      "owner_pos_correction_task needs a multiple of 2 regions");
  for (int region_idx = 0; region_idx < regions.size(); region_idx++)
    clog_assert(
        task->regions[region_idx].privilege_fields.size() == 1,
        "owner_pos_correction_task called with wrong number of fields");

  context_t & context_ = context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map =
      context_.coloring_map();

  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;

  auto ghost_owner_pos_fid = Legion::FieldID(internal_field::ghost_owner_pos);

  size_t region_idx = 0;
  for (auto idx_space : coloring_map) {
    LegionRuntime::Accessor::RegionAccessor<
        generic_type, LegionRuntime::Arrays::Point<2>> ghost_ref_acc =
            regions[region_idx].get_field_accessor(ghost_owner_pos_fid)
            .typeify<LegionRuntime::Arrays::Point<2>>();
    Legion::Domain ghost_domain = runtime->get_index_space_domain(
        ctx, regions[region_idx].get_logical_region().get_index_space());
    LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();

    region_idx++;

    LegionRuntime::Accessor::RegionAccessor<
        generic_type, LegionRuntime::Arrays::Point<2>> owner_ref_acc =
            regions[region_idx].get_field_accessor(ghost_owner_pos_fid)
            .typeify<LegionRuntime::Arrays::Point<2>>();
    Legion::Domain owner_domain = runtime->get_index_space_domain(
        ctx, regions[region_idx].get_logical_region().get_index_space());

    region_idx++;

    for(LegionRuntime::Arrays::GenericPointInRectIterator<2> itr(ghost_rect);
        itr; itr++) {
      auto ghost_ptr = Legion::DomainPoint::from_point<2>(itr.p);
      LegionRuntime::Arrays::Point<2> old_location =
        ghost_ref_acc.read(ghost_ptr);

      {
        clog_tag_guard(legion_tasks);
        clog(trace) << "points to " << old_location.x[0] << ","
                    << old_location.x[1] << " local mirror is "
                    << ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1]
                    << std::endl;
      } // scope

      // NOTE: We stored a forward pointer in old shared location to new
      // location
      LegionRuntime::Arrays::Point<2> new_location =
          owner_ref_acc.read(
              Legion::DomainPoint::from_point<2>(old_location));
      ghost_ref_acc.write(ghost_ptr, new_location);

      {
        clog_tag_guard(legion_tasks);
        clog(trace) << ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1]
                    << " points to " << new_location.x[0] << ","
                    << new_location.x[1] << std::endl;
      } // scope

    } // for itr

  }  // idx_space

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

  {
    clog_tag_guard(legion_tasks);
    clog(trace) << "executing compaction task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map =
    context_.coloring_map();

  auto ghost_owner_pos_fid = Legion::FieldID(internal_field::ghost_owner_pos);

  {
    clog_tag_guard(legion_tasks);
    // In old position of shared, write compacted location
    // In compacted position of ghost, write the reference/pointer
    // to pre-compacted shared
    // ghost reference/pointer will need to communicate with other ranks in
    // correction_task to obtain corrected pointer
    size_t region_idx = 0;
    for (auto idx_space : coloring_map) {

      Legion::IndexSpace ispace =
          regions[region_idx].get_logical_region().get_index_space();
      LegionRuntime::Accessor::RegionAccessor<
          LegionRuntime::Accessor::AccessorType::Generic,
          LegionRuntime::Arrays::Point<2>>
          acc_ref = regions[region_idx]
                        .get_field_accessor(ghost_owner_pos_fid)
                        .typeify<LegionRuntime::Arrays::Point<2>>();

      Legion::Domain domain = runtime->get_index_space_domain(ctx, ispace);
      LegionRuntime::Arrays::Rect<2> rect = domain.get_rect<2>();
      LegionRuntime::Arrays::GenericPointInRectIterator<2> expanded_itr(rect);

      for (auto exclusive_itr = idx_space.second.exclusive.begin();
           exclusive_itr != idx_space.second.exclusive.end(); ++exclusive_itr) {
        clog(trace) << my_color << " key " << idx_space.first << " exclusive "
                    << " " << *exclusive_itr << std::endl;
        expanded_itr++;
      } // exclusive_itr

      for (auto shared_itr = idx_space.second.shared.begin();
           shared_itr != idx_space.second.shared.end(); ++shared_itr) {
        const flecsi::coloring::entity_info_t shared = *shared_itr;
        const LegionRuntime::Arrays::Point<2> reference =
            LegionRuntime::Arrays::make_point(shared.rank, shared.offset);
        // reference is the old location, expanded_itr.p is the new location
        acc_ref.write(
            Legion::DomainPoint::from_point<2>(reference), expanded_itr.p);

        clog(trace) << my_color << " key " << idx_space.first << " shared was "
                    << " " << *shared_itr << " now at " << expanded_itr.p
                    << std::endl;

        expanded_itr++;
      } // shared_itr

      for (auto ghost_itr = idx_space.second.ghost.begin();
           ghost_itr != idx_space.second.ghost.end(); ++ghost_itr) {
        const flecsi::coloring::entity_info_t ghost = *ghost_itr;
        const LegionRuntime::Arrays::Point<2> reference =
            LegionRuntime::Arrays::make_point(ghost.rank, ghost.offset);
        // reference is where we used to point, expanded_itr.p is where ghost
        // is now
        acc_ref.write(
            Legion::DomainPoint::from_point<2>(expanded_itr.p), reference);
        clog(trace) << "color " << my_color << " key " << idx_space.first
                    << " ghost "
                    << " " << *ghost_itr <<
            //" now at " << expanded_itr.p <<
            std::endl;
        expanded_itr++;
      } // ghost_itr
      region_idx++;
    } // for idx_space
  } // clog_tag_guard

} // owner_pos_compaction_task

/*!
 Ghost copy task writes data from shared into ghost

 @ingroup legion-execution
 */

flecsi_internal_legion_task(ghost_copy_task, void) {
  using offset_t = data::sparse_data_offset_t;

  const int my_color = runtime->find_local_MPI_rank();

  context_t & context = context_t::instance();

  struct args_t {
    size_t data_client_hash;
    size_t index_space;
    bool sparse;
    size_t reserve;
    size_t max_entries_per_index;
  };
  args_t args = *(args_t *)task->args;

  if (!args.sparse){
    clog_assert(regions.size() == 2, "ghost_copy_task requires 2 regions");
    clog_assert(task->regions.size() == 2, "ghost_copy_task requires 2 regions");
  } else {
    clog_assert(regions.size() == 4, "ghost_copy_task requires 2 regions");
    clog_assert(task->regions.size() == 4, "ghost_copy_task requires 2 regions");
  }

  clog_assert(
      (task->regions[1].privilege_fields.size() -
       task->regions[0].privilege_fields.size()) == 1,
      "ghost region additionally requires ghost_owner_pos_fid");

  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto position_ref_acc = regions[1]
                            .get_field_accessor(ghost_owner_pos_fid)
                            .typeify<LegionRuntime::Arrays::Point<2>>();

  Legion::Domain owner_domain = runtime->get_index_space_domain(
    ctx, regions[0].get_logical_region().get_index_space());
  Legion::Domain ghost_domain = runtime->get_index_space_domain(
    ctx, regions[1].get_logical_region().get_index_space());

  if (!args.sparse){
  // For each field, copy data from shared to ghost
		for (auto fid : task->regions[0].privilege_fields) {
			// Look up field info in context
			auto iitr = context.field_info_map().find(
					{args.data_client_hash, args.index_space});
			clog_assert(iitr != context.field_info_map().end(), "invalid index space");
			auto fitr = iitr->second.find(fid);
			clog_assert(fitr != iitr->second.end(), "invalid fid");
			const context_t::field_info_t & field_info = fitr->second;

			const Legion::FieldAccessor<READ_ONLY, char, 2,
					Legion::coord_t, Realm::AffineAccessor< char, 2, Legion::coord_t> >
					owner_acc(regions[0], fid, field_info.size);
			const Legion::FieldAccessor<READ_WRITE, char, 2,
					Legion::coord_t, Realm::AffineAccessor<char, 2, Legion::coord_t> >
					ghost_acc(regions[1], fid, field_info.size);

			for (Legion::Domain::DomainPointIterator itr(ghost_domain); itr; itr++) {
				//auto ghost_ptr = Legion::DomainPoint::from_point<2>(itr.p);
				auto &ghost_ptr = itr.p;
        LegionRuntime::Arrays::Point<2> owner_location =
					 position_ref_acc.read(ghost_ptr);
				auto owner_ptr = Legion::DomainPoint::from_point<2>(owner_location);

			  char *ptr_ghost_acc = (char*)(ghost_acc.ptr(ghost_ptr));
			  char *ptr_owner_acc = (char*)(owner_acc.ptr(owner_ptr));
			  memcpy(ptr_ghost_acc, ptr_owner_acc, field_info.size);
		  } // for ghost_domain
		} // for fid
  }else {//sparse


    Legion::Domain ghost_entries_domain = runtime->get_index_space_domain(
       ctx, regions[3].get_logical_region().get_index_space());

    auto entries_position_ref_acc = regions[3]
                              .get_field_accessor(ghost_owner_pos_fid)
                              .typeify<LegionRuntime::Arrays::Point<2>>();

    const int my_color = runtime->find_local_MPI_rank();

    for (auto fid : task->regions[0].privilege_fields) {
      // Look up field info in context
      auto iitr = context.field_info_map().find(
          {args.data_client_hash, args.index_space});
      clog_assert(iitr != context.field_info_map().end(), "invalid index space");
      auto fitr = iitr->second.find(fid);
      clog_assert(fitr != iitr->second.end(), "invalid fid");
      const context_t::field_info_t & field_info = fitr->second;

      const Legion::FieldAccessor<READ_ONLY, char, 2,
          Legion::coord_t, Realm::AffineAccessor< char, 2, Legion::coord_t> >
          owner_offset_acc(regions[0], fid, sizeof(offset_t));
      const Legion::FieldAccessor<READ_WRITE, char, 2,
          Legion::coord_t, Realm::AffineAccessor<char, 2, Legion::coord_t> >
          ghost_offset_acc(regions[1], fid, sizeof(offset_t));
      const Legion::FieldAccessor<READ_ONLY, char, 2,
          Legion::coord_t, Realm::AffineAccessor< char, 2, Legion::coord_t> >
          owner_acc(regions[2], fid, field_info.size +sizeof(size_t));
      const Legion::FieldAccessor<READ_WRITE, char, 2,
          Legion::coord_t, Realm::AffineAccessor<char, 2, Legion::coord_t> >
          ghost_acc(regions[3], fid, field_info.size +sizeof(size_t));

      for (Legion::Domain::DomainPointIterator itr(ghost_entries_domain);
				itr; itr++) {
        auto &ghost_ptr = itr.p;
        LegionRuntime::Arrays::Point<2> owner_location =
           entries_position_ref_acc.read(ghost_ptr);
        auto owner_ptr = Legion::DomainPoint::from_point<2>(owner_location);

        char *ptr_ghost_acc = (char*)(ghost_acc.ptr(ghost_ptr));
        char *ptr_owner_acc = (char*)(owner_acc.ptr(owner_ptr));
        size_t size = field_info.size + sizeof(size_t);
        size_t chunk = size;// * args.max_entries_per_index;
        memcpy(ptr_ghost_acc, ptr_owner_acc, chunk);
      }//for ghost_entries_domain
      
      for (Legion::Domain::DomainPointIterator itr(ghost_domain);
        itr; itr++){
        auto &ghost_ptr = itr.p;
        LegionRuntime::Arrays::Point<2> owner_location =
           position_ref_acc.read(ghost_ptr);
        offset_t * ghost_start_and_count_location =
          reinterpret_cast <offset_t*>(ghost_offset_acc.ptr(itr.p));
        const offset_t * owner_start_and_count_location =
          reinterpret_cast <const offset_t*>(
            owner_offset_acc.ptr(
              Legion::DomainPoint::from_point<2>(owner_location)));
        ghost_start_and_count_location->set_count(
          owner_start_and_count_location->count());

      } // for ghost_domain
    } // for fids
  
  }//if
} // ghost_copy_task


flecsi_internal_legion_task(sparse_set_owner_position_task, void){
  using offset_t = data::sparse_data_offset_t;

  const int my_color = runtime->find_local_MPI_rank();

  context_t & context = context_t::instance();

  clog_assert(regions.size() == 3, "sparse_set_owner_position task requires 3 regions");
 
  auto ghost_owner_pos_fid = 
      LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto position_ref_acc = regions[0]
                              .get_field_accessor(ghost_owner_pos_fid)
                              .typeify<LegionRuntime::Arrays::Point<2>>();

  Legion::Domain ghost_domain = runtime->get_index_space_domain(
      ctx, regions[0].get_logical_region().get_index_space());

  Legion::Domain ghost_entries_domain = runtime->get_index_space_domain(
      ctx, regions[2].get_logical_region().get_index_space());

  LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();
  LegionRuntime::Arrays::Rect<2> ghost_entries_rect = 
		ghost_entries_domain.get_rect<2>();

  LegionRuntime::Arrays::GenericPointInRectIterator<2> entries_itr(
		ghost_entries_rect);

  size_t num_owner_entries =
		ghost_entries_rect.hi[1]-ghost_entries_rect.lo[1]+1;
  size_t num_ghosts = ghost_rect.hi[1]-ghost_rect.lo[1]+1;
  if (num_ghosts!=0){
     size_t max_entries_per_index=num_owner_entries/num_ghosts;

  Legion::FieldID fid = *(task->regions[1].privilege_fields.begin());

  const Legion::FieldAccessor<READ_ONLY, offset_t, 2,
      Legion::coord_t, Realm::AffineAccessor<offset_t, 2, Legion::coord_t> >
      owner_offset_acc(regions[1], fid, sizeof(offset_t));

  Legion::FieldAccessor<READ_WRITE, LegionRuntime::Arrays::Point<2>, 2,
      Legion::coord_t, Realm::AffineAccessor<
		  LegionRuntime::Arrays::Point<2>, 2, Legion::coord_t> >
      ghost_entries_acc(regions[2], ghost_owner_pos_fid, 
      sizeof(LegionRuntime::Arrays::Point<2>));  

  std::vector <LegionRuntime::Arrays::Point<2>> owner_points;

  for (Legion::Domain::DomainPointIterator itr(ghost_domain); itr; itr++) {
    LegionRuntime::Arrays::Point<2> owner_location =
           position_ref_acc.read(itr.p);
    const offset_t * owner_start_and_count_location =
        reinterpret_cast <const offset_t*>(owner_offset_acc.ptr(
              Legion::DomainPoint::from_point<2>(owner_location)));
  
      for (size_t count =0; count <max_entries_per_index;
       count ++)
      {
        LegionRuntime::Arrays::Point<2> owner_point=
        LegionRuntime::Arrays::make_point(owner_location[0],
          owner_start_and_count_location->start()+count);
        owner_points.push_back(owner_point);
      }//for
  }//for

  size_t i=0;
  for (Legion::Domain::DomainPointIterator itr(ghost_entries_domain); itr;
			itr++) {
    auto &ptr = itr.p;
    ghost_entries_acc.write(ptr, owner_points[i]);     
    i++;
  }
}
}

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
