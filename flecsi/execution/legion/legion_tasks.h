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
          "ghost dependency closure error in specialization_driver()");
      clog_assert(ghost_ref.x[1] >= owners_rects[owner_map[ghost_ref.x[0]]].lo[1],
          "ghost owner position error in specialization_driver()");
      clog_assert(ghost_ref.x[1] <= owners_rects[owner_map[ghost_ref.x[0]]].hi[1],
          "ghost owner position error in specialization_driver()");

      // NOTE: We stored a forward pointer in old shared location to new shared location
      LegionRuntime::Arrays::Point<2> owner_ref =
          accs_owners_refs[owner_map[ghost_ref.x[0]]].read(Legion::DomainPoint::from_point<2>(ghost_ref));
      acc_ghost_ref.write(ghost_ptr, owner_ref);

      clog(trace) << ghost_ptr.point_data[0] << "," << ghost_ptr.point_data[1] << " points to " << owner_ref.x[0] <<
          "," << owner_ref.x[1] << std::endl;

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

  auto& ispace_dmap = context_.index_space_data_map();

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  clog_assert(task->arglen > 0, "spmd_task called without arguments");

  Legion::Deserializer args_deserializer(task->args, task->arglen);
  size_t num_idx_spaces;
  args_deserializer.deserialize(&num_idx_spaces, sizeof(size_t));

  size_t* idx_spaces = (size_t*)malloc(sizeof(size_t) * num_idx_spaces);
  args_deserializer.deserialize((void*)idx_spaces,
    sizeof(size_t) * num_idx_spaces);

  for(size_t i = 0; i < num_idx_spaces; ++i){
    context_.add_index_space(idx_spaces[i]);
  }

  clog_assert(regions.size() >= num_idx_spaces, "fewer regions than data handles");
  clog_assert(task->regions.size() >= num_idx_spaces, "fewer regions than data handles");

  Legion::PhaseBarrier* pbarriers_as_master = (Legion::PhaseBarrier*)
      malloc(sizeof(Legion::PhaseBarrier) * num_idx_spaces);
  args_deserializer.deserialize((void*)pbarriers_as_master, sizeof(Legion::PhaseBarrier) * num_idx_spaces);

  size_t* num_owners = (size_t*)malloc(sizeof(size_t) * num_idx_spaces);
  args_deserializer.deserialize((void*)num_owners, sizeof(size_t) * num_idx_spaces);

  for(size_t idx_space : context_.index_spaces()){
    ispace_dmap[idx_space].pbarrier_as_owner_ptr = &pbarriers_as_master[idx_space];
  }

  std::vector<std::vector<Legion::PhaseBarrier>> ghost_owners_pbarriers(num_idx_spaces);

  // FIXME again assuming handles are 0, 1, 2, ..
  for (size_t idx_space : context_.index_spaces()) {
    size_t n = num_owners[idx_space];

    ghost_owners_pbarriers[idx_space].resize(n);
    args_deserializer.deserialize((void*)&ghost_owners_pbarriers[idx_space][0],
        sizeof(Legion::PhaseBarrier) * n);
    
    ispace_dmap[idx_space].ghost_owners_pbarriers_ptrs.resize(n);

    for(size_t owner = 0; owner < n; ++owner){
      ispace_dmap[idx_space].ghost_owners_pbarriers_ptrs[owner] = 
        &ghost_owners_pbarriers[idx_space][owner];
    }
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

  std::vector<std::vector<Legion::LogicalRegion>>
    ghost_owners_lregions(num_idx_spaces);
  
  std::vector<legion_map> global_to_local_color_map(num_idx_spaces);

  size_t region_index = 0;
  for (size_t idx_space : context_.index_spaces()) {

    ispace_dmap[idx_space].color_region = regions[region_index].get_logical_region();

    const std::unordered_map<size_t, flecsi::coloring::coloring_info_t> coloring_info_map
      = context_.coloring_info(idx_space);  // FIX_ME what if the keys are not 0,1,2,...

    auto itr = coloring_info_map.find(my_color);
    clog_assert(itr != coloring_info_map.end(), "Can't find partition info for my color");
    const flecsi::coloring::coloring_info_t coloring_info = itr->second;

    clog(trace) << my_color << " handle " << idx_space <<
        " exclusive " << coloring_info.exclusive <<
        " shared " << coloring_info.shared <<
        " ghost " << coloring_info.ghost << std::endl;

    Legion::IndexSpace color_ispace = 
      regions[region_index].get_logical_region().get_index_space();
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

    ispace_dmap[idx_space].primary_ghost_ip = primary_ghost_ip;

    Legion::LogicalPartition primary_ghost_lp = runtime->get_logical_partition(ctx,
        regions[region_index].get_logical_region(), primary_ghost_ip);
    region_index++;

    ispace_dmap[idx_space].primary_lr = 
    runtime->get_logical_subregion_by_color(ctx, primary_ghost_lp, 
                                            PRIMARY_PART);

    ispace_dmap[idx_space].ghost_lr = 
      runtime->get_logical_subregion_by_color(ctx, primary_ghost_lp, 
                                              GHOST_PART);

    Legion::DomainColoring excl_shared_coloring;
    LegionRuntime::Arrays::Rect<2> exclusive_rect(LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive - 1));
    excl_shared_coloring[EXCLUSIVE_PART] = Legion::Domain::from_rect<2>(exclusive_rect);
    LegionRuntime::Arrays::Rect<2> shared_rect(LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared - 1));
    excl_shared_coloring[SHARED_PART] = Legion::Domain::from_rect<2>(shared_rect);

    Legion::IndexPartition excl_shared_ip = runtime->create_index_partition(ctx,
        ispace_dmap[idx_space].primary_lr.get_index_space(), color_domain_1D, excl_shared_coloring, true /*disjoint*/);

    ispace_dmap[idx_space].excl_shared_ip = excl_shared_ip;

    Legion::LogicalPartition excl_shared_lp = runtime->get_logical_partition(ctx,
        ispace_dmap[idx_space].primary_lr, excl_shared_ip);

    ispace_dmap[idx_space].exclusive_lr = 
    runtime->get_logical_subregion_by_color(ctx, excl_shared_lp, 
                                            EXCLUSIVE_PART);

    ispace_dmap[idx_space].shared_lr = 
    runtime->get_logical_subregion_by_color(ctx, excl_shared_lp, SHARED_PART);

    // Add neighbors regions to context_
    for (size_t owner = 0; owner < num_owners[idx_space]; owner++) {
      ghost_owners_lregions[idx_space].push_back(regions[region_index].get_logical_region());
      const void* owner_color;
      size_t size;
      const bool can_fail = false;
      const bool wait_until_ready = true;
      runtime->retrieve_semantic_information(regions[region_index].get_logical_region(), OWNER_COLOR_TAG,
          owner_color, size, can_fail, wait_until_ready);
      clog_assert(size == sizeof(LegionRuntime::Arrays::coord_t), "Unable to map gid to lid with Legion semantic tag");
      global_to_local_color_map[idx_space][*(LegionRuntime::Arrays::coord_t*)owner_color] = owner;
      clog(trace) << my_color << " key " << idx_space << " gid " << *(LegionRuntime::Arrays::coord_t*)owner_color <<
          " maps to " << owner << std::endl;
      region_index++;
      clog_assert(region_index <= regions.size(), "SPMD attempted to access more regions than passed");
    } // for owner
    ispace_dmap[idx_space].ghost_owners_lregions =ghost_owners_lregions[idx_space];
    ispace_dmap[idx_space].global_to_local_color_map = 
      global_to_local_color_map[idx_space];
    for(auto itr = global_to_local_color_map[idx_space].begin(); itr !=
            global_to_local_color_map[idx_space].end(); itr++)
        clog(error) << my_color << ":" << idx_space << " " << itr->first <<
          " SET AS " << itr->second << std::endl;
    for(auto itr = ispace_dmap[idx_space].global_to_local_color_map.begin(); itr !=
            ispace_dmap[idx_space].global_to_local_color_map.end(); itr++)
        clog(error) << my_color << ":" << idx_space << " " << itr->first <<
          " FIRST COPY " << itr->second << std::endl;


    // Fix ghost reference/pointer to point to compacted position of shared that it needs
    Legion::TaskLauncher fix_ghost_refs_launcher(context_.task_id<__flecsi_internal_task_key(fix_ghost_refs_task)>(),
        Legion::TaskArgument(nullptr, 0));

    fix_ghost_refs_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_lr, READ_WRITE,
            EXCLUSIVE, ispace_dmap[idx_space].color_region)
        .add_field(ghost_owner_pos_fid));

    fix_ghost_refs_launcher.add_future(Legion::Future::from_value(runtime,
        global_to_local_color_map[idx_space]));

    for (size_t owner = 0; owner < num_owners[idx_space]; owner++)
      fix_ghost_refs_launcher.add_region_requirement(
          Legion::RegionRequirement(ghost_owners_lregions[idx_space][owner], READ_ONLY, EXCLUSIVE,
              ghost_owners_lregions[idx_space][owner]).add_field(ghost_owner_pos_fid));

    runtime->execute_task(ctx, fix_ghost_refs_launcher);

  } // for idx_space

  // Get the input arguments from the Legion runtime
  const LegionRuntime::HighLevel::InputArgs & args =
    LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);
#endif

  // run default or user-defined driver 
  driver(args.argc, args.argv); 

#if !defined(ENABLE_LEGION_TLS)
  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());
#endif

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

  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t>
    coloring_map = context_.coloring_map();

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  {
  clog_tag_guard(legion_tasks);

  // In old position of shared, write compacted location
  // In compacted position of ghost, write the reference/pointer to pre-compacted shared
  // ghost reference/pointer will need to communicate with other ranks in spmd_task() to obtain
  // corrected pointer
  for (auto idx_space : coloring_map) {

    Legion::IndexSpace ispace = regions[idx_space.first].get_logical_region().get_index_space();
    LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, LegionRuntime::Arrays::Point<2>> acc_ref =
          regions[idx_space.first].get_field_accessor(ghost_owner_pos_fid).typeify<LegionRuntime::Arrays::Point<2>>();

    Legion::Domain domain = runtime->get_index_space_domain(ctx, ispace);
    LegionRuntime::Arrays::Rect<2> rect = domain.get_rect<2>();
    LegionRuntime::Arrays::GenericPointInRectIterator<2> expanded_itr(rect);

    for (auto exclusive_itr = idx_space.second.exclusive.begin(); exclusive_itr != idx_space.second.exclusive.end(); ++exclusive_itr) {
      clog(trace) << my_color << " key " << idx_space.first << " exclusive " <<
        " " <<  *exclusive_itr << std::endl;
      expanded_itr++;
    } // exclusive_itr

    for (auto shared_itr = idx_space.second.shared.begin(); shared_itr != idx_space.second.shared.end(); ++shared_itr) {
      const flecsi::coloring::entity_info_t shared = *shared_itr;
      const LegionRuntime::Arrays::Point<2> reference = LegionRuntime::Arrays::make_point(shared.rank,
          shared.offset);
      // reference is the old location, expanded_itr.p is the new location
      acc_ref.write(Legion::DomainPoint::from_point<2>(reference), expanded_itr.p);

      clog(trace) << my_color << " key " << idx_space.first << " shared was " <<
        " " <<  *shared_itr << " now at " << expanded_itr.p << std::endl;
      expanded_itr++;
    } // shared_itr

    for (auto ghost_itr = idx_space.second.ghost.begin(); ghost_itr != idx_space.second.ghost.end(); ++ghost_itr) {
      const flecsi::coloring::entity_info_t ghost = *ghost_itr;
      const LegionRuntime::Arrays::Point<2> reference = LegionRuntime::Arrays::make_point(ghost.rank,
          ghost.offset);
      // reference is where we used to point, expanded_itr.p is where ghost is now
      acc_ref.write(Legion::DomainPoint::from_point<2>(expanded_itr.p), reference);
      clog(trace) << "color " << my_color << " key " << idx_space.first << " ghost " <<
        " " << *ghost_itr <<
        //" now at " << expanded_itr.p <<
        std::endl;
      expanded_itr++;
    } // ghost_itr
  } // for idx_space
  }

} // compaction_task

//----------------------------------------------------------------------------//
//! Ghost copy task writes data from shared into ghost
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(ghost_copy_task, void) {
    clog(error) << "COPY TASK" << std::endl;

  context_t& context = context_t::instance();

  struct args_t {
    size_t index_space;
    size_t owner;
  };
  args_t args = *(args_t*)task->args;

  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  // regions 0 and 1 have the same fields except for ghost_owner_pos_fid

  typedef Legion::STL::map<Legion::coord_t, Legion::coord_t> legion_map;
  legion_map neighbor_map = task->futures[0].get_result<legion_map>();

  for(auto itr = neighbor_map.begin(); itr != neighbor_map.end(); itr++)
      clog(error) << itr->first << " MAPS TO " << itr->second << std::endl;

  auto ghost_owner_pos_fid = 
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

  auto acc_position_ref =
    regions[1].get_field_accessor(ghost_owner_pos_fid).typeify<
    LegionRuntime::Arrays::Point<2>>();

  Legion::Domain ghost_domain = runtime->get_index_space_domain(ctx,
          regions[1].get_logical_region().get_index_space());

  for (Legion::Domain::DomainPointIterator ghost_itr(ghost_domain); ghost_itr;
          ghost_itr++) {
      LegionRuntime::Arrays::Point<2> ghost_ref = acc_position_ref.read(ghost_itr.p);
      clog(error) << "position " << ghost_ref.x[0] << "," << ghost_ref.x[1] << std::endl;

      if (neighbor_map[ghost_ref.x[0]] == args.owner)
          clog(error) << "COPY " << neighbor_map[ghost_ref.x[0]] << "==" <<
            args.owner << std::endl;
  }

#if 0
  // For each field, copy data from shared to ghost
  for(auto fid : task->regions[0].privilege_fields){
    // Look up field info in context
    auto iitr = context.field_info_map().find(index_space);
    clog_assert(iitr != context.field_info_map().end(), "invalid index space");
    auto fitr = iitr->second.find(fid);
    clog_assert(fitr != iitr->second.end(), "invalid fid");
    const context_t::field_info_t& fi = fitr->second;

    Legion::PhysicalRegion pr = regions[0];
    Legion::LogicalRegion lr = pr.get_logical_region();
    Legion::IndexSpace is = lr.get_index_space();

    auto acc_shared = regions[0].get_field_accessor(fid);

    Legion::Domain domain = 
      runtime->get_index_space_domain(ctx, is); 
    
    // get raw buffer to shared data
    LegionRuntime::Arrays::Rect<2> r = domain.get_rect<2>();
    LegionRuntime::Arrays::Rect<2> sr;
    LegionRuntime::Accessor::ByteOffset bo[2];
    void* data_shared = acc_shared.template raw_rect_ptr<2>(r, sr, bo);
    data_shared += bo[1];
    size_t size = sr.hi[1] - sr.lo[1] + 1;

    pr = regions[1];
    lr = pr.get_logical_region();
    is = lr.get_index_space();

    auto acc_ghost = regions[1].get_field_accessor(fid);

    domain = runtime->get_index_space_domain(ctx, is); 
    
    // get raw buffer to ghost data
    r = domain.get_rect<2>();
    void* data_ghost = acc_ghost.template raw_rect_ptr<2>(r, sr, bo);
    data_ghost += bo[1];
    size_t shared_size = sr.hi[1] - sr.lo[1] + 1;
    
    clog_assert(size == shared_size, "ghost/shared size mismatch");

    // finally, copy the raw data shared -> ghost
    std::memcpy(data_ghost, data_shared, size * fi.size);
  } // for fid
#endif
}

#undef __flecsi_internal_legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
