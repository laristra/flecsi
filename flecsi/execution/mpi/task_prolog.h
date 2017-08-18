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
#include "flecsi/coloring/mpi_utils.h"

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
      // TODO: skip internal data handle
      auto& context = context_t::instance();
      const int my_color = context.color();
      auto &my_coloring_info =
        context.coloring_info(h.index_space).at(my_color);
      
      std::vector<int> peers;
      std::set_union(my_coloring_info.shared_users.begin(),
                     my_coloring_info.shared_users.end(),
                     my_coloring_info.ghost_owners.begin(),
                     my_coloring_info.ghost_owners.end(),
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
      // FIXME: the group for MPI_Win_post are the "origin" processes, i.e.
      // the peer processes calling MPI_Get to get our shared cells. Thus
      // granting access of local window to these processes. This is the set
      // union of the entry_info.shared of shared cells.
      // On the other hand, the group for MPI_Win_start are the 'target'
      // processes, i.e. the peer processes this rank is going to get ghost
      // cells from. This is the union of entry_info.rank of ghost cells.
      MPI_Win_post(rma_group, 0, win);
      MPI_Win_start(rma_group, 0, win);


      std::vector<T> buffer(my_coloring_info.ghost);
      auto index_coloring = context.coloring(h.index_space);

      // FIXME: What do we do when T is a user defined data type?
      int i = 0;
      for (auto ghost : index_coloring.ghost) {
        clog_rank(warn, 1) << "ghost id: " <<  ghost.id << ", rank: " << ghost.rank
                            << ", offset: " << ghost.offset
                            << std::endl;
        MPI_Get(h.ghost_data+i, 1, MPI_UNSIGNED_LONG_LONG,
                ghost.rank, ghost.offset,
                1, flecsi::coloring::mpi_typetraits__<T>::type(), win);
        i++;
      }

      MPI_Win_complete(win);
      MPI_Win_wait(win);

      MPI_Group_free(&rma_group);
      MPI_Win_free(&win);

      for (int i = 0; i < h.ghost_size; i++) {
        clog_rank(warn, 1) << "ghost data: " << h.ghost_data[i] << std::endl;
      }

    } // handle

    template<
      typename T,
      size_t PERMISSIONS
    >
    void
    handle(
      data_client_handle__<T, PERMISSIONS> & h
    )
    {
      auto& context_ = context_t::instance();

      auto storage = h.set_storage(new typename T::storage_t);

      bool _read{ PERMISSIONS == ro || PERMISSIONS == rw };

      int color = context_.color();

      for(size_t i{0}; i<h.num_handle_entities; ++i) {
        data_client_handle_entity_t & ent = h.handle_entities[i];

        const size_t index_space = ent.index_space;
        const size_t dim = ent.dim;
        const size_t domain = ent.domain;

        // get color_info for this field.
        auto& color_info = (context_.coloring_info(index_space)).at(color);
        ent.num_exclusive = color_info.exclusive;
        ent.num_shared = color_info.shared;
        ent.num_ghost = color_info.ghost;

        auto num_entities = ent.num_exclusive + ent.num_shared + ent.num_ghost;

        // see if the field data is registered for this entity field.
        auto& registered_field_data = context_.registered_field_data();
        auto fieldDataIter = registered_field_data.find(ent.fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = ent.size * num_entities;

          execution::context_t::instance().register_field_data(ent.fid,
                                                               size);
        }
        auto ents = reinterpret_cast<topology::mesh_entity_base_*>(registered_field_data[ent.fid].data());

        fieldDataIter = registered_field_data.find(ent.id_fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = ent.size * num_entities;

          execution::context_t::instance().register_field_data(ent.id_fid,
                                                               size);
        }
        auto ids = reinterpret_cast<utils::id_t *>(registered_field_data[ent.id_fid].data());

        // new allocation every time.
        storage->init_entities(ent.domain, ent.dim,
                               ents, ids, ent.size,
                               num_entities, ent.num_exclusive,
                               ent.num_shared, ent.num_ghost,
                               _read);
      } // for

      for(size_t i{0}; i<h.num_handle_adjacencies; ++i) {
        data_client_handle_adjacency_t &adj = h.handle_adjacencies[i];

        const size_t adj_index_space = adj.adj_index_space;
        const size_t from_index_space = adj.from_index_space;
        const size_t to_index_space = adj.to_index_space;

        // 1. get number of indices, it is the total number of entities in the
        // index space times the "degree", TODO: here I hardcoded it to 4.
        // get color_info for this field.
        auto& color_info = (context_.coloring_info(from_index_space)).at(color);
        adj.num_indices = 4*(color_info.exclusive + color_info.shared + color_info.ghost);

        // see if the field data is registered for this entity field.
        auto& registered_field_data = context_.registered_field_data();
        auto fieldDataIter = registered_field_data.find(adj.index_fid);
        if (fieldDataIter == registered_field_data.end()) {
          size_t size = sizeof(utils::id_t) * adj.num_indices;
          execution::context_t::instance().register_field_data(adj.index_fid,
                                                               size);
        }
        adj.indices_buf = reinterpret_cast<size_t *>(registered_field_data[adj.index_fid].data());
        clog(trace) << "num_indices: " << adj.num_indices << std::endl;

        // 2. get number of offset, it should be number of cells?
        adj.num_offsets = (color_info.exclusive + color_info.shared + color_info.ghost);
        // 3. allocate field data for index_buf and offset_buf.
        fieldDataIter = registered_field_data.find(adj.offset_fid);
        if (fieldDataIter == registered_field_data.end()) {
          // TODO: what is the element type?
          size_t size = sizeof(size_t) * adj.num_offsets;

          execution::context_t::instance().register_field_data(adj.offset_fid,
                                                               size);
        }
        adj.offsets_buf = reinterpret_cast<size_t *>(registered_field_data[adj.offset_fid].data());
        clog(trace) << "num_offsets: " << adj.num_offsets << std::endl;

        storage->init_connectivity(adj.from_domain, adj.to_domain,
                                   adj.from_dim, adj.to_dim,
                                   reinterpret_cast<utils::offset_t *>(adj.offsets_buf), adj.num_offsets,
                                   reinterpret_cast<utils::id_t *>(adj.indices_buf), adj.num_indices,
                                   _read);
      }

      if(!_read){
        h.initialize_storage();
      }
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
