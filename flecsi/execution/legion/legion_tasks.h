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

#include <flecsi/execution/legion/helper.h>
#include <flecsi/execution/legion/internal_task.h>

#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/coloring/parmetis_colorer.h>

#define PRIMARY_PART 0
#define GHOST_PART 1
#define EXCLUSIVE_PART 0
#define SHARED_PART 1
#define SUBRECT_PART 0
#define OWNER_COLOR_TAG 1

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
  FID_CELL_TO_CELL_ID,
  FID_CELL_TO_CELL_PTR,
  FID_CELL_TO_VERTEX_ID,
  FID_CELL_TO_VERTEX_PTR,
  FID_VERTEX_ID,
  FID_VERTEX_PARTITION_COLOR,
	FID_VERTEX_PARTITION_COLOR_ID,
};

typedef struct init_mesh_task_rt_s {
	int cell_to_cell_count;
	int cell_to_vertex_count;
}init_mesh_task_rt_t;

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

  clog_assert(
      regions.size() >= 1, "owner_pos_correction_task called with no regions");
  clog_assert(
      task->regions.size() >= 1,
      "owner_pos_correction_task called with no regions");

  for (int region_idx = 0; region_idx < regions.size(); region_idx++)
    clog_assert(
        task->regions[region_idx].privilege_fields.size() == 1,
        "owner_pos_correction_task called with wrong number of fields");

  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  legion_map owner_map = task->futures[0].get_result<legion_map>();

  auto ghost_owner_pos_fid = Legion::FieldID(internal_field::ghost_owner_pos);

  LegionRuntime::Accessor::RegionAccessor<
      generic_type, LegionRuntime::Arrays::Point<2>>
      ghost_ref_acc = regions[0]
                          .get_field_accessor(ghost_owner_pos_fid)
                          .typeify<LegionRuntime::Arrays::Point<2>>();
  Legion::Domain ghost_domain = runtime->get_index_space_domain(
      ctx, regions[0].get_logical_region().get_index_space());
  LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();

  if (owner_map.size() > 0) {

    std::vector<LegionRuntime::Accessor::RegionAccessor<
        generic_type, LegionRuntime::Arrays::Point<2>>>
        owners_refs_accs;
    std::vector<LegionRuntime::Arrays::Rect<2>> owners_rects;
    for (int owner_idx = 0; owner_idx < owner_map.size(); owner_idx++) {
      owners_refs_accs.push_back(
          regions[1 + owner_idx]
              .get_field_accessor(ghost_owner_pos_fid)
              .typeify<LegionRuntime::Arrays::Point<2>>());
      Legion::Domain owner_domain = runtime->get_index_space_domain(
          ctx, regions[1 + owner_idx].get_logical_region().get_index_space());
      owners_rects.push_back(owner_domain.get_rect<2>());
    }

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
                    << " nbr " << owner_map[old_location.x[0]] << " range "
                    << owners_rects[owner_map[old_location.x[0]]].lo[0] << ":"
                    << owners_rects[owner_map[old_location.x[0]]].lo[1] << ","
                    << owners_rects[owner_map[old_location.x[0]]].hi[1]
                    << std::endl;
      } // scope

      clog_assert(
          old_location.x[0] == owners_rects[owner_map[old_location.x[0]]].lo[0],
          "ghost dependency closure error in specialization_driver()");
      clog_assert(
          old_location.x[1] >= owners_rects[owner_map[old_location.x[0]]].lo[1],
          "ghost owner position error in specialization_driver()");
      clog_assert(
          old_location.x[1] <= owners_rects[owner_map[old_location.x[0]]].hi[1],
          "ghost owner position error in specialization_driver()");

      // NOTE: We stored a forward pointer in old shared location to new
      // location
      LegionRuntime::Arrays::Point<2> new_location =
          owners_refs_accs[owner_map[old_location.x[0]]].read(
              Legion::DomainPoint::from_point<2>(old_location));
      ghost_ref_acc.write(ghost_ptr, new_location);

      {
        clog_tag_guard(legion_tasks);
        clog(trace) << ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1]
                    << " points to " << new_location.x[0] << ","
                    << new_location.x[1] << std::endl;
      } // scope

    } // for itr
  } // if we have owners

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
    // spmd_task() to obtain corrected pointer
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
  const int my_color = runtime->find_local_MPI_rank();

  context_t & context = context_t::instance();

  struct args_t {
    size_t data_client_hash;
    size_t index_space;
    size_t owner;
  };
  args_t args = *(args_t *)task->args;

  clog_assert(regions.size() == 2, "ghost_copy_task requires 2 regions");
  clog_assert(task->regions.size() == 2, "ghost_copy_task requires 2 regions");
  clog_assert(
      (task->regions[1].privilege_fields.size() -
       task->regions[0].privilege_fields.size()) == 1,
      "ghost region additionally requires ghost_owner_pos_fid");

  legion_map owner_map = task->futures[0].get_result<legion_map>();

  for (auto itr = owner_map.begin(); itr != owner_map.end(); itr++) {
    clog_tag_guard(legion_tasks);
    clog(trace) << "my_color= " << my_color << " gid " << itr->first
                << " maps to lid " << itr->second << " current owner lid is "
                << args.owner << std::endl;
  }

  auto ghost_owner_pos_fid =
      LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto position_ref_acc = regions[1]
                              .get_field_accessor(ghost_owner_pos_fid)
                              .typeify<LegionRuntime::Arrays::Point<2>>();

  Legion::Domain owner_domain = runtime->get_index_space_domain(
      ctx, regions[0].get_logical_region().get_index_space());
  Legion::Domain ghost_domain = runtime->get_index_space_domain(
      ctx, regions[1].get_logical_region().get_index_space());

  LegionRuntime::Arrays::Rect<2> owner_rect = owner_domain.get_rect<2>();
  LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();
  LegionRuntime::Arrays::Rect<2> owner_sub_rect;
  LegionRuntime::Arrays::Rect<2> ghost_sub_rect;
  LegionRuntime::Accessor::ByteOffset byte_offset[2];

  LegionRuntime::Arrays::Point<2> * position_ref_data =
      reinterpret_cast<LegionRuntime::Arrays::Point<2> *>(
          position_ref_acc.template raw_rect_ptr<2>(
              ghost_rect, ghost_sub_rect, byte_offset));
  size_t position_max = ghost_rect.hi[1] - ghost_rect.lo[1] + 1;

  // For each field, copy data from shared to ghost
  for (auto fid : task->regions[0].privilege_fields) {
    // Look up field info in context
    auto iitr = context.field_info_map().find(
        {args.data_client_hash, args.index_space});
    clog_assert(iitr != context.field_info_map().end(), "invalid index space");
    auto fitr = iitr->second.find(fid);
    clog_assert(fitr != iitr->second.end(), "invalid fid");
    const context_t::field_info_t & field_info = fitr->second;

    auto acc_shared = regions[0].get_field_accessor(fid);
    auto acc_ghost = regions[1].get_field_accessor(fid);

    uint8_t * data_shared =
        reinterpret_cast<uint8_t *>(acc_shared.template raw_rect_ptr<2>(
            owner_rect, owner_sub_rect, byte_offset));

    {
      clog_tag_guard(legion_tasks);
      clog(trace) << "my_color = " << my_color << " owner lid = " << args.owner
                  << " owner rect = " << owner_rect.lo[0] << ","
                  << owner_rect.lo[1] << " to " << owner_rect.hi[0] << ","
                  << owner_rect.hi[1] << std::endl;
    }

    uint8_t * ghost_data =
        reinterpret_cast<uint8_t *>(acc_ghost.template raw_rect_ptr<2>(
            ghost_rect, ghost_sub_rect, byte_offset));

    for (size_t ghost_pt = 0; ghost_pt < position_max; ghost_pt++) {
      LegionRuntime::Arrays::Point<2> ghost_ref = position_ref_data[ghost_pt];

      {
        clog_tag_guard(legion_tasks);
        clog(trace) << my_color << " copy from position " << ghost_ref.x[0]
                    << "," << ghost_ref.x[1] << std::endl;
      }

      if (owner_map[ghost_ref.x[0]] == args.owner) {
        size_t owner_offset = ghost_ref.x[1] - owner_sub_rect.lo[1];
        uint8_t * owner_copy_ptr = data_shared + owner_offset * field_info.size;
        size_t ghost_offset = ghost_pt;
        uint8_t * ghost_copy_ptr = ghost_data + ghost_offset * field_info.size;
        std::memcpy(ghost_copy_ptr, owner_copy_ptr, field_info.size);
      } // if
    } // for ghost_pt
  } // for fid
} // ghost_copy_task

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
	
	const int cell_starting_point = cell_domain.lo().point_data[0];
	const int vertex_starting_point = vertex_domain.lo().point_data[0];
/*	int num_rows = 8;
	int num_rows_partition = 2;
	int num_rows_per_partition = num_rows / num_rows_partition;
	int num_cells_per_partition = num_rows_per_partition * num_rows_per_partition;
	*/
	flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
	int total_num_cells = sd.num_entities(1);
	int total_num_vertices = sd.num_entities(0);
	int num_cells_per_color = total_num_cells / total_num_colors;
	int num_vertices_per_color = total_num_vertices / total_num_colors;
	printf("rank %d, num_vertex %d, num_colors %d, num_vertices_per_color %d, cell_start %d, vertex_start %d\n", 
		my_rank, vertex_domain.get_volume(), total_num_colors, num_vertices_per_color, cell_starting_point, vertex_starting_point);
									 
	auto partetis_dcrs = flecsi::coloring::make_dcrs(sd);
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();
	auto color_results = colorer->parmetis_color(partetis_dcrs);
	
	// init cell
	int ct = 0;								 
  int cell_to_vertex_count = 0;
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
	  int cell_id = cell_starting_point + ct;
#if 0
		int y_idx = cell_id / num_rows;
		int x_idx = cell_id % num_rows;
		int y_par_idx = y_idx / num_rows_per_partition;
		int x_par_idx = x_idx / num_rows_per_partition;
		//cell_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(y_par_idx * num_rows_partition + x_par_idx);
#endif
		cell_id_acc[*pir] = cell_id;
    cell_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(color_results[ct]);
		std::vector<size_t> vertices_of_cell = sd.entities(2, 0, cell_id);
		printf("rank %d, cell_id %d, cell_id_acc %d, new color %d, V(", my_rank, cell_id, (int)cell_id_acc[*pir], color_results[ct]);
		for(int i = 0; i < vertices_of_cell.size(); i++) {
			printf("%d ", vertices_of_cell[i]);
		}
		printf(")\n");
		cell_to_vertex_count += vertices_of_cell.size();
		ct ++;
	}
	
	ct = 0;								 
	for (Legion::PointInDomainIterator<1> pir(vertex_domain); pir(); pir++) {
	  int vertex_id = vertex_starting_point +  ct;
		vertex_id_acc[*pir] = vertex_id;
		vertex_color_id_acc[*pir] = 99.9;
		vertex_color_acc[*pir] = LegionRuntime::Arrays::Point<1>(99);
		printf("rank %d, vertex_id %d, vertex_id_acc %d\n", my_rank, vertex_id, (int)vertex_id_acc[*pir]);
		ct ++;
	}

	auto dcrs = flecsi::coloring::make_dcrs<2,2,2,0>(sd);
	printf("rank %d, index size %d\n\n", my_rank, dcrs.indices.size());
	init_mesh_task_rt_t rt_value;
	rt_value.cell_to_cell_count = dcrs.indices.size();
	rt_value.cell_to_vertex_count = cell_to_vertex_count;
	return rt_value;
}

/*!
 init mesh task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(init_adjacency_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_WRITE,int,1> cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_WRITE,LegionRuntime::Arrays::Point<1>,1> cell_color_acc(regions[0], FID_CELL_PARTITION_COLOR);
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
	
  Legion::Domain cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
  Legion::Domain cell_to_cell_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
  Legion::Domain cell_to_vertex_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
							 
	flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
	auto dcrs = flecsi::coloring::make_dcrs<2,2,2,0>(sd);
	
	assert(cell_to_cell_domain.get_volume() == dcrs.indices.size());
	printf("rank %d, index size %d, cell %d, cell2cell %d, cell2vertex %ld\n", my_rank, dcrs.indices.size(), cell_domain.get_volume(), cell_to_cell_domain.get_volume(), cell_to_vertex_domain.get_volume());
	printf("rank %d, index ", my_rank);
	for (int i = 0; i < dcrs.indices.size(); i++) {
		printf("%d ", dcrs.indices[i]);
	}
	printf("\n");
	
	printf("rank %d, offset", my_rank);
	for (int i = 0; i < dcrs.offsets.size(); i++) {
		printf("%d ", dcrs.offsets[i]);
	}
	printf("\n");
	
	int idx_cell2cell = 0;
	int idx_cell2vertex = 0;
	Legion::PointInDomainIterator<1> pir_cell2vertex(cell_to_vertex_domain);
	for (Legion::PointInDomainIterator<1> pir(cell_domain); pir(); pir++) {
		int cell_id = cell_id_acc[*pir];
		printf("rank %d, cell %d[%d - %d]\n", my_rank, cell_id, dcrs.offsets[idx_cell2cell], dcrs.offsets[idx_cell2cell+1]-1);
		cell_cell_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell]),
		  LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace[my_rank] + dcrs.offsets[idx_cell2cell+1]-1));
		idx_cell2cell++;
		
		// find the vertex of a cell and fill into the cell2vertex 
		printf("rank %d, cell2vetex %d(", my_rank, cell_id);
		std::vector<size_t> vertices_of_cell = sd.entities(2, 0, cell_id);
		for (int i = 0; i < vertices_of_cell.size(); i++) {
			printf("%d,", vertices_of_cell[i]);
			cell_to_vertex_id_acc[*pir_cell2vertex] = vertices_of_cell[i];
			cell_to_vertex_ptr_acc[*pir_cell2vertex] = LegionRuntime::Arrays::Point<1>(cell_to_vertex_id_acc[*pir_cell2vertex]); 
			pir_cell2vertex ++;
		}
		printf(")[%d,%d]; \n", cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex, cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex + vertices_of_cell.size() -1);
		cell_vertex_nrange_acc[*pir] = LegionRuntime::Arrays::Rect<1>(
	    LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex),
		  LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace[my_rank] + idx_cell2vertex + vertices_of_cell.size() -1));
		idx_cell2vertex += vertices_of_cell.size();

	}
	printf("\n");
	idx_cell2cell = 0;
  for (Legion::PointInDomainIterator<1> pir(cell_to_cell_domain); pir(); pir++) {
		cell_to_cell_id_acc[*pir] = dcrs.indices[idx_cell2cell];
		cell_to_cell_ptr_acc[*pir] = LegionRuntime::Arrays::Point<1>(cell_to_cell_id_acc[*pir]); 
		idx_cell2cell ++;
	}
	
	printf("rank %d, cell to vertex ", my_rank);
  for (Legion::PointInDomainIterator<1> pir(cell_to_vertex_domain); pir(); pir++) {
		printf("%d ", (int)cell_to_vertex_id_acc[*pir]);
	}
	printf("\n\n");
}

/*!
 verify vertex color task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(verify_vertex_color_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_alias_id_acc(regions[0], FID_VERTEX_ID);
//	const Legion::FieldAccessor<READ_ONLY,double,1> vertex_alias_color_id_acc(regions[0], FID_VERTEX_PARTITION_COLOR_ID);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> vertex_alias_color_id_acc(regions[0], FID_VERTEX_PARTITION_COLOR);
	//const Legion::FieldAccessor<READ_ONLY,int,1> ghost_cell_id_acc(regions[1], FID_CELL_ID);
	
	int ct = 0;
  Legion::Domain vertex_alias_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
									 
	printf("alias vertex rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_alias_domain); pir(); pir++) {
		//	printf("%d:%.0f ", (int)vertex_alias_id_acc[*pir], (double)vertex_alias_color_id_acc[*pir]);
		LegionRuntime::Arrays::Point<1> pt = vertex_alias_color_id_acc[*pir];
	  printf("%d:%d ", (int)vertex_alias_id_acc[*pir], pt.x[0]);
		ct ++;
	}
	printf(" total %d\n", ct);
}

/*!
 verify dp task

 @ingroup legion-execution
 */

__flecsi_internal_legion_task(verify_dp_task, void) {
	const int my_rank = runtime->find_local_MPI_rank();
	const Legion::FieldAccessor<READ_ONLY,int,1> primary_cell_id_acc(regions[0], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,LegionRuntime::Arrays::Point<1>,1> cell_color_acc(regions[0], FID_CELL_PARTITION_COLOR);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> ghost_cell_id_acc(regions[1], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> shared_cell_id_acc(regions[2], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> execlusive_cell_id_acc(regions[3], FID_CELL_ID);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> primary_vertex_id_acc(regions[4], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> ghost_vertex_id_acc(regions[5], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> shared_vertex_id_acc(regions[6], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> exclusive_vertex_id_acc(regions[7], FID_VERTEX_ID);
	
	const Legion::FieldAccessor<READ_ONLY,int,1> vertex_of_ghost_cell_id_acc(regions[8], FID_VERTEX_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> reachable_cell_id_acc(regions[9], FID_CELL_ID);
	const Legion::FieldAccessor<READ_ONLY,int,1> cell_primary_image_nrange_id_acc(regions[10], FID_CELL_TO_CELL_ID);
	
	int ct = 0;
  Legion::Domain cell_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
	
	printf("[Cell] primary rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_primary_domain); pir(); pir++) {
	  if (ct == 0) {
		  int color = cell_color_acc.read(*pir);
			printf("parmetis color %d, ", color);
		}
		printf("%d ", (int)primary_cell_id_acc[*pir]);
	  ct ++;
	}
	printf(" CP total %d\n", ct);
	
	ct = 0;
  Legion::Domain cell_ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[1].region.get_index_space());
	
	printf("[Cell] ghost rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_ghost_domain); pir(); pir++) {
	  printf("%d ", (int)ghost_cell_id_acc[*pir]);
		ct ++;
	}
  printf(" CG total %d\n", ct);
	
	ct = 0;
  Legion::Domain cell_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[2].region.get_index_space());
	
	printf("[Cell] shared rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_shared_domain); pir(); pir++) {
	  printf("%d ", (int)shared_cell_id_acc[*pir]);
		ct ++;
	}
	printf(" CS total %d\n", ct);
	
	ct = 0;
  Legion::Domain cell_execlusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[3].region.get_index_space());
	
	printf("[Cell] execlusive rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(cell_execlusive_domain); pir(); pir++) {
	  printf("%d ", (int)execlusive_cell_id_acc[*pir]);
		ct ++;
	}
	printf(" CE total %d\n", ct);
	
	ct = 0;
  Legion::Domain vertex_primary_domain = runtime->get_index_space_domain(ctx,
                   task->regions[4].region.get_index_space());
	
	printf("[Vertex] primary rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_primary_domain); pir(); pir++) {
	  printf("%d ", (int)primary_vertex_id_acc[*pir]);
		ct ++;
	}
	printf(" VP total %d\n", ct);
	
	ct = 0;
  Legion::Domain vertex_ghost_domain = runtime->get_index_space_domain(ctx,
                   task->regions[5].region.get_index_space());
	
	printf("[Vertex] ghost rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_ghost_domain); pir(); pir++) {
	  printf("%d ", (int)ghost_vertex_id_acc[*pir]);
		ct ++;
	}
	printf(" VG total %d\n", ct);
	
	ct = 0;
  Legion::Domain vertex_shared_domain = runtime->get_index_space_domain(ctx,
                   task->regions[6].region.get_index_space());
	
	printf("[Vertex] shared rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_shared_domain); pir(); pir++) {
	  printf("%d ", (int)shared_vertex_id_acc[*pir]);
		ct ++;
	}
	printf(" VS total %d\n", ct);
	
	ct = 0;
  Legion::Domain vertex_exclusive_domain = runtime->get_index_space_domain(ctx,
                   task->regions[7].region.get_index_space());
	
	printf("[Vertex] exclusive rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_exclusive_domain); pir(); pir++) {
	  printf("%d ", (int)exclusive_vertex_id_acc[*pir]);
		ct ++;
	}
	printf(" VE total %d\n", ct);
	
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
	
	printf("\n");
}



/*!
  FIXME DEOCUMENTATION
 */
struct MaxReductionOp {
  static const Legion::ReductionOpID redop_id = (size_t(1) << 20) - 4095;

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
  static const Legion::ReductionOpID redop_id = (size_t(1) << 20) - 4096;

  typedef double LHS;
  typedef double RHS;
  static const double identity;

  template<bool EXCLUSIVE>
  static void apply(LHS & lhs, RHS rhs);

  template<bool EXCLUSIVE>
  static void fold(RHS & rhs1, RHS rhs2);
};

struct MinReductionPointOp {
  static const Legion::ReductionOpID redop_id = (size_t(1) << 20) - 4097;

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
	//const Legion::FieldAccessor<,int,1> vertex_alias_id_acc(regions[0], FID_VERTEX_ID);
//	const Legion::FieldAccessor<REDUCE,double,1,Legion::coord_t
	//				,Realm::AffineAccessor<double,1,Legion::coord_t
//					> > vertex_alias_color_acc(regions[0], FID_VERTEX_PARTITION_COLOR_ID, MinReductionOp::redop_id);
	//const Legion::FieldAccessor<READ_ONLY,int,1> ghost_cell_id_acc(regions[1], FID_CELL_ID);
/*		const Legion::FieldAccessor<REDUCE,LegionRuntime::Arrays::Point<1>,1,Legion::coord_t,
					Realm::AffineAccessor<LegionRuntime::Arrays::Point<1>,1,Legion::coord_t
								> > vertex_alias_color_acc(regions[0], FID_VERTEX_PARTITION_COLOR, MinReductionPointOp::redop_id);
*/	
	const Legion::ReductionAccessor<MinReductionPointOp, false, 1, Legion::coord_t,
																	Realm::AffineAccessor<LegionRuntime::Arrays::Point<1>,1,Legion::coord_t>>
																  vertex_alias_color_acc(regions[0], FID_VERTEX_PARTITION_COLOR, MinReductionPointOp::redop_id); 
																	
	int ct = 0;
  Legion::Domain vertex_alias_domain = runtime->get_index_space_domain(ctx,
                   task->regions[0].region.get_index_space());
									 
	printf("alias vertex rank %d, ", my_rank);								 
  for (Legion::PointInDomainIterator<1> pir(vertex_alias_domain); pir(); pir++) {
//		printf("%d ", (int)vertex_alias_id_acc[*pir]);
		int color = my_rank;
		//vertex_alias_color_acc.reduce<MinReductionOp, false>(*pir, color);
		vertex_alias_color_acc.reduce(*pir, LegionRuntime::Arrays::Point<1>(color));
	//	vertex_alias_color_acc[*pir] <<= LegionRuntime::Arrays::Point<1>(color);
	  ct ++;
	}
	printf(" total %d\n", ct);
}

#undef __flecsi_internal_legion_task

} // namespace execution
} // namespace flecsi
