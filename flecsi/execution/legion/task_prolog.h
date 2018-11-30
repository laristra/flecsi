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

#include <vector>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/data/data.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/internal_field.h>

#include <flecsi/utils/const_string.h>
#include <flecsi/utils/tuple_walker.h>

clog_register_tag(prolog);

namespace flecsi {
namespace execution {

/*!
 The task_prolog_t type can be called to walk the task args after the
 task launcher is created, but before the task has run. This allows
 synchronization dependencies to be added to the execution flow.

 @ingroup execution
 */

struct task_prolog_t : public flecsi::utils::tuple_walker_u<task_prolog_t> {

  /*!
   Construct a task_prolog_t instance.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  task_prolog_t(
      Legion::Runtime * runtime,
      Legion::Context & context,
      Legion::Domain & color_domain)
      : runtime(runtime), context(context), color_domain(color_domain) {
  } // task_prolog_t

  /*!
   Walk the data handles for a flecsi task, store info for ghost copies
   in member variables, and add phase barriers to launcher as needed.

   @tparam T                     The data type referenced by the handle.
   @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                                 indices of the index partition.
   @tparam SHARED_PERMISSIONS    The permissions required on the shared
                                 indices of the index partition.
   @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                                 indices of the index partition.

   @param runtime The Legion task runtime.
   @param context The Legion task runtime context.
   */

  template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS>
  void handle(dense_accessor_u<
              T,
              EXCLUSIVE_PERMISSIONS,
              SHARED_PERMISSIONS,
              GHOST_PERMISSIONS> & a) {
    if(sparse){
      return;
    }

    auto & h = a.handle;

    if(!h.global && !h.color) {
      auto & flecsi_context = context_t::instance();

      bool read_phase = false;
      bool write_phase = false;
      const int my_color = runtime->find_local_MPI_rank();

      read_phase = GHOST_PERMISSIONS != na;
      write_phase = (SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw);

      if(read_phase) {
        if(!*(h.ghost_is_readable)) {
          {
            clog_tag_guard(prolog);
            clog(trace) << "rank " << my_color << " READ PHASE PROLOGUE"
                        << std::endl;
          } // scope

          // As user

          ghost_owners_partitions.push_back(h.ghost_owners_lp);
//          owner_subregion_partitions.push_back(h.ghost_owners_subregion_lp);
          ghost_partitions.push_back(h.ghost_lp);
          entire_regions.push_back(h.entire_region);
          fids.push_back(h.fid);
          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          local_args.sparse = false;
          args.push_back(local_args);

          *(h.ghost_is_readable) = true;

        } // !ghost_is_readable
      } // read_phase

      if (write_phase && (*h.ghost_is_readable)) {
        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
      } // if
    } // end if

  } // handle


  Legion::LogicalPartition
  create_ghost_owners_partition_for_sparse_entries (size_t idx_space,
		size_t fid)
	{
      auto & context_ = context_t::instance();
      auto& ispace_dmap = context_.index_space_data_map();
      //auto& flecsi_ispace = data.index_space(idx_space);
      //auto& flecsi_sis = data.sparse_index_space(idx_space);
      size_t sparse_idx_space = idx_space + 8192;

      using field_info_t = context_t::field_info_t;

      auto ghost_owner_pos_fid =
        LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

      auto constexpr key =
          flecsi::utils::const_string_t{
						EXPAND_AND_STRINGIFY(sparse_set_owner_position_task)}.hash();

      const auto sparse_set_pos_id = context_.task_id<key>();

      Legion::IndexLauncher sparse_pos_launcher(sparse_set_pos_id,
        color_domain, Legion::TaskArgument(nullptr, 0),
        Legion::ArgumentMap());

      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_lp,
            0/*projection ID*/,
            READ_ONLY, EXCLUSIVE, ispace_dmap[idx_space].entire_region))
                .add_field(ghost_owner_pos_fid);
      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[idx_space].ghost_owners_lp,
            0/*projection ID*/,
            READ_ONLY, EXCLUSIVE, ispace_dmap[idx_space].entire_region))
                .add_field(fid);
      sparse_pos_launcher.add_region_requirement(
        Legion::RegionRequirement(ispace_dmap[sparse_idx_space].ghost_lp,
            0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE,
						ispace_dmap[sparse_idx_space].entire_region))
                .add_field(ghost_owner_pos_fid);

      sparse_pos_launcher.tag = MAPPER_FORCE_RANK_MATCH;
      auto future = runtime->execute_index_space(context, sparse_pos_launcher);
      future.wait_all_results(false);

      Legion::LogicalRegion sis_primary_lr =
          ispace_dmap[sparse_idx_space].entire_region;
//				ispace_dmap[sparse_idx_space].primary_lp.get_logical_region();

      Legion::IndexSpace is_of_colors = runtime->create_index_space(context,
        color_domain);

      ispace_dmap[sparse_idx_space].ghost_owners_ip =
        runtime->create_partition_by_image(context,
        sis_primary_lr.get_index_space(),
        ispace_dmap[sparse_idx_space].ghost_lp,
        ispace_dmap[sparse_idx_space].entire_region,
				ghost_owner_pos_fid, is_of_colors);

     runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_owners_ip,
        "ghost owners index partition");
     ispace_dmap[sparse_idx_space].ghost_owners_lp =
      runtime->get_logical_partition(
      context, sis_primary_lr, ispace_dmap[sparse_idx_space].ghost_owners_ip);
     runtime->attach_name(ispace_dmap[sparse_idx_space].ghost_owners_lp,
        "ghost owners logical partition");
 
    return ispace_dmap[sparse_idx_space].ghost_owners_lp;
  }

  /*!
   Walk the data handles for a flecsi task, store info for ghost copies
   in member variables, and add phase barriers to launcher as needed.

   Use member variables initialized by the walk to launch 1 copy per owner
   region

   */

  void launch_copies() {
    auto & flecsi_context = context_t::instance();

    // group by ghost_owners_partition
    std::vector<std::set<size_t>> handle_groups;
    std::vector<bool> is_sparse_group;
    for (size_t handle{0}; handle < ghost_owners_partitions.size(); handle++) {
      bool found_group = false;
      for (size_t group{0}; group < handle_groups.size(); group++) {
        auto first = handle_groups[group].begin();
        if (ghost_owners_partitions[handle] == ghost_owners_partitions[*first]) {
          handle_groups[group].insert(handle);
          found_group = true;
          continue;
        }
      } // for group
      if(!found_group) {
        std::set<size_t> new_group;
        new_group.insert(handle);
        handle_groups.push_back(new_group);
        is_sparse_group.push_back(args[handle].sparse);
      }
    }  // for handle

    // launch copy task per group of handles with same ghost_owners_partition
    for (size_t group{0}; group < handle_groups.size(); group++) {
      bool is_sparse = is_sparse_group[group];
      auto first_itr = handle_groups[group].begin();
      size_t first = *first_itr;

      Legion::RegionRequirement rr_owners(ghost_owners_partitions[first],
          0/*projection ID*/, READ_ONLY, EXCLUSIVE, entire_regions[first]);
      Legion::RegionRequirement rr_ghost(ghost_partitions[first],
          0/*projection ID*/, READ_WRITE, EXCLUSIVE, entire_regions[first]);

      Legion::RegionRequirement rr_entries_shared;

      Legion::RegionRequirement rr_entries_ghost;

      if(is_sparse){
        rr_entries_shared =
          Legion::RegionRequirement(
          ghost_owner_entries_partitions[first], 0, READ_ONLY, SIMULTANEOUS,
          entries_regions[first]);

        rr_entries_ghost =
          Legion::RegionRequirement(
          ghost_entries_partitions[first], 0,  READ_WRITE, SIMULTANEOUS,
          entries_regions[first]);        
      }

      auto ghost_owner_pos_fid =
        LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);

      rr_ghost.add_field(ghost_owner_pos_fid);
      rr_entries_ghost.add_field(ghost_owner_pos_fid);

      // TODO - circular dependency including internal_task.h
      auto constexpr key =
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(ghost_copy_task)}
              .hash();

      const auto ghost_copy_tid = flecsi_context.task_id<key>();

      Legion::IndexLauncher ghost_launcher(ghost_copy_tid, color_domain,
          Legion::TaskArgument(&args[first], sizeof(args[first])),
          Legion::ArgumentMap());

      for (auto handle_itr = handle_groups[group].begin();
          handle_itr != handle_groups[group].end(); handle_itr++) {
        size_t handle = *handle_itr;

        rr_owners.add_field(fids[handle]);
        rr_ghost.add_field(fids[handle]);

         if(is_sparse){
          rr_entries_shared.add_field(fids[handle]);
          rr_entries_ghost.add_field(fids[handle]);
         }

      }

      ghost_launcher.add_region_requirement(rr_owners);
      ghost_launcher.add_region_requirement(rr_ghost);

      if(is_sparse){
        ghost_launcher.add_region_requirement(rr_entries_shared);
        ghost_launcher.add_region_requirement(rr_entries_ghost);
      }

      ghost_launcher.tag = MAPPER_FORCE_RANK_MATCH;
      runtime->execute_index_space(context, ghost_launcher);
    } // for group

  } // launch copies

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  void handle(
    sparse_accessor <
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS
    > &a
  )
  {
    if(!sparse){
      return;
    }

    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & h = a.handle;

    auto & flecsi_context = context_t::instance();

    bool read_phase = false;
    bool write_phase = false;
    const int my_color = runtime->find_local_MPI_rank();

    read_phase = GHOST_PERMISSIONS != na;
    write_phase = (SHARED_PERMISSIONS == wo) || (SHARED_PERMISSIONS == rw);

    if (read_phase) {
      if (!*(h.ghost_is_readable)) {
          clog_tag_guard(prolog);
          clog(trace) << "rank " << my_color << " READ PHASE PROLOGUE"
                      << std::endl;

          // offsets
          ghost_owners_partitions.push_back(h.ghost_owners_offsets_lp);
//          owner_subregion_partitions.push_back(
//			h.ghost_owners_offsets_subregion_lp);

          if ( h.ghost_owners_entries_lp==Legion::LogicalPartition::NO_PART)
             h.ghost_owners_entries_lp=
							create_ghost_owners_partition_for_sparse_entries(h.index_space,
								h.fid);  
          ghost_owner_entries_partitions.push_back(
             h.ghost_owners_entries_lp);
         
          entire_regions.push_back(h.offsets_entire_region); 
          entries_regions.push_back(h.entries_entire_region);
            

          ghost_partitions.push_back(h.offsets_ghost_lp);
          ghost_entries_partitions.push_back(h.entries_ghost_lp);
          
 //         color_partitions.push_back(h.offsets_color_lp);
//          color_entries_partitions.push_back(h.entries_color_lp);

          fids.push_back(h.fid);

          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          local_args.sparse = true;
          local_args.reserve = h.reserve;
          local_args.max_entries_per_index = h.max_entries_per_index;
          args.push_back(local_args);

          *(h.ghost_is_readable) = true;

      } // !ghost_is_readable
    } // read_phase

    if (write_phase && (*h.ghost_is_readable)) {

      *(h.ghost_is_readable) = false;
      *(h.write_phase_started) = true;
    } // if
  }

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  void handle(ragged_accessor<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS> & a) {
    handle(reinterpret_cast<sparse_accessor<T, EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS, GHOST_PERMISSIONS> &>(a));
  } // handle

  template<
    typename T
  >
  void
  handle(
    sparse_mutator<
    T
    > &m
  )
  {
    if(!sparse){
      return;
    }
 
    auto & h = m.h_;

    using sparse_field_data_t = context_t::sparse_field_data_t;

    auto & flecsi_context = context_t::instance();
    const int my_color = runtime->find_local_MPI_rank();

    //read 
    if (!*(h.ghost_is_readable)) {
          clog_tag_guard(prolog);
          clog(trace) << "rank " << my_color << " READ PHASE PROLOGUE"
                      << std::endl;

          // offsets
          ghost_owners_partitions.push_back(h.ghost_owners_offsets_lp);
//          owner_subregion_partitions.push_back(
//                      h.ghost_owners_offsets_subregion_lp);
          if ( h.ghost_owners_entries_lp==Legion::LogicalPartition::NO_PART)
             h.ghost_owners_entries_lp=
              create_ghost_owners_partition_for_sparse_entries(h.index_space,
								h.fid);
          ghost_owner_entries_partitions.push_back(
            h.ghost_owners_entries_lp);
         
          entire_regions.push_back(h.offsets_entire_region);
          entries_regions.push_back(h.entries_entire_region);


          ghost_partitions.push_back(h.offsets_ghost_lp);
          ghost_entries_partitions.push_back(h.entries_ghost_lp);

 //         color_partitions.push_back(h.offsets_color_lp);
//          color_entries_partitions.push_back(h.entries_color_lp);

          fids.push_back(h.fid);

          ghost_copy_args local_args;
          local_args.data_client_hash = h.data_client_hash;
          local_args.index_space = h.index_space;
          local_args.sparse = true;
          local_args.reserve = h.reserve;
          local_args.max_entries_per_index = h.max_entries_per_index();
          args.push_back(local_args);

          *(h.ghost_is_readable) = true;
    } 

      //write
      if(*(h.ghost_is_readable) ){
        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
       }
  }

  template<typename T>
  void handle(ragged_mutator<T> & m) {
    handle(reinterpret_cast<sparse_mutator<T> &>(m));
  }

  /*!
    Don't do anything with flecsi task argument that are not data handles.
   */

  template<typename T>
  static
    typename std::enable_if_t<!std::is_base_of<dense_accessor_base_t, T>::value>
    handle(T &) {} // handle

  // member variables
  Legion::Runtime * runtime;
  Legion::Context & context;
  Legion::Domain &color_domain;
  std::vector<Legion::LogicalPartition> ghost_owners_partitions;
  std::vector<Legion::LogicalPartition> ghost_partitions;
  std::vector<Legion::LogicalRegion> entire_regions;
  std::vector<Legion::LogicalRegion> entries_regions;
//  std::vector<Legion::LogicalPartition> owner_subregion_partitions;
  std::vector<Legion::LogicalPartition> ghost_owner_entries_partitions;
  std::vector<Legion::LogicalPartition> ghost_entries_partitions;
  std::vector<Legion::LogicalPartition> entries_partitions;
  //Legion::TaskLauncher & launcher;

  std::vector<Legion::FieldID> fids;
  struct ghost_copy_args {
    size_t data_client_hash;
    size_t index_space;
    bool sparse = false;
    size_t reserve;
    size_t max_entries_per_index;
  };

  std::vector<struct ghost_copy_args> args;
  std::vector<Legion::Future> futures;
  size_t reserve;
  size_t max_entries_per_index;
  bool sparse = false;

}; // struct task_prolog_t

} // namespace execution
} // namespace flecsi
