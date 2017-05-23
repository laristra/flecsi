/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_legion_tasks_h
#define flecsi_execution_legion_legion_tasks_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <legion.h>
#include <legion_stl.h>
#include <legion_utilities.h>

#include "flecsi/execution/legion/internal_task.h"
#include "flecsi/execution/legion/internal_field.h"

#define PRIMARY_PART 0
#define GHOST_PART 1
#define EXCLUSIVE_PART 0
#define SHARED_PART 1
#define OWNER_COLOR_TAG 1

typedef Legion::STL::map<LegionRuntime::Arrays::coord_t, LegionRuntime::Arrays::coord_t> legion_map;


clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_legion_task
//!
//! This macro simplifies pure Legion task definitions by filling in the
//! boiler-plate function arguments.
//!
//! @param task_name   The plain-text task name.
//! @param return_type The return type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#define __flecsi_internal_legion_task(task_name, return_type)                  \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
/* Legion task template */                                                     \
inline return_type task_name(                                                  \
  const LegionRuntime::HighLevel::Task * task,                                 \
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,       \
  LegionRuntime::HighLevel::Context ctx,                                       \
  LegionRuntime::HighLevel::HighLevelRuntime * runtime                         \
)

//----------------------------------------------------------------------------//
//! Fix ghost refs task updates ghost reference/pointer to new location
//! by reading from old location.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(fix_ghost_refs_task, void) {

  {
  clog_tag_guard(legion_tasks);
  clog(trace) << "Executing fix ghost refs task " << std::endl;
  }

  clog_assert(regions.size() >= 1, "fix_ghost_refs_task called with no regions");
  clog_assert(task->regions.size() >= 1, "fix_ghost_refs_task called with no regions");
  for (int region_idx = 0; region_idx < regions.size(); region_idx++)
    clog_assert(task->regions[region_idx].privilege_fields.size() == 1,
        "fix_ghost_refs_task called with wrong number of fields");

  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  legion_map owner_map = task->futures[0].get_result<legion_map>();

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  if (owner_map.size() > 0) {

    std::vector<LegionRuntime::Accessor::RegionAccessor<generic_type,
      LegionRuntime::Arrays::Point<2>>> accs_owners_refs;
    std::vector<LegionRuntime::Arrays::Rect<2>> owners_rects;
    for (int owner_idx = 0; owner_idx < owner_map.size(); owner_idx++) {
      accs_owners_refs.push_back(regions[1+owner_idx].get_field_accessor(ghost_owner_pos_fid)
          .typeify<LegionRuntime::Arrays::Point<2>>());
      Legion::Domain owner_domain = runtime->get_index_space_domain(ctx,
          regions[1+owner_idx].get_logical_region().get_index_space());
      owners_rects.push_back(owner_domain.get_rect<2>());
    }

    LegionRuntime::Accessor::RegionAccessor<generic_type,
      LegionRuntime::Arrays::Point<2>> acc_ghost_ref
      = regions[0].get_field_accessor(ghost_owner_pos_fid)
      .typeify<LegionRuntime::Arrays::Point<2>>();
    Legion::Domain ghost_domain = runtime->get_index_space_domain(ctx, regions[0]
      .get_logical_region().get_index_space());
    LegionRuntime::Arrays::Rect<2> ghost_rect = ghost_domain.get_rect<2>();
    for (LegionRuntime::Arrays::GenericPointInRectIterator<2> itr(ghost_rect); itr; itr++) {
      auto ghost_ptr = Legion::DomainPoint::from_point<2>(itr.p);
      LegionRuntime::Arrays::Point<2> ghost_ref = acc_ghost_ref.read(ghost_ptr);
      clog(trace) << "points to " << ghost_ref.x[0] << "," << ghost_ref.x[1] << " local mirror is " <<
          ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1] <<
          " nbr " << owner_map[ghost_ref.x[0]] <<
          " range " << owners_rects[owner_map[ghost_ref.x[0]]].lo[0] <<
          ":" << owners_rects[owner_map[ghost_ref.x[0]]].lo[1] <<
          "," << owners_rects[owner_map[ghost_ref.x[0]]].hi[1] << std::endl;
      clog_assert(ghost_ref.x[0] == owners_rects[owner_map[ghost_ref.x[0]]].lo[0],
          "owner map error");
      clog_assert(ghost_ref.x[1] >= owners_rects[owner_map[ghost_ref.x[0]]].lo[1],
          "underflow error");
      clog_assert(ghost_ref.x[1] <= owners_rects[owner_map[ghost_ref.x[0]]].hi[1],
          "overflow error");
#if 0
      // NOTE: We stored a forward pointer in old shared location to new shared location
      LegionRuntime::Arrays::Point<2> owner_ref =
          accs_owners_refs[owner_map[ghost_ref.x[0]]].read(Legion::DomainPoint::from_point<2>(ghost_ref));
      acc_ghost_ref.write(ghost_ptr, owner_ref);
      clog(error) << ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1] << " points to " << owner_ref.x[0] <<
          "," << owner_ref.x[1] << std::endl;
#endif
    } // for itr
  } // if we have owners

} // fix_ghost_refs_task

//----------------------------------------------------------------------------//
//! Initial SPMD task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(spmd_task, void) {
  const int my_color = task->index_point.point_data[0];

  // spmd_task is an inner task
  runtime->unmap_all_regions(ctx);

  {
  clog_tag_guard(legion_tasks);
  clog(info) << "Executing spmd task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  clog_assert(task->arglen > 0, "spmd_task called without arguments");

  Legion::Deserializer args_deserializer(task->args, task->arglen);
  size_t num_handles;
  args_deserializer.deserialize(&num_handles, sizeof(size_t));

  clog_assert(regions.size() >= num_handles, "fewer regions than data handles");
  clog_assert(task->regions.size() >= num_handles, "fewer regions than data handles");

  Legion::PhaseBarrier* pbarriers_as_master = (Legion::PhaseBarrier*)
      malloc(sizeof(Legion::PhaseBarrier) * num_handles);
  args_deserializer.deserialize((void*)pbarriers_as_master, sizeof(Legion::PhaseBarrier) * num_handles);

  size_t* num_owners = (size_t*)malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize((void*)num_owners, sizeof(size_t) * num_handles);

  context_.set_pbarriers_as_masters(pbarriers_as_master);

  std::vector<Legion::PhaseBarrier*> ghost_owners_pbarriers;

  for (size_t handle_idx = 0; handle_idx < num_handles; handle_idx++) {

    Legion::PhaseBarrier* ghost_owners_pbarriers_buf = (Legion::PhaseBarrier*)
        malloc(sizeof(Legion::PhaseBarrier) * num_owners[handle_idx]);
    args_deserializer.deserialize((void*)ghost_owners_pbarriers_buf, sizeof(Legion::PhaseBarrier) * num_owners[handle_idx]);

    ghost_owners_pbarriers.push_back(ghost_owners_pbarriers_buf); // FIXME free and clear after driver
    context_.push_ghost_owners_pbarriers(ghost_owners_pbarriers_buf);
  }

  size_t num_fields;
  args_deserializer.deserialize(&num_fields, sizeof(size_t));

  using field_info_t = context_t::field_info_t;
  auto field_info_buf = 
    (field_info_t*)malloc(sizeof(field_info_t) * num_fields);
  
  args_deserializer.deserialize(field_info_buf,
                                sizeof(field_info_t) * num_fields);
  
  for(size_t i = 0; i < num_fields; ++i){
    field_info_t& fi = field_info_buf[i];
    context_.put_field_info(fi);
  }

  std::vector<std::vector<Legion::LogicalRegion>> ghost_owners_lregions(num_handles);
  std::vector<legion_map> global_to_local_color_map(num_handles);

  size_t region_index = 0;
  for (size_t handle_idx = 0; handle_idx < num_handles; handle_idx++) {

    context_.push_color_region(regions[region_index].get_logical_region());

    const std::unordered_map<size_t, flecsi::coloring::coloring_info_t> coloring_info_map
      = context_.coloring_info(handle_idx);  // FIX_ME what if the keys are not 0,1,2,...

    auto itr = coloring_info_map.find(my_color);
    clog_assert(itr != coloring_info_map.end(), "Can't find partition info for my color");
    const flecsi::coloring::coloring_info_t coloring_info = itr->second;

    clog(trace) << my_color << " handle " << handle_idx <<
        " exclusive " << coloring_info.exclusive <<
        " shared " << coloring_info.shared <<
        " ghost " << coloring_info.ghost << std::endl;

    Legion::IndexSpace color_ispace = regions[region_index].get_logical_region().get_index_space();
    LegionRuntime::Arrays::Rect<1> color_bounds_1D(0,1);
    Legion::Domain color_domain_1D = Legion::Domain::from_rect<1>(color_bounds_1D);

    Legion::DomainColoring primary_ghost_coloring;
    LegionRuntime::Arrays::Rect<2> primary_rect(LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared - 1));
    primary_ghost_coloring[PRIMARY_PART] = Legion::Domain::from_rect<2>(primary_rect);
    LegionRuntime::Arrays::Rect<2> ghost_rect(LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared
            + coloring_info.ghost - 1));
    primary_ghost_coloring[GHOST_PART] = Legion::Domain::from_rect<2>(ghost_rect);

    Legion::IndexPartition primary_ghost_ip = runtime->create_index_partition(ctx,
        color_ispace, color_domain_1D, primary_ghost_coloring, true /*disjoint*/);

    context_.push_primary_ghost_ip(primary_ghost_ip);

    Legion::LogicalPartition primary_ghost_lp = runtime->get_logical_partition(ctx,
        regions[region_index].get_logical_region(), primary_ghost_ip);
    region_index++;

    context_.push_primary_lr(runtime->get_logical_subregion_by_color(ctx,
      primary_ghost_lp, PRIMARY_PART));

    context_.push_ghost_lr(runtime->get_logical_subregion_by_color(ctx,
      primary_ghost_lp, GHOST_PART));

    Legion::DomainColoring excl_shared_coloring;
    LegionRuntime::Arrays::Rect<2> exclusive_rect(LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive - 1));
    excl_shared_coloring[EXCLUSIVE_PART] = Legion::Domain::from_rect<2>(exclusive_rect);
    LegionRuntime::Arrays::Rect<2> shared_rect(LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared - 1));
    excl_shared_coloring[SHARED_PART] = Legion::Domain::from_rect<2>(shared_rect);

    Legion::IndexPartition excl_shared_ip = runtime->create_index_partition(ctx,
        color_ispace, color_domain_1D, excl_shared_coloring, true /*disjoint*/);

    context_.push_excl_shared_ip(excl_shared_ip);

    Legion::LogicalPartition excl_shared_lp = runtime->get_logical_partition(ctx,
        context_.get_primary_lr(handle_idx), excl_shared_ip);

    context_.push_exclusive_lr(runtime->get_logical_subregion_by_color(ctx,
      excl_shared_lp, EXCLUSIVE_PART));

    context_.push_shared_lr(runtime->get_logical_subregion_by_color(ctx,
      excl_shared_lp, SHARED_PART));

    // Add neighbors regions to context_
    for (size_t owner = 0; owner < num_owners[handle_idx]; owner++) {
      ghost_owners_lregions[handle_idx].push_back(regions[region_index].get_logical_region());
      const void* owner_color;
      size_t size;
      const bool can_fail = false;
      const bool wait_until_ready = true;
      runtime->retrieve_semantic_information(regions[region_index].get_logical_region(), OWNER_COLOR_TAG,
          owner_color, size, can_fail, wait_until_ready);
      clog_assert(size == sizeof(LegionRuntime::Arrays::coord_t), "Unable to map gid to lid with Legion semantic tag");
      global_to_local_color_map[handle_idx][*(LegionRuntime::Arrays::coord_t*)owner_color] = owner;
      clog(trace) << my_color << " key " << handle_idx << " gid " << *(LegionRuntime::Arrays::coord_t*)owner_color <<
          " maps to " << owner << std::endl;
      region_index++;
      clog_assert(region_index <= regions.size(), "SPMD attempted to access more regions than passed");
    } // for owner
    context_.push_ghost_owners_lregions(ghost_owners_lregions[handle_idx]);
    context_.push_global_to_local_color_map(global_to_local_color_map[handle_idx]);

    // Fix ghost reference/pointer to point to compacted position of shared that it needs
    Legion::TaskLauncher fix_ghost_refs_launcher(context_.task_id<__flecsi_internal_task_key(fix_ghost_refs_task)>(),
        Legion::TaskArgument(nullptr, 0));

    fix_ghost_refs_launcher.add_region_requirement(
        Legion::RegionRequirement(context_.get_ghost_lr(handle_idx), READ_WRITE,
            EXCLUSIVE, context_.get_color_region(handle_idx))
        .add_field(ghost_owner_pos_fid));

    fix_ghost_refs_launcher.add_future(Legion::Future::from_value(runtime,
        global_to_local_color_map[handle_idx]));

    for (size_t owner = 0; owner < num_owners[handle_idx]; owner++)
      fix_ghost_refs_launcher.add_region_requirement(
          Legion::RegionRequirement(ghost_owners_lregions[handle_idx][owner], READ_ONLY, EXCLUSIVE,
              ghost_owners_lregions[handle_idx][owner]).add_field(ghost_owner_pos_fid));

    runtime->execute_task(ctx, fix_ghost_refs_launcher);

  } // for handle_idx

  // Get the input arguments from the Legion runtime
  const LegionRuntime::HighLevel::InputArgs & args =
    LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  // Set the current task context to the driver
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);

  // run default or user-defined driver 
  driver(args.argc, args.argv); 

  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());

  // FIXME free all malloc, vectors, etc.

} // spmd_task

//----------------------------------------------------------------------------//
//! Interprocess communication to pass control to MPI runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to wait for control to pass back to the Legion
//! runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to unset mpi execute state.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task


//----------------------------------------------------------------------------//
//! Compaction task writes new location in old location.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(compaction_task, void) {
  const int my_color = task->index_point.point_data[0];

  {
  clog_tag_guard(legion_tasks);
  clog(trace) << "Executing compaction task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  {
  clog_tag_guard(legion_tasks);

  // In old position of shared, write compacted location
  // In compacted position of ghost, write the reference/pointer to pre-compacted shared
  // ghost reference/pointer will need to communicate with other ranks in spmd_task() to obtain
  // corrected pointer
  for (auto handle : coloring_map) {

    Legion::IndexSpace ispace = regions[handle.first].get_logical_region().get_index_space();
    Legion::FieldID fid_ref = ghost_owner_pos_fid;
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, LegionRuntime::Arrays::Point<2>> acc_ref =
          regions[handle.first].get_field_accessor(fid_ref).typeify<LegionRuntime::Arrays::Point<2>>();

    Legion::Domain domain = runtime->get_index_space_domain(ctx, ispace);
    LegionRuntime::Arrays::Rect<2> rect = domain.get_rect<2>();
    LegionRuntime::Arrays::GenericPointInRectIterator<2> expanded_itr(rect);

    for (auto exclusive_itr = handle.second.exclusive.begin(); exclusive_itr != handle.second.exclusive.end(); ++exclusive_itr) {
      clog(trace) << my_color << " key " << handle.first << " exclusive " <<
        " " <<  *exclusive_itr << std::endl;
      expanded_itr++;
    } // exclusive_itr

    for (auto shared_itr = handle.second.shared.begin(); shared_itr != handle.second.shared.end(); ++shared_itr) {
      const flecsi::coloring::entity_info_t shared = *shared_itr;
      const LegionRuntime::Arrays::Point<2> reference = LegionRuntime::Arrays::make_point(shared.rank,
          shared.offset);
      // reference is the old location, expanded_itr.p is the new location
      acc_ref.write(Legion::DomainPoint::from_point<2>(reference), expanded_itr.p);

      clog(trace) << my_color << " key " << handle.first << " shared was " <<
        " " <<  *shared_itr << " now at " << expanded_itr.p << std::endl;
      expanded_itr++;
    } // shared_itr

    for (auto ghost_itr = handle.second.ghost.begin(); ghost_itr != handle.second.ghost.end(); ++ghost_itr) {
      const flecsi::coloring::entity_info_t ghost = *ghost_itr;
      const LegionRuntime::Arrays::Point<2> reference = LegionRuntime::Arrays::make_point(ghost.rank,
          ghost.offset);
      // reference is where we used to point, expanded_itr.p is where ghost is now
      acc_ref.write(Legion::DomainPoint::from_point<2>(expanded_itr.p), reference);
      clog(error) << "color " << my_color << " key " << handle.first << " ghost " <<
        " " << *ghost_itr <<
        //" now at " << expanded_itr.p <<
        std::endl;
      expanded_itr++;
    } // ghost_itr
  } // for handle
  }

} // compaction_task

#undef __flecsi_internal_legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
