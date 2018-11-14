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

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>

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

using legion_map = Legion::STL::
    map<LegionRuntime::Arrays::coord_t, LegionRuntime::Arrays::coord_t>;
using subrect_map = Legion::STL::map<size_t, LegionRuntime::Arrays::Rect<2>>;

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {
  
enum FieldIDs {
  FID_CELL_ID = 328,
  FID_CELL_PARTITION_COLOR,
  FID_CELL_CELL_NRANGE,
  FID_CELL_VERTEX_NRANGE,
  FID_CELL_EDGE_NRANGE,
  FID_CELL_TO_CELL_ID,
  FID_CELL_TO_CELL_PTR,
  FID_CELL_TO_VERTEX_ID,
  FID_CELL_TO_VERTEX_PTR,
  FID_CELL_TO_EDGE_ID,
  FID_CELL_TO_EDGE_PTR,
  FID_VERTEX_ID,
  FID_VERTEX_PARTITION_COLOR,
	FID_VERTEX_PARTITION_COLOR_ID,
  FID_EDGE_ID,
  FID_EDGE_PARTITION_COLOR,
  FID_CELL_OFFSET,
  FID_VERTEX_OFFSET,
  FID_EDGE_OFFSET,
};

typedef struct init_mesh_task_rt_s {
	int cell_to_cell_count;
	int cell_to_vertex_count;
  int cell_to_edge_count;
}init_mesh_task_rt_t;

typedef struct task_entity_s {
  int entity_id;
  int entity_map_id;
  Legion::FieldID id_fid;
  Legion::FieldID color_fid;
  Legion::FieldID offset_fid;
  int print_flag;
}task_entity_t;

typedef struct task_mesh_definition_s {
  intptr_t md_ptr;
  int entity_id;
  int entity_array[4];
  int total_num_entities;
}task_mesh_definition_t;

#define TASK_MD_OFFSET  12
#define DP_VERBOSE_PRINT 1

void dp_debug_printf(int verbose_level, const char *format, ...);

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
 @def __flecsi_internal_legion_task

 This macro simplifies pure Legion task definitions by filling in the
 boiler-plate function arguments.

 @param task_name   The plain-text task name.
 @param return_type The return type of the task.

 @ingroup legion-execution
*/

#define __flecsi_internal_legion_task(task_name, return_type)                  \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Legion task template */                                                   \
  inline return_type task_name(                                                \
      const Legion::Task * task,                                               \
      const std::vector<Legion::PhysicalRegion> & regions,                     \
      Legion::Context ctx, Legion::Runtime * runtime)

/*!
 Onwer pos correction task corrects the owner position reference/pointer in
 the ghost partition by reading from old location in primary position.

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(owner_pos_correction_task, void) {

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

    for (LegionRuntime::Arrays::GenericPointInRectIterator<2> itr(ghost_rect);
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

__flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

/*!
 Interprocess communication to wait for control to pass back to the Legion
 runtime.

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

/*!
 Interprocess communication to unset mpi execute state.

 @ingroup legion-execution
*/

__flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

/*!
 Compaction task writes new location in old location.

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(owner_pos_compaction_task, void) {
  const int my_color = task->index_point.point_data[0];

  {
    clog_tag_guard(legion_tasks);
    clog(trace) << "Executing compaction task " << my_color << std::endl;
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

__flecsi_internal_legion_task(ghost_copy_task, void) {

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


__flecsi_internal_legion_task(sparse_set_owner_position_task, void){
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
//        std::cout <<"IRINA DEBUG, start = "<< owner_start_and_count_location->start()<<" , count = "<<owner_start_and_count_location->count()<<std::endl;
       // auto &ptr = entries_itr.p;
       // ghost_entries_acc.write(ptr, owner_point);
      }//for
  }//for

  size_t i=0;
  for (Legion::Domain::DomainPointIterator itr(ghost_entries_domain); itr;
			itr++) {
    auto &ptr = itr.p;
    ghost_entries_acc.write(ptr, owner_points[i]);     
//if (runtime->find_local_MPI_rank()==0){
//std::cout <<"IRINA DEBUG owner point ["<<i<<"] = "<<
//owner_points[i]<<"    to "<<itr.p<<std::endl;
//}
    i++;
  }
}
}

/*!
 Owners subregions task returns subrects required from every neighbor

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(owners_subregions_task, subrect_map) {
  const int my_color = runtime->find_local_MPI_rank();
  // clog(error) << "rank " << my_color << " owners_subregions_task" <<
  // std::endl;

  clog_assert(regions.size() == 1, "owners_subregions_task requires 1 region");
  clog_assert(
      task->regions.size() == 1, "owners_subregions_task requires 1 region");
  clog_assert(
      task->regions[0].privilege_fields.size() == 1,
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

  for (size_t ghost_pt = 0; ghost_pt < position_max; ghost_pt++) {
    LegionRuntime::Arrays::Point<2> ghost_ref = position_ref_data[ghost_pt];

    size_t lid = owner_map[ghost_ref.x[0]];

    auto itr = lid_to_subrect_map.find(lid);
    if (itr == lid_to_subrect_map.end()) {
      LegionRuntime::Arrays::Rect<2> new_rect(ghost_ref, ghost_ref);
      lid_to_subrect_map[lid] = new_rect;
    } else {
      if (ghost_ref.x[1] < lid_to_subrect_map[lid].lo[1]) {
        LegionRuntime::Arrays::Rect<2> new_rect(
            ghost_ref, lid_to_subrect_map[lid].hi);
        lid_to_subrect_map[lid] = new_rect;
      } else if (ghost_ref.x[1] > lid_to_subrect_map[lid].hi[1]) {
        LegionRuntime::Arrays::Rect<2> new_rect(
            lid_to_subrect_map[lid].lo, ghost_ref);
        lid_to_subrect_map[lid] = new_rect;
      }
    } // if itr == end

  } // for ghost_pt

  return lid_to_subrect_map;
} // owners_subregions

#if 0
/*!
 init mesh task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_mesh_task, init_mesh_task_rt_t) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Point<1>,1> cell_color_acc(regions[0], FID_CELL_PARTITION_COLOR);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Rect<1>,1> cell_cell_nrange_acc(regions[0], FID_CELL_CELL_NRANGE);
	const Legion::FieldAccessor<READ_WRITE,int,1> vertex_id_acc(regions[1], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_WRITE,double,1> vertex_color_id_acc(regions[1], FID_VERTEX_PARTITION_COLOR_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Point<1>,1> vertex_color_acc(regions[1], FID_VERTEX_PARTITION_COLOR);
	
	int total_num_colors = task->index_domain.get_volume();
  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());

  Legion::Domain vertex_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
  // Starting point of cell and vertex
	const int cell_starting_point = cell_domain.lo().point_data[0];
	const int vertex_starting_point = vertex_domain.lo().point_data[0];

	flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
	int total_num_cells = sd.num_entities(1);
	int total_num_vertices = sd.num_entities(0);

  // TODO: how to recover DIMENSION from sd
	auto partetis_dcrs = flecsi::coloring::make_dcrs(sd, 2, 1);
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();
	auto color_results = colorer->parmetis_color(partetis_dcrs);
	
	// Init cell id and color
	int ct = 0;								 
  int cell_to_vertex_count = 0;
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
	  int cell_id = cell_starting_point + ct;
		cell_id_acc[*pir] = cell_id;
    cell_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(color_results[ct]);
		std::vector<size_t> vertices_of_cell = sd.entities(2, 0, cell_id);
		dp_debug_printf(2, "rank %d, cell_id %d, new color %d, V(", my_rank, (int)cell_id_acc[*pir], color_results[ct]);
		for(int i = 0; i < vertices_of_cell.size(); i++) {
			dp_debug_printf(2, "%d ", vertices_of_cell[i]);
		}
		dp_debug_printf(2, ")\n");
		cell_to_vertex_count += vertices_of_cell.size();
		ct ++;
	}
	
  // Init vertex id and color(set to max)
  dp_debug_printf(2, "rank %d, vertex: ", my_rank);
	ct = 0;								 
	for (Legion::PointInDomainIterator<1> pir(vertex_domain); pir(); pir++) {
	  int vertex_id = vertex_starting_point +  ct;
		vertex_id_acc[*pir] = vertex_id;
		vertex_color_id_acc[*pir] = total_num_colors;
		vertex_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(total_num_colors);
		dp_debug_printf(2, "%d, ", (int)vertex_id_acc[*pir]);
		ct ++;
	}
  dp_debug_printf(2, "\n");

	auto dcrs = flecsi::coloring::make_dcrs(sd, 2, 0);
	dp_debug_printf(1, "rank %d, init_mesh, num_cell %d, num_vertex %d, num_colors %d, cell_start %d, vertex_start %d, dcrs index size %d\n", 
		my_rank, cell_domain.get_volume(), vertex_domain.get_volume(), total_num_colors, cell_starting_point, vertex_starting_point, dcrs.indices.size());
	init_mesh_task_rt_t rt_value;
	rt_value.cell_to_cell_count = dcrs.indices.size();
	rt_value.cell_to_vertex_count = cell_to_vertex_count;
	return rt_value;
}

/*!
 init adjacency task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_adjacency_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Rect<1>,1> cell_cell_nrange_acc(regions[0], FID_CELL_CELL_NRANGE);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Rect<1>,1> cell_vertex_nrange_acc(regions[0], FID_CELL_VERTEX_NRANGE);
	const Legion::FieldAccessor<WRITE_DISCARD,int,1> cell_to_cell_id_acc(regions[1], FID_CELL_TO_CELL_ID);
	const Legion::FieldAccessor<WRITE_DISCARD,LegionRuntime::Arrays::Point<1>,1> cell_to_cell_ptr_acc(regions[1], FID_CELL_TO_CELL_PTR);
	const Legion::FieldAccessor<WRITE_DISCARD,int,1> cell_to_vertex_id_acc(regions[2], FID_CELL_TO_VERTEX_ID);
	const Legion::FieldAccessor<WRITE_DISCARD,LegionRuntime::Arrays::Point<1>,1> cell_to_vertex_ptr_acc(regions[2], FID_CELL_TO_VERTEX_PTR);
	
	int num_color;
	MPI_Comm_size(MPI_COMM_WORLD, &num_color);
	
	const int* cell_to_cell_count_per_subspace = (const int*)task->args;
	const int* cell_to_vertex_count_per_subspace = &(cell_to_cell_count_per_subspace[num_color+1]);

#if 0	
	printf("cell to cell ");
	for (int i = 0; i <= num_color; i++) {
		printf("%d ", cell_to_cell_count_per_subspace[i]);
	}
	printf("\n");
	
	printf("cell to vertex ");
	for (int i = 0; i <= num_color; i++) {
		printf("%d ", cell_to_vertex_count_per_subspace[i]);
	}
	printf("\n");
#endif
	
  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
  Legion::Domain cell_to_cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
  Legion::Domain cell_to_vertex_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
							 
	flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
	auto dcrs = flecsi::coloring::make_dcrs(sd, 2, 0);
	
	assert(cell_to_cell_domain.get_volume() == dcrs.indices.size());
	dp_debug_printf(1, "rank %d, init_adjacency_task, index size %d, num_cell %d, cell2cell %d, cell2vertex %ld\n", my_rank, dcrs.indices.size(), cell_domain.get_volume(), cell_to_cell_domain.get_volume(), cell_to_vertex_domain.get_volume());
	dp_debug_printf(2, "rank %d, dcrs index ", my_rank);
	for (int i = 0; i < dcrs.indices.size(); i++) {
		dp_debug_printf(2, "%d ", dcrs.indices[i]);
	}
	dp_debug_printf(2, "\n");
	
	dp_debug_printf(2, "rank %d, dcrs offset ", my_rank);
	for (int i = 0; i < dcrs.offsets.size(); i++) {
		dp_debug_printf(2, "%d ", dcrs.offsets[i]);
	}
	dp_debug_printf(2, "\n");
	
	int idx_cell2cell = 0;
	int idx_cell2vertex = 0;
	Legion::PointInDomainIterator<1> pir_cell2vertex(cell_to_vertex_domain);
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
		int cell_id = cell_id_acc[*pir];
		dp_debug_printf(2, "rank %d, cell %d, nrange[%d - %d]\n", my_rank, cell_id, dcrs.offsets[idx_cell2cell], dcrs.offsets[idx_cell2cell+1]-1);
		cell_cell_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell]),
		  LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell+1]-1));
		idx_cell2cell++;
		
		// find the vertex of a cell and fill into the cell2vertex 
		dp_debug_printf(2, "rank %d, cell %d, cell2vertex(", my_rank, cell_id);
		std::vector<size_t> vertices_of_cell = sd.entities(2, 0, cell_id);
		for (int i = 0; i < vertices_of_cell.size(); i++) {
			dp_debug_printf(2, "%d,", vertices_of_cell[i]);
			cell_to_vertex_id_acc[*pir_cell2vertex] = vertices_of_cell[i];
			cell_to_vertex_ptr_acc[*pir_cell2vertex] = LegionRuntime::Arrays::Point<1>(cell_to_vertex_id_acc[*pir_cell2vertex]); 
			pir_cell2vertex ++;
		}
		dp_debug_printf(2, ")[%d,%d]; \n", cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex, cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex + vertices_of_cell.size() -1);
		cell_vertex_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex),
		  LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex + vertices_of_cell.size() -1));
		idx_cell2vertex += vertices_of_cell.size();

	}
	dp_debug_printf(2, "\n");
	idx_cell2cell = 0;
  for (Legion::PointInDomainIterator<1> pir(cell_to_cell_domain); pir(); pir++) {
		cell_to_cell_id_acc[*pir] = dcrs.indices[idx_cell2cell];
		cell_to_cell_ptr_acc[*pir] = LegionRuntime::Arrays::Point<1>(cell_to_cell_id_acc[*pir]); 
		idx_cell2cell ++;
	}
#if 0	
	printf("rank %d, cell to vertex ", my_rank);
  for (Legion::PointInDomainIterator<1> pir(cell_to_vertex_domain); pir(); pir++) {
		printf("%d ", (int)cell_to_vertex_id_acc[*pir]);
	}
	printf("\n\n");
#endif
}

/*!
 verify vertex color task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(verify_vertex_color_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_alias_id_acc(regions[0], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> vertex_alias_color_id_acc(regions[0], FID_VERTEX_PARTITION_COLOR);
	
	int ct = 0;
  Legion::Domain vertex_alias_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
									 
	dp_debug_printf(2, "Alias vertex rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_alias_domain); pir(); pir++) {
		LegionRuntime::Arrays::Point<1> pt = vertex_alias_color_id_acc[*pir];
	  dp_debug_printf(2, "%d:%d ", (int)vertex_alias_id_acc[*pir], pt.x[0]);
		ct ++;
	}
	dp_debug_printf(2, " total %d\n", ct);
}

/*!
 init offset task
 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_entity_offset_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const Legion::FieldAccessor<READ_WRITE,int,1> cell_primary_offset_acc(regions[0], FID_CELL_OFFSET);
  const Legion::FieldAccessor<READ_WRITE,int,1> vertex_primary_offset_acc(regions[1], FID_VERTEX_OFFSET);
  
  // Primary cell
	int ct = 0;
  Legion::Domain cell_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	dp_debug_printf(2, "[%d, Cell] primary offset, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_primary_domain); pir(); pir++) {
    cell_primary_offset_acc[*pir] = ct;
		dp_debug_printf(2, "%d ", (int)cell_primary_offset_acc[*pir]);
	  ct ++;
	}
	dp_debug_printf(2, " CP total %d\n", ct);
  
  // Primary vertex
	ct = 0;
  Legion::Domain vertex_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
	dp_debug_printf(2, "[%d, Vertex] primary offset, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_primary_domain); pir(); pir++) {
    vertex_primary_offset_acc[*pir] = ct;
	  dp_debug_printf(2, "%d ", (int)vertex_primary_offset_acc[*pir]);
		ct ++;
	}
	dp_debug_printf(2, " VP total %d\n", ct);
}

/*!
 verify dp task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(verify_dp_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_primary_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> cell_color_acc(regions[0], FID_CELL_PARTITION_COLOR);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_ghost_id_acc(regions[1], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_shared_id_acc(regions[2], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_exclusive_id_acc(regions[3], FID_CELL_ID);
  
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_ghost_offset_acc(regions[1], FID_CELL_OFFSET);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_shared_offset_acc(regions[2], FID_CELL_OFFSET);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_exclusive_offset_acc(regions[3], FID_CELL_OFFSET);
  
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> cell_ghost_color_acc(regions[1], FID_CELL_PARTITION_COLOR);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_primary_id_acc(regions[4], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_ghost_id_acc(regions[5], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_shared_id_acc(regions[6], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_exclusive_id_acc(regions[7], FID_VERTEX_ID);
  
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_ghost_offset_acc(regions[5], FID_VERTEX_OFFSET);
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_shared_offset_acc(regions[6], FID_VERTEX_OFFSET);
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_exclusive_offset_acc(regions[7], FID_VERTEX_OFFSET);
  
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> vertex_ghost_color_acc(regions[5], FID_VERTEX_PARTITION_COLOR);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_of_ghost_cell_id_acc(regions[8], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> reachable_cell_id_acc(regions[9], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_primary_image_nrange_id_acc(regions[10], FID_CELL_TO_CELL_ID);

#if 0  
  const Legion::FieldAccessor<READ_ONLY,int,1> cell_all_shared_id_acc(regions[11], FID_CELL_ID);
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> cell_all_shared_color_acc(regions[11], FID_CELL_PARTITION_COLOR);
  const Legion::FieldAccessor<READ_ONLY,int,1> vertex_all_shared_id_acc(regions[12], FID_VERTEX_ID);
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> vertex_all_shared_color_acc(regions[12], FID_VERTEX_PARTITION_COLOR);
#endif
  
  printf("\n");
  
  flecsi::coloring::index_coloring_t cells;
  coloring::coloring_info_t cell_color_info;
  flecsi::coloring::index_coloring_t vertices;
  coloring::coloring_info_t vertex_color_info;
  
  // Primary cell
	int ct = 0;
  Legion::Domain cell_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	printf("[%d, Cell] primary, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_primary_domain); pir(); pir++) {
	  if (ct == 0) {
		  int color = cell_color_acc.read(*pir);
			printf("parmetis color %d, ", color);
		}
		printf("%d ", (int)cell_primary_id_acc[*pir]);
	  ct ++;
    
    int cell_id = (int)cell_primary_id_acc[*pir];
    cells.primary.insert(cell_id);
	}
	printf(" CP total %d\n", ct);
  
  std::set<size_t> empty_set;
	
  // Ghost cell
	ct = 0;
  Legion::Domain cell_ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
	printf("[%d, Cell] ghost, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_ghost_domain); pir(); pir++) {
#if 0
    Legion::Domain cell_all_shared_domain_inner = runtime->get_index_space_domain(ctx,
                     task->regions[11].region.get_index_space());							 
    int shared_color = 0;
    for (Legion::PointInDomainIterator<1> pir_all_shared(cell_all_shared_domain_inner); pir_all_shared(); pir_all_shared++) {
  	  if (cell_ghost_id_acc[*pir] == cell_all_shared_id_acc[*pir_all_shared]) {
  	    shared_color = cell_all_shared_color_acc.read(*pir_all_shared);
        break;  
  	  }
  	}
#endif
    int color = cell_ghost_color_acc.read(*pir);
//	  printf("%d:%d:%d ", (int)cell_ghost_id_acc[*pir], shared_color, color);
    printf("%d ", (int)cell_ghost_id_acc[*pir]);
		ct ++;
    cell_color_info.ghost_owners.insert(color);
    
    int cell_id = (int)cell_ghost_id_acc[*pir];
    int offset = (int)cell_ghost_offset_acc[*pir];
    cells.ghost.insert(
            flecsi::coloring::entity_info_t(cell_id,
            color, offset, empty_set));
	}
  cell_color_info.ghost = ct;
  printf(" CG total %d\n", ct);
	
  // Shared cell
	ct = 0;
  Legion::Domain cell_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
	
	printf("[%d, Cell] shared, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_shared_domain); pir(); pir++) {
	  printf("%d ", (int)cell_shared_id_acc[*pir]);
		ct ++;
    
    int cell_id = (int)cell_shared_id_acc[*pir];
    int offset = (int)cell_shared_offset_acc[*pir];
    cells.shared.insert(
            flecsi::coloring::entity_info_t(cell_id,
            my_rank, offset, empty_set));
	}
  cell_color_info.shared = ct;
	printf(" CS total %d\n", ct);
	
  // Exclusive cell
	ct = 0;
  Legion::Domain cell_exclusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[3].region.get_index_space());
	
	printf("[%d, Cell] execlusive, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_exclusive_domain); pir(); pir++) {
	  printf("%d ", (int)cell_exclusive_id_acc[*pir]);
		ct ++;
    int cell_id = (int)cell_exclusive_id_acc[*pir];
    int offset = (int)cell_exclusive_offset_acc[*pir];
    cells.exclusive.insert(
            flecsi::coloring::entity_info_t(cell_id,
            my_rank, offset, empty_set));
	}
  cell_color_info.exclusive = ct;
	printf(" CE total %d\n", ct);
	
  // Primary vertex
	ct = 0;
  Legion::Domain vertex_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[4].region.get_index_space());
	
	printf("[%d, Vertex] primary, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_primary_domain); pir(); pir++) {
	  printf("%d ", (int)vertex_primary_id_acc[*pir]);
		ct ++;
    
    int vertex_id = (int)vertex_primary_id_acc[*pir];
    vertices.primary.insert(vertex_id);
	}
	printf(" VP total %d\n", ct);
	
  // Ghost vertex
	ct = 0;
  Legion::Domain vertex_ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[5].region.get_index_space());
	
	printf("[%d, Vertex] ghost, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_ghost_domain); pir(); pir++) {
#if 0
    Legion::Domain vertex_all_shared_domain_inner = runtime->get_index_space_domain(ctx,
                     task->regions[12].region.get_index_space());							 
    int shared_color = 0;
    for (Legion::PointInDomainIterator<1> pir_all_shared(vertex_all_shared_domain_inner); pir_all_shared(); pir_all_shared++) {
  	  if (vertex_ghost_id_acc[*pir] == vertex_all_shared_id_acc[*pir_all_shared]) {
  	    shared_color = vertex_all_shared_color_acc.read(*pir_all_shared);
        break;  
  	  }
  	}
#endif
    int color = vertex_ghost_color_acc.read(*pir);
	  //printf("%d:%d:%d ", (int)vertex_ghost_id_acc[*pir], shared_color, color);
    printf("%d ", (int)vertex_ghost_id_acc[*pir]);
		ct ++;
    vertex_color_info.ghost_owners.insert(color);
    
    int vertex_id = (int)vertex_ghost_id_acc[*pir];
    int offset = (int)vertex_ghost_offset_acc[*pir];
    vertices.ghost.insert(
            flecsi::coloring::entity_info_t(vertex_id,
            color, offset, empty_set));
	}
  vertex_color_info.ghost = ct;
	printf(" VG total %d\n", ct);
	
  // Shared vertex
	ct = 0;
  Legion::Domain vertex_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[6].region.get_index_space());
	
	printf("[%d, Vertex] shared, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_shared_domain); pir(); pir++) {
	  printf("%d ", (int)vertex_shared_id_acc[*pir]);
		ct ++;
    
    int vertex_id = (int)vertex_shared_id_acc[*pir];
    int offset = (int)vertex_shared_offset_acc[*pir];
    vertices.shared.insert(
            flecsi::coloring::entity_info_t(vertex_id,
            my_rank, offset, empty_set));
	}
  vertex_color_info.shared = ct;
	printf(" VS total %d\n", ct);
	
  // Exclusive vertex
	ct = 0;
  Legion::Domain vertex_exclusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[7].region.get_index_space());
	
	printf("[%d, Vertex] exclusive, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_exclusive_domain); pir(); pir++) {
	  printf("%d ", (int)vertex_exclusive_id_acc[*pir]);
		ct ++;
    
    int vertex_id = (int)vertex_exclusive_id_acc[*pir];
    int offset = (int)vertex_exclusive_offset_acc[*pir];
    vertices.exclusive.insert(
            flecsi::coloring::entity_info_t(vertex_id,
            my_rank, offset, empty_set));
	}
  vertex_color_info.exclusive = ct;
	printf(" VE total %d\n", ct);

#if 0  
  // All shared cell
	ct = 0;
  Legion::Domain cell_all_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[11].region.get_index_space());
	
	printf("[%d, Cell] all shared, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_all_shared_domain); pir(); pir++) {
	  printf("%d ", (int)cell_all_shared_id_acc[*pir]);
		ct ++;
	}
	printf(" CAS total %d\n", ct);
  
  // All shared vertex
	ct = 0;
  Legion::Domain vertex_all_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[12].region.get_index_space());
	
	printf("[%d, Vertex] all shared, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_all_shared_domain); pir(); pir++) {
	  printf("%d ", (int)vertex_all_shared_id_acc[*pir]);
		ct ++;
	}
	printf(" VAS total %d\n", ct);
#endif
  
  std::set<size_t> cell_test = {0,1,3};
  cell_color_info.shared_users = cell_test;
  vertex_color_info.shared_users = cell_test;
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();
  auto cell_coloring_info = communicator->gather_coloring_info(cell_color_info);
  auto vertex_coloring_info =
    communicator->gather_coloring_info(vertex_color_info);
  context_t & context_ = context_t::instance();
  context_.add_coloring(0, cells, cell_coloring_info);
  context_.add_coloring(1, vertices, vertex_coloring_info);
  //context_.add_coloring_coloring(0, cells);
  //context_.add_coloring_coloring(1, vertices);
#if 0
	ct = 0;
  Legion::Domain vertex_of_ghost_cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[8].region.get_index_space());
	
	printf("[Vertex] ghostcell rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_of_ghost_cell_domain); pir(); pir++) {
	  printf("%d ", (int)vertex_of_ghost_cell_id_acc[*pir]);
		ct ++;
	}
	printf(" total %d\n", ct);
	
	ct = 0;
  Legion::Domain cell_reachable_domain = runtime->get_index_space_domain(ctx,
                   task->regions[9].region.get_index_space());
	
	printf("[Cell] reachable rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_reachable_domain); pir(); pir++) {
	  printf("%d ", (int)reachable_cell_id_acc[*pir]);
		ct ++;
	}
	printf(" total %d\n", ct);
	
	ct = 0;
  Legion::Domain cell_primary_image_nrange_domain = runtime->get_index_space_domain(ctx,
                   task->regions[10].region.get_index_space());
	
	printf("[Cell] primary_image_nrange rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_primary_image_nrange_domain); pir(); pir++) {
	  printf("%d ", (int)cell_primary_image_nrange_id_acc[*pir]);
		ct ++;
	}
	printf(" total %d\n", ct);
#endif
	
	printf("\n");
}

#endif

/*!
 init cell task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_cell_task, init_mesh_task_rt_t) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  assert (task->regions.size() == 1);
  assert (task->regions[0].privilege_fields.size() == 2);
  
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Point<1>,1> cell_color_acc(regions[0], FID_CELL_PARTITION_COLOR);
	
	int total_num_colors = task->index_domain.get_volume();
  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
  // Starting point of cell and vertex
	const int cell_starting_point = cell_domain.lo().point_data[0];

  task_mesh_definition_t task_md = *(task_mesh_definition_t*)task->args;
  flecsi::topology::mesh_definition_base__ *sd = (flecsi::topology::mesh_definition_base__ *)task_md.md_ptr;
									 
	auto partetis_dcrs = flecsi::coloring::make_dcrs(*sd, sd->get_dimension(), sd->get_dimension()-1);
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();
	auto color_results = colorer->parmetis_color(partetis_dcrs);
	
	// Init cell id and color
	int ct = 0;								 
  int cell_to_vertex_count = 0;
  int cell_to_edge_count = 0;
  //TODO: face
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
	  int cell_id = cell_starting_point + ct;
		cell_id_acc[*pir] = cell_id;
    cell_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(color_results[ct]);
    for (int i = 1; i < task_md.total_num_entities; i++) {
		  if (task_md.entity_array[i] == 0) {
        std::vector<size_t> vertices_of_cell = sd->entities(sd->get_dimension(), 0, cell_id);
  		  dp_debug_printf(2, "rank %d, cell_id %d, new color %d, Vertex(", my_rank, (int)cell_id_acc[*pir], color_results[ct]);
  		  for(int i = 0; i < vertices_of_cell.size(); i++) {
  			  dp_debug_printf(2, "%d ", vertices_of_cell[i]);
  		  }
  		  dp_debug_printf(2, ")\n");
  		  cell_to_vertex_count += vertices_of_cell.size();
      }
		  if (task_md.entity_array[i] == 1) {
        std::vector<size_t> edges_of_cell = sd->entities(sd->get_dimension(), 1, cell_id);
  		  dp_debug_printf(2, "rank %d, cell_id %d, new color %d, Edge(", my_rank, (int)cell_id_acc[*pir], color_results[ct]);
  		  for(int i = 0; i < edges_of_cell.size(); i++) {
  			  dp_debug_printf(2, "%d ", edges_of_cell[i]);
  		  }
  		  dp_debug_printf(2, ")\n");
  		  cell_to_edge_count += edges_of_cell.size();
      }
    }
		ct ++;
	}

	auto dcrs = flecsi::coloring::make_dcrs(*sd, sd->get_dimension(), 0);
	dp_debug_printf(1, "<%d, init_cell>, entity_id %d, num_cell %d, num_colors %d, cell_start %d, cell_to_cell_count %d, cell_to_vertex_count %d, cell_to_edge_count %d\n", 
		my_rank, sd->get_dimension(), cell_domain.get_volume(), total_num_colors, cell_starting_point, dcrs.indices.size(), cell_to_vertex_count, cell_to_edge_count);
	init_mesh_task_rt_t rt_value;
	rt_value.cell_to_cell_count = dcrs.indices.size();
	rt_value.cell_to_vertex_count = cell_to_vertex_count;
  rt_value.cell_to_edge_count = cell_to_edge_count;
	return rt_value;
}

/*!
 init non_cell task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_non_cell_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  assert (task->regions.size() == 1);
  assert (task->regions[0].privilege_fields.size() == 2);
  
  const task_entity_t *task_entity = (const task_entity_t*)task->args;
  int entity_id = task_entity->entity_id;
  int id_fid = task_entity->id_fid; 
  int color_fid = task_entity->color_fid;
  
	const Legion::FieldAccessor<READ_WRITE,int,1> entity_id_acc(regions[0], id_fid);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Point<1>,1> entity_color_acc(regions[0], color_fid);
	
	int total_num_colors = task->index_domain.get_volume();

  Legion::Domain entity_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
  // Starting point of vertex
	const int entity_starting_point = entity_domain.lo().point_data[0];

  // Init vertex id and color(set to max)
  dp_debug_printf(1, "<%d, init non_cell task>, entity_id %d, : ", my_rank, entity_id);
	int ct = 0;								 
	for (Legion::PointInDomainIterator<1> pir(entity_domain); pir(); pir++) {
	  int vertex_id = entity_starting_point +  ct;
		entity_id_acc[*pir] = vertex_id;
		entity_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(total_num_colors);
		dp_debug_printf(2, "%d, ", (int)entity_id_acc[*pir]);
		ct ++;
	}
  dp_debug_printf(1, "\n");
}

/*!
 init cell_to_cell task
 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_cell_to_cell_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);

  assert (task->regions.size() == 2);
  assert (task->regions[0].privilege_fields.size() == 2);
  assert (task->regions[1].privilege_fields.size() == 2);
    
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Rect<1>,1> cell_cell_nrange_acc(regions[0], FID_CELL_CELL_NRANGE);
	const Legion::FieldAccessor<WRITE_DISCARD,int,1> cell_to_cell_id_acc(regions[1], FID_CELL_TO_CELL_ID);
	const Legion::FieldAccessor<WRITE_DISCARD,LegionRuntime::Arrays::Point<1>,1> cell_to_cell_ptr_acc(regions[1], FID_CELL_TO_CELL_PTR);
	
	int num_color;
	MPI_Comm_size(MPI_COMM_WORLD, &num_color);
	
	const int* task_args_buff = (const int*)task->args;
  const int* cell_to_cell_count_per_subspace = task_args_buff + TASK_MD_OFFSET;
  
  task_mesh_definition_t task_md = *(task_mesh_definition_t*)task->args;
  flecsi::topology::mesh_definition_base__ *sd = (flecsi::topology::mesh_definition_base__ *)task_md.md_ptr;

  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
  Legion::Domain cell_to_cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
							 
	auto dcrs = flecsi::coloring::make_dcrs(*sd, sd->get_dimension(), 0);
	
	assert(cell_to_cell_domain.get_volume() == dcrs.indices.size());
	dp_debug_printf(1, "<%d, init_cell_to_cell_task>, index size %d, num_cell %ld, cell2cell %ld\n", my_rank, dcrs.indices.size(), cell_domain.get_volume(), cell_to_cell_domain.get_volume());
	dp_debug_printf(2, "rank %d, dcrs index ", my_rank);
	for (int i = 0; i < dcrs.indices.size(); i++) {
		dp_debug_printf(2, "%d ", dcrs.indices[i]);
	}
	dp_debug_printf(2, "\n");
	
	dp_debug_printf(2, "rank %d, dcrs offset ", my_rank);
	for (int i = 0; i < dcrs.offsets.size(); i++) {
		dp_debug_printf(2, "%d ", dcrs.offsets[i]);
	}
	dp_debug_printf(2, "\n");
	
	int idx_cell2cell = 0;
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
		int cell_id = cell_id_acc[*pir];
		dp_debug_printf(2, "rank %d, cell %d, nrange[%d - %d]\n", my_rank, cell_id, dcrs.offsets[idx_cell2cell], dcrs.offsets[idx_cell2cell+1]-1);
		cell_cell_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell]),
		  LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell+1]-1));
		idx_cell2cell++;
	}
  
	dp_debug_printf(2, "\n");
	idx_cell2cell = 0;
  for (Legion::PointInDomainIterator<1> pir(cell_to_cell_domain); pir(); pir++) {
		cell_to_cell_id_acc[*pir] = dcrs.indices[idx_cell2cell];
		cell_to_cell_ptr_acc[*pir] = LegionRuntime::Arrays::Point<1>(cell_to_cell_id_acc[*pir]); 
		idx_cell2cell ++;
	}
}

/*!
 init cell_to_others task
 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_cell_to_others_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  assert (task->regions.size() == 2);
  assert (task->regions[0].privilege_fields.size() == 2);
  assert (task->regions[1].privilege_fields.size() == 2);
  
  std::set<Legion::FieldID>::iterator it = task->regions[0].privilege_fields.begin();
  Legion::FieldID cell_id_fid = *(it);
  it++;
  Legion::FieldID cell_others_nrange_fid = *(it);
  it = task->regions[1].privilege_fields.begin();
  Legion::FieldID cell_to_others_id_fid = *(it);
  it++;
  Legion::FieldID cell_to_others_ptr_fid = *(it);
  
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], cell_id_fid);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Rect<1>,1> cell_others_nrange_acc(regions[0], cell_others_nrange_fid);
	const Legion::FieldAccessor<WRITE_DISCARD,int,1> cell_to_others_id_acc(regions[1], cell_to_others_id_fid);
	const Legion::FieldAccessor<WRITE_DISCARD,LegionRuntime::Arrays::Point<1>,1> cell_to_others_ptr_acc(regions[1], cell_to_others_ptr_fid);
	
	int num_color;
	MPI_Comm_size(MPI_COMM_WORLD, &num_color);
	
	const int* task_args_buff = (const int*)task->args;
  const int* cell_to_others_count_per_subspace = task_args_buff + TASK_MD_OFFSET;
  
  task_mesh_definition_t task_md = *(task_mesh_definition_t*)task->args;
  flecsi::topology::mesh_definition_base__ *sd = (flecsi::topology::mesh_definition_base__ *)task_md.md_ptr;
  int entity_id = task_md.entity_id;
  
  dp_debug_printf(1, "<%d, init_cell_to_others>, entity_id %d\n", my_rank, entity_id);
	
  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
  Legion::Domain cell_to_others_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
							 
	
	int idx_cell2others = 0;
	Legion::PointInDomainIterator<1> pir_cell2others(cell_to_others_domain);
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
		int cell_id = cell_id_acc[*pir];
		
		// find the vertex of a cell and fill into the cell2vertex 
		dp_debug_printf(2, "rank %d, cell %d, cell2others(", my_rank, cell_id);
		std::vector<size_t> others_of_cell = sd->entities(sd->get_dimension(), entity_id, cell_id);
		for (int i = 0; i < others_of_cell.size(); i++) {
			dp_debug_printf(2, "%d,", others_of_cell[i]);
			cell_to_others_id_acc[*pir_cell2others] = others_of_cell[i];
			cell_to_others_ptr_acc[*pir_cell2others] = LegionRuntime::Arrays::Point<1>(cell_to_others_id_acc[*pir_cell2others]); 
			pir_cell2others ++;
		}
		dp_debug_printf(2, ")[%d,%d]; \n", cell_to_others_count_per_subspace[my_rank] + idx_cell2others, cell_to_others_count_per_subspace[my_rank] + idx_cell2others + others_of_cell.size() -1);
		cell_others_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_others_count_per_subspace[my_rank] + idx_cell2others),
		  LegionRuntime::Arrays::Point<1>(cell_to_others_count_per_subspace[my_rank] + idx_cell2others + others_of_cell.size() -1));
		idx_cell2others += others_of_cell.size();

	}
	dp_debug_printf(2, "\n");
}

/*!
 set entity offset task
 @ingroup legion-execution
 */

__flecsi_internal_legion_task(set_entity_offset_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  assert (task->regions.size() == 1);
  assert (task->regions[0].privilege_fields.size() == 1);
  Legion::FieldID fid = *(task->regions[0].privilege_fields.begin());
  
  const Legion::FieldAccessor<READ_WRITE,int,1> primary_offset_acc(regions[0], fid);
  
  const int* entity_id = (const int*)task->args;
  
  // Primary cell
	int ct = 0;
  Legion::Domain primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	dp_debug_printf(1, "[%d, %d] primary offset, ", my_rank, *entity_id);								 
  for (Legion::PointInDomainIterator<1> pir(primary_domain); pir(); pir++) {
    primary_offset_acc[*pir] = ct;
		dp_debug_printf(2, "%d ", (int)primary_offset_acc[*pir]);
	  ct ++;
	}
	dp_debug_printf(1, " P total %d\n", ct);
  
}

/*!
 output partition task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(output_partition_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  const task_entity_t *task_entity = (const task_entity_t*)task->args;
  int entity_map_id = task_entity->entity_map_id;
  int entity_id = task_entity->entity_id;
  int id_fid = task_entity->id_fid; 
  int color_fid = task_entity->color_fid;
  int offset_fid = task_entity->offset_fid;
  
  assert(task->regions.size() == 4);
  assert(task->regions[0].privilege_fields.size() == 2);
	const Legion::FieldAccessor<READ_ONLY,int,1> primary_id_acc(regions[0], id_fid);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> primary_color_acc(regions[0], color_fid);
	
  assert(task->regions[1].privilege_fields.size() == 3);
	const Legion::FieldAccessor<READ_ONLY,int,1> ghost_id_acc(regions[1], id_fid);
  const Legion::FieldAccessor<READ_ONLY,int,1> ghost_offset_acc(regions[1], offset_fid);
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> ghost_color_acc(regions[1], color_fid);
  
  assert(task->regions[2].privilege_fields.size() == 2);  
  const Legion::FieldAccessor<READ_ONLY,int,1> shared_id_acc(regions[2], id_fid);
  const Legion::FieldAccessor<READ_ONLY,int,1> shared_offset_acc(regions[2], offset_fid);
  
  assert(task->regions[3].privilege_fields.size() == 2);
  const Legion::FieldAccessor<READ_ONLY,int,1> exclusive_id_acc(regions[3], id_fid);
	const Legion::FieldAccessor<READ_ONLY,int,1> exclusive_offset_acc(regions[3], offset_fid);
  

  dp_debug_printf(1, "\n");
  
  flecsi::coloring::index_coloring_t entity;
  coloring::coloring_info_t entity_color_info;
  
  // Primary cell
	int ct = 0;
  Legion::Domain primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	dp_debug_printf(1, "[%d, %d, %d] primary, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(primary_domain); pir(); pir++) {
	  if (ct == 0) {
		  int color = primary_color_acc.read(*pir);
			dp_debug_printf(2, "parmetis color %d, ", color);
		}
		dp_debug_printf(2, "%d ", (int)primary_id_acc[*pir]);
	  ct ++;
    
    int cell_id = (int)primary_id_acc[*pir];
    entity.primary.insert(cell_id);
	}
	dp_debug_printf(1, " CP total %d\n", ct);
  
  std::set<size_t> empty_set;
	
  // Ghost cell
	ct = 0;
  Legion::Domain ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
	dp_debug_printf(1, "[%d, %d, %d] ghost, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(ghost_domain); pir(); pir++) {
    int color = ghost_color_acc.read(*pir);
//	  printf("%d:%d:%d ", (int)cell_ghost_id_acc[*pir], shared_color, color);
    dp_debug_printf(2, "%d ", (int)ghost_id_acc[*pir]);
		ct ++;
    entity_color_info.ghost_owners.insert(color);
    
    int cell_id = (int)ghost_id_acc[*pir];
    int offset = (int)ghost_offset_acc[*pir];
    entity.ghost.insert(
            flecsi::coloring::entity_info_t(cell_id,
            color, offset, empty_set));
	}
  entity_color_info.ghost = ct;
  dp_debug_printf(1, " CG total %d\n", ct);
  
  // Shared cell
	ct = 0;
  Legion::Domain shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
	
	dp_debug_printf(1, "[%d, %d, %d] shared, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(shared_domain); pir(); pir++) {
	  dp_debug_printf(2, "%d ", (int)shared_id_acc[*pir]);
		ct ++;
    
    int cell_id = (int)shared_id_acc[*pir];
    int offset = (int)shared_offset_acc[*pir];
    entity.shared.insert(
            flecsi::coloring::entity_info_t(cell_id,
            my_rank, offset, empty_set));
	}
  entity_color_info.shared = ct;
	dp_debug_printf(1, " CS total %d\n", ct);

  // Exclusive cell
	ct = 0;
  Legion::Domain exclusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[3].region.get_index_space());
	
	dp_debug_printf(1, "[%d, %d, %d] execlusive, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(exclusive_domain); pir(); pir++) {
	  dp_debug_printf(2, "%d ", (int)exclusive_id_acc[*pir]);
		ct ++;
    int cell_id = (int)exclusive_id_acc[*pir];
    int offset = (int)exclusive_offset_acc[*pir];
    entity.exclusive.insert(
            flecsi::coloring::entity_info_t(cell_id,
            my_rank, offset, empty_set));
	}
  entity_color_info.exclusive = ct;
	dp_debug_printf(1, " CE total %d\n", ct);
	
	dp_debug_printf(1, "\n");
  
  std::set<size_t> cell_test = {0,1,3};
  entity_color_info.shared_users = cell_test;
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();
  auto entity_coloring_info = communicator->gather_coloring_info(entity_color_info);
  context_t & context_ = context_t::instance();
  context_.add_coloring(entity_map_id, entity, entity_coloring_info);
}

/*!
 print partition task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(print_partition_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  const int point = task->index_point.point_data[0];
  
  assert(point == my_rank);
  
  const task_entity_t *task_entity = (const task_entity_t*)task->args;
  int entity_map_id = task_entity->entity_map_id;
  int entity_id = task_entity->entity_id;
  int id_fid = task_entity->id_fid; 
  int color_fid = task_entity->color_fid;
  int offset_fid = task_entity->offset_fid;
  int print_flag = task_entity->print_flag;
  
  assert(task->regions.size() == 4);
  assert(task->regions[0].privilege_fields.size() == 2);
	const Legion::FieldAccessor<READ_ONLY,int,1> primary_id_acc(regions[0], id_fid);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> primary_color_acc(regions[0], color_fid);
	
  assert(task->regions[1].privilege_fields.size() == 3);
	const Legion::FieldAccessor<READ_ONLY,int,1> ghost_id_acc(regions[1], id_fid);
  const Legion::FieldAccessor<READ_ONLY,int,1> ghost_offset_acc(regions[1], offset_fid);
  const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> ghost_color_acc(regions[1], color_fid);
  
  assert(task->regions[2].privilege_fields.size() == 2);  
  const Legion::FieldAccessor<READ_ONLY,int,1> shared_id_acc(regions[2], id_fid);
  const Legion::FieldAccessor<READ_ONLY,int,1> shared_offset_acc(regions[2], offset_fid);
  
  assert(task->regions[3].privilege_fields.size() == 2);
  const Legion::FieldAccessor<READ_ONLY,int,1> exclusive_id_acc(regions[3], id_fid);
	const Legion::FieldAccessor<READ_ONLY,int,1> exclusive_offset_acc(regions[3], offset_fid);
  

  printf("\n");
  
  flecsi::coloring::index_coloring_t entity;
  coloring::coloring_info_t entity_color_info;
  
  // Primary cell
	int ct = 0;
  Legion::Domain primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	printf("[%d, %d, %d] primary, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(primary_domain); pir(); pir++) {
	  if (ct == 0) {
		  int color = primary_color_acc.read(*pir);
			printf("parmetis color %d, ", color);
		}
		if (print_flag) {
      printf("%d ", (int)primary_id_acc[*pir]);
    }
	  ct ++;
    
	}
	printf(" CP total %d\n", ct);
  
  std::set<size_t> empty_set;
	
  // Ghost cell
	ct = 0;
  Legion::Domain ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
	printf("[%d, %d, %d] ghost, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(ghost_domain); pir(); pir++) {
    int color = ghost_color_acc.read(*pir);
//	  printf("%d:%d:%d ", (int)cell_ghost_id_acc[*pir], shared_color, color);
    if (print_flag) {
      printf("%d ", (int)ghost_id_acc[*pir]);
    }
		ct ++;
    
	}
  printf(" CG total %d\n", ct);
  
  // Shared cell
	ct = 0;
  Legion::Domain shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
	
	printf("[%d, %d, %d] shared, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(shared_domain); pir(); pir++) {
	  if (print_flag) {
      printf("%d ", (int)shared_id_acc[*pir]);
    }
		ct ++;
    
	}
	printf(" CS total %d\n", ct);

  // Exclusive cell
	ct = 0;
  Legion::Domain exclusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[3].region.get_index_space());
	
	printf("[%d, %d, %d] execlusive, ", my_rank, entity_id, entity_map_id);								 
  for (Legion::PointInDomainIterator<1> pir(exclusive_domain); pir(); pir++) {
	  if (print_flag) {
      printf("%d ", (int)exclusive_id_acc[*pir]);
    }
		ct ++;
	}
	printf(" CE total %d\n", ct);
	
	printf("\n");

}

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

struct MinReductionPointOp {
  static const Legion::ReductionOpID redop_id = min_pt_repo_id;

  typedef LegionRuntime::Arrays::Point<1> LHS;
  typedef LegionRuntime::Arrays::Point<1> RHS;
  static const LegionRuntime::Arrays::Point<1> identity;

  template<bool EXCLUSIVE>
  static void apply(LHS & lhs, RHS rhs);

  template<bool EXCLUSIVE>
  static void fold(RHS & rhs1, RHS rhs2);
};

/*!
 init vertex color task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_vertex_color_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
  
  assert (task->regions[0].privilege_fields.size() == 1);
  Legion::FieldID fid = *(task->regions[0].privilege_fields.begin());
  
	const Legion::ReductionAccessor<MinReductionPointOp, false, 1, Legion::coord_t,
																	Realm::AffineAccessor<LegionRuntime::Arrays::Point<1>,1,Legion::coord_t>>
																  vertex_alias_color_acc(regions[0], fid, MinReductionPointOp::redop_id); 
																	
	int ct = 0;
  Legion::Domain vertex_alias_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
									 							 
  for (Legion::PointInDomainIterator<1> pir(vertex_alias_domain); pir(); pir++) {
		int color = my_rank;
		vertex_alias_color_acc.reduce(*pir, LegionRuntime::Arrays::Point<1>(color));
	//	vertex_alias_color_acc[*pir] <<= LegionRuntime::Arrays::Point<1>(color);
	  ct ++;
	}
}

#undef __flecsi_internal_legion_task

} // namespace execution
} // namespace flecsi
