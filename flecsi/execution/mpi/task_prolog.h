/*~--------------------------------------------------------------------------~*
*  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
* /@@/////  /@@          @@////@@ @@////// /@@
* /@@       /@@  @@@@@  @@    // /@@       /@@
* /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
* /@@////   /@@/@@@@@@@/@@       ////////@@/@@
* /@@       /@@/@@//// //@@    @@       /@@/@@
* /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
* //       ///  //////   //////  ////////  //
*
* Copyright (c) 2016 Los Alamos National Laboratory, LLC
* All rights reserved
*~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpi_task_prolog_h
#define flecsi_execution_mpi_task_prolog_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 19, 2017
//----------------------------------------------------------------------------//

#include <vector>

#include "mpi.h"
#include "flecsi/data/data.h"
#include "flecsi/execution/context.h"

namespace flecsi {
namespace execution {

  //--------------------------------------------------------------------------//
  //! The task_prolog_t type can be called to walk the task args after the
  //! task launcher is created, but before the task has run. This allows
  //! synchronization dependencies to be added to the execution flow.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  struct task_prolog_t : public utils::tuple_walker__<task_prolog_t>
  {

    //------------------------------------------------------------------------//
    //! Construct a task_prolog_t instance.
    //!
    //------------------------------------------------------------------------//

    task_prolog_t() = default;


    //------------------------------------------------------------------------//
    //! FIXME: Need a description.
    //!
    //! @tparam T                     The data type referenced by the handle.
    //! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
    //!                               indices of the index partition.
    //! @tparam SHARED_PERMISSIONS    The permissions required on the shared
    //!                               indices of the index partition.
    //! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
    //!                               indices of the index partition.
    //!
    //! @param runtime The Legion task runtime.
    //! @param context The Legion task runtime context.
    //------------------------------------------------------------------------//

    template<
      typename T,
      size_t EXCLUSIVE_PERMISSIONS,
      size_t SHARED_PERMISSIONS,
      size_t GHOST_PERMISSIONS
    >
    void
    handle(
      data_handle__<
        T,
        EXCLUSIVE_PERMISSIONS,
        SHARED_PERMISSIONS,
        GHOST_PERMISSIONS
      > & h
    )
    {
      auto& flecsi_context = context_t::instance();

      const int my_color = flecsi_context.rank;
      auto &coloring_info = flecsi_context.coloring_info(h.index_space);

      auto &my_coloring_info = flecsi_context.coloring_info(h.index_space).at(my_color);

      std::vector<int> peers;
      std::set_union(my_coloring_info.shared_users.begin(), my_coloring_info.shared_users.end(),
                     my_coloring_info.ghost_owners.begin(), my_coloring_info.ghost_owners.end(),
                     std::back_inserter(peers));

      MPI_Group comm_grp, rma_group;
      MPI_Comm_group(MPI_COMM_WORLD, &comm_grp);
      MPI_Group_incl(comm_grp, peers.size(), peers.data(), &rma_group);
      MPI_Group_free(&comm_grp);

      // A pull model using MPI_Get:
      // 1. create MPI window for shared portion of the local buffer.
      MPI_Win win;
      MPI_Win_create(h.shared_data, my_coloring_info.shared * sizeof(T),
                     sizeof(T), MPI_INFO_NULL, MPI_COMM_WORLD,
                     &win);

      // 2. iterate through each ghost cell and MPI_Get from the peer.
      // FIXME: the group for MPI_Win_post are the "origin" processes, i.e. the peer
      // processes calling MPI_Get to get our shared cells. Thus granting access of
      // local window to these processes. This is the set union of the entry_info.shared
      // of shared cells.
      // On the other hand, the group for MPI_Win_start are the 'target' processes, i.e. the
      // peer processes this rank is going to get ghost cells from. This is the union of
      // entry_info.rank of ghost cells.
      MPI_Win_post(rma_group, 0, win);
      MPI_Win_start(rma_group, 0, win);


      std::vector<T> buffer(my_coloring_info.ghost);
      auto index_coloring = flecsi_context.coloring(h.index_space);

      int i = 0;
      for (auto ghost : index_coloring.ghost) {
        clog_rank(warn, 1) << "ghost id: " <<  ghost.id << ", rank: " << ghost.rank
                            << ", offset: " << ghost.offset
                            << std::endl;
        MPI_Get(h.ghost_data+i, 1, MPI_UNSIGNED_LONG_LONG,
                ghost.rank, ghost.offset,
                1, MPI_UNSIGNED_LONG_LONG, win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);

      MPI_Group_free(&rma_group);
      MPI_Win_free(&win);


      for (int i = 0; i < h.ghost_size; i++) {
        clog_rank(warn, 1) << "ghost data: " << h.ghost_data[i] << std::endl;
        //h.ghost_data[i] = i;
      }
#if 0
      if (GHOST_PERMISSIONS != dno)
        read_phase = true;

      if ( (SHARED_PERMISSIONS == dwd) || (SHARED_PERMISSIONS == drw) )
        write_phase = true;

      if (read_phase) {
          clog(trace) << "rank " << my_color <<
              " READ PHASE PROLOGUE" << std::endl;
        if (!*(h.ghost_is_readable)) {
          // as master
          clog(trace) << "rank " << my_color << " arrives & advances " <<
              *(h.pbarrier_as_owner_ptr) <<
              std::endl;

          h.pbarrier_as_owner_ptr->arrive(1);                     // phase WRITE
          *(h.pbarrier_as_owner_ptr) = runtime->advance_phase_barrier(context,
              *(h.pbarrier_as_owner_ptr));                          // phase WRITE

          // as slave
          for (size_t owner=0; owner<h.ghost_owners_pbarriers_ptrs.size(); owner++) {
            clog(trace) << "rank " << my_color << " WAITS " <<
                *(h.ghost_owners_pbarriers_ptrs[owner]) <<
                std::endl;

            clog(trace) << "rank " << my_color << " arrives & advances " <<
                *(h.ghost_owners_pbarriers_ptrs[owner]) <<
                std::endl;

            Legion::RegionRequirement rr_shared(h.ghost_owners_lregions[owner],
              READ_ONLY, EXCLUSIVE, h.ghost_owners_lregions[owner]);

            Legion::RegionRequirement rr_ghost(h.ghost_lr,
              WRITE_DISCARD, EXCLUSIVE, h.color_region);

            auto iitr = flecsi_context.field_info_map().find(h.index_space);
            clog_assert(iitr != flecsi_context.field_info_map().end(),
              "invalid index space");

            auto ghost_owner_pos_fid = 
              LegionRuntime::HighLevel::FieldID(
              internal_field::ghost_owner_pos);

            rr_ghost.add_field(ghost_owner_pos_fid);

            for(auto& fitr : iitr->second){
              const context_t::field_info_t& fi = fitr.second;
              rr_shared.add_field(fi.fid);
              rr_ghost.add_field(fi.fid);
            }

            // TODO - circular dependency including internal_task.h
            auto constexpr key = 
              flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(
                ghost_copy_task)}.hash();

            const auto ghost_copy_tid = flecsi_context.task_id<key>();
            
            struct {
              size_t index_space;
              size_t owner;
            } args;
            args.index_space = h.index_space;
            args.owner = owner;
            Legion::TaskLauncher
              launcher(ghost_copy_tid,
              Legion::TaskArgument(&args, sizeof(args)));

            clog(trace) << "gid to lid map size = " <<
                    h.global_to_local_color_map_ptr->size() << std::endl;
            launcher.add_future(Legion::Future::from_value(runtime,
                    *(h.global_to_local_color_map_ptr)));

            launcher.add_region_requirement(rr_shared);
            launcher.add_region_requirement(rr_ghost);
            launcher.add_wait_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));// phase READ
            launcher.add_arrival_barrier(*(h.ghost_owners_pbarriers_ptrs[owner]));// phase WRITE
            runtime->execute_task(context, launcher);

            *(h.ghost_owners_pbarriers_ptrs[owner]) = runtime->advance_phase_barrier(context,
                *(h.ghost_owners_pbarriers_ptrs[owner]));             // phase WRITE

          }  // for owner as user

          *(h.ghost_is_readable) = true;
        } // !ghost_is_readable
      } // read_phase

      if (write_phase && (*h.ghost_is_readable)) {
        clog(trace) << "rank " << runtime->find_local_MPI_rank() <<
            " WRITE PHASE PROLOGUE" << std::endl;
        clog(trace) << "rank " << my_color << " wait & arrival barrier " <<
            *(h.pbarrier_as_owner_ptr) <<
            std::endl;
        launcher.add_wait_barrier(*(h.pbarrier_as_owner_ptr));      // phase WRITE
        launcher.add_arrival_barrier(*(h.pbarrier_as_owner_ptr));   // phase READ

        *(h.ghost_is_readable) = false;
        *(h.write_phase_started) = true;
      }
#endif
    } // handle

    //------------------------------------------------------------------------//
    //! FIXME: Need to document.
    //------------------------------------------------------------------------//

    template<
      typename T
    >
    static
    typename std::enable_if_t<!std::is_base_of<data_handle_base_t, T>::value>
    handle(
      T&
    )
    {
    } // handle

  }; // struct task_prolog_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpi_task_prolog_h
