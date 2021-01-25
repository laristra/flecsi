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

#include <hpx/include/async.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>

#include <cinchlog.h>
#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_HPX)
#error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <mpi.h>

#include <flecsi/coloring/coloring_types.h>
#include <flecsi/coloring/index_coloring.h>
#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/row_vector.h>
#include <flecsi/data/common/serdez.h>
#include <flecsi/data/sparse_data_handle.h>
#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/hpx/future.h>
#include <flecsi/execution/hpx/runtime_driver.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/export_definitions.h>
#include <flecsi/utils/mpi_type_traits.h>

#include <flecsi/execution/hpx/future.h>
#include <flecsi/utils/const_string.h>

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
/*!
  The hpx_context_policy_t is the backend runtime context policy for HPX.

 @ingroup mpi-execution
 */

struct hpx_context_policy_t {

  /*!
    The registration_function_t type defines a function type for
    registration callbacks.
   */

  using registration_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;

  using task_info_t = std::tuple<task_id_t,
    processor_type_t,
    launch_t,
    std::string,
    registration_function_t>;

  struct sparse_field_data_t {

    sparse_field_data_t() {}

    sparse_field_data_t(size_t type_size,
      size_t num_exclusive,
      size_t num_shared,
      size_t num_ghost,
      size_t max_entries_per_index)
      : type_size(type_size), num_exclusive(num_exclusive),
        num_shared(num_shared), num_ghost(num_ghost),
        num_total(num_exclusive + num_shared + num_ghost),
        max_entries_per_index(max_entries_per_index),
        rows(num_total * sizeof(data::row_vector_u<uint8_t>)) {}

    std::ostream & write(std::ostream & os,
      const data::serdez_untyped_t * serdez) const {

      os.write((char *)&type_size, sizeof(size_t));
      os.write((char *)&num_exclusive, sizeof(size_t));
      os.write((char *)&num_shared, sizeof(size_t));
      os.write((char *)&num_ghost, sizeof(size_t));
      os.write((char *)&num_total, sizeof(size_t));
      os.write((char *)&max_entries_per_index, sizeof(size_t));

      // use serdez operator to write actual row data
      const char * row_ptr = (char *)rows.data();
      for(int i = 0; i < num_total; ++i) {
        serdez->serialize(row_ptr, os);
        row_ptr += sizeof(data::row_vector_u<uint8_t>);
      }
      return os;
    }

    std::istream & read(std::istream & is,
      const data::serdez_untyped_t * serdez) {
      is.read((char *)&type_size, sizeof(size_t));
      is.read((char *)&num_exclusive, sizeof(size_t));
      is.read((char *)&num_shared, sizeof(size_t));
      is.read((char *)&num_ghost, sizeof(size_t));
      is.read((char *)&num_total, sizeof(size_t));
      is.read((char *)&max_entries_per_index, sizeof(size_t));

      // use serdez operator to read actual row data
      char * row_ptr = (char *)rows.data();
      for(int i = 0; i < num_total; ++i) {
        serdez->deserialize(row_ptr, is);
        row_ptr += sizeof(data::row_vector_u<uint8_t>);
      }
      return is;
    }

    size_t type_size;

    // total # of exclusive, shared, ghost entries
    size_t num_exclusive = 0;
    size_t num_shared = 0;
    size_t num_ghost = 0;
    size_t num_total = 0;

    size_t max_entries_per_index;

    std::vector<uint8_t> rows;
  }; // sparse_field_data_t

  using field_id_t = size_t;

  FLECSI_EXPORT hpx_context_policy_t();

  /*!
   FleCSI context initialization. This method initializes the FleCSI
   runtime using MPI.

   @param argc The command-line argument count passed from main.
   @param argv The command-line argument values passed from main.

   @return An integer value with a non-zero error code upon failure,
           zero otherwise.
   */
  int initialize(int argc, char * argv[]) {
    // start HPX runtime system, execute driver code in the context of HPX
    return start_hpx(&hpx_runtime_driver, argc, argv);
  } // hpx_context_policy_t::initialize

  /*!
    Return the color for which the context was initialized.
   */
  FLECSI_EXPORT size_t color() const;

  /*!
    Return the number of colors.
   */

  FLECSI_EXPORT std::size_t colors() const;

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    The unique_tid_t type create a unique id generator for registering
    tasks.
   */

  using unique_tid_t = utils::unique_id_t<task_id_t>;

  /*!
   Register a task with the runtime.

   @param key       The task hash key.
   @param name      The task name string.
   @param call_back The registration call back function.
   */

  //------------------------------------------------------------------------//
  // Function interface.
  //------------------------------------------------------------------------//

  struct index_space_data_t {
    std::map<field_id_t, bool> ghost_is_readable;
    std::map<field_id_t, execution::hpx_future_u<void>> future;
  };

  struct index_subspace_data_t {
    size_t capacity;
  };

  auto & index_space_data_map() {
    return index_space_data_map_;
  }

  /*!
    Get the index subspace data map.
   */

  auto & index_subspace_data_map() {
    return index_subspace_data_map_;
  }

  using coloring_info_t = flecsi::coloring::coloring_info_t;
  using index_coloring_t = flecsi::coloring::index_coloring_t;

  /*!
   Field metadata is used maintain MPI information and data types for
   MPI windows/one-sided communication to perform ghost copies.
   */
  struct field_metadata_t {

    MPI_Group comm_grp = MPI_GROUP_NULL;
    MPI_Group shared_users_grp = MPI_GROUP_NULL;
    MPI_Group ghost_owners_grp = MPI_GROUP_NULL;

    std::map<int, MPI_Datatype> origin_types;
    std::map<int, MPI_Datatype> target_types;

    MPI_Datatype data_type;

    MPI_Win win = MPI_WIN_NULL;

#if defined(FLECSI_USE_AGGCOMM)
    std::vector<std::vector<std::array<size_t, 2>>> shared_indices;
    std::vector<std::vector<std::array<size_t, 2>>> ghost_indices;
    std::vector<size_t> shared_field_sizes;
    std::vector<size_t> ghost_field_sizes;
    unsigned char * shared_data_buffer;
    unsigned char * ghost_data_buffer;
#endif
  };

  /*!
   Field metadata is used maintain MPI information and data types for
   MPI windows/one-sided communication to perform ghost copies.
   */
  struct sparse_field_metadata_t {
    MPI_Group comm_grp = MPI_GROUP_NULL;
    MPI_Group shared_users_grp = MPI_GROUP_NULL;
    MPI_Group ghost_owners_grp = MPI_GROUP_NULL;

    std::map<int, std::vector<int>> compact_origin_lengs;
    std::map<int, std::vector<int>> compact_origin_disps;

    std::map<int, std::vector<int>> compact_target_lengs;
    std::map<int, std::vector<int>> compact_target_disps;

    std::map<int, MPI_Datatype> origin_types;
    std::map<int, MPI_Datatype> target_types;

#if defined(FLECSI_USE_AGGCOMM)
    std::map<int, std::vector<size_t>> shared_indices;
    std::map<int, std::vector<size_t>> ghost_indices;
    std::vector<uint32_t> ghost_row_sizes;
#endif

    MPI_Win win = MPI_WIN_NULL;

    std::function<void(void)> deleter;
  };

  /*!
   Create MPI datatypes use for ghost copy by inspecting shared regions,
   and ghost owners, to compute origin and target lengths and displacements
   for MPI windows.
   */
  template<typename T>
  void register_field_metadata(const field_id_t fid,
    const coloring_info_t & coloring_info,
    const index_coloring_t & index_coloring) {
#if !defined(FLECSI_USE_AGGCOMM)
    std::map<int, std::vector<int>> compact_origin_lengs;
    std::map<int, std::vector<int>> compact_origin_disps;

    std::map<int, std::vector<int>> compact_target_lengs;
    std::map<int, std::vector<int>> compact_target_disps;

    field_metadata_t metadata;

    register_field_metadata_<T>(metadata, fid, coloring_info, index_coloring,
      compact_origin_lengs, compact_origin_disps, compact_target_lengs,
      compact_target_disps);

    MPI_Type_contiguous(
      static_cast<int>(sizeof(T)), MPI_BYTE, &metadata.data_type);
    MPI_Type_commit(&metadata.data_type);

    for(auto owner : coloring_info.ghost_owners) {
      int ghost_owner = static_cast<int>(owner);
      MPI_Datatype origin_type;
      MPI_Datatype target_type;

      MPI_Type_indexed(
        static_cast<int>(compact_origin_lengs[ghost_owner].size()),
        compact_origin_lengs[ghost_owner].data(),
        compact_origin_disps[ghost_owner].data(), metadata.data_type,
        &origin_type);
      MPI_Type_commit(&origin_type);
      metadata.origin_types.insert({ghost_owner, origin_type});

      MPI_Type_indexed(
        static_cast<int>(compact_target_lengs[ghost_owner].size()),
        compact_target_lengs[ghost_owner].data(),
        compact_target_disps[ghost_owner].data(), metadata.data_type,
        &target_type);
      MPI_Type_commit(&target_type);
      metadata.target_types.insert({ghost_owner, target_type});
    }

    auto data = field_data[fid].data();
    auto shared_data = data + coloring_info.exclusive * sizeof(T);
    MPI_Win_create(shared_data, coloring_info.shared * sizeof(T),
      static_cast<int>(sizeof(T)), MPI_INFO_NULL, MPI_COMM_WORLD,
      &metadata.win);

    field_metadata.insert({fid, metadata});
#else
    field_metadata_t metadata;
    int mpiSize;
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

    // FIXME: Do this per index_space instead of per field id

    metadata.shared_indices.resize(mpiSize);
    metadata.ghost_indices.resize(mpiSize);
    metadata.ghost_field_sizes.resize(mpiSize);
    metadata.shared_field_sizes.resize(mpiSize);

    // indices are stored as vectors of pairs, each pair consisting of: starting
    // index, how many consecutive indices

    size_t ghost_cnt = 0;

    for(auto const & ghost : index_coloring.ghost) {

      if(metadata.ghost_indices[ghost.rank].size() == 0 ||
         ghost_cnt * sizeof(T) !=
           (metadata.ghost_indices[ghost.rank].back()[0] +
             metadata.ghost_indices[ghost.rank].back()[1]))
        metadata.ghost_indices[ghost.rank].push_back(
          {ghost_cnt * sizeof(T), sizeof(T)});
      else
        metadata.ghost_indices[ghost.rank].back()[1] += sizeof(T);
      ++ghost_cnt;
    }

    for(auto const & shared : index_coloring.shared) {

      for(auto const & s : shared.shared) {

        if(metadata.shared_indices[s].size() == 0 ||
           shared.offset * sizeof(T) != (metadata.shared_indices[s].back()[0] +
                                          metadata.shared_indices[s].back()[1]))

          metadata.shared_indices[s].push_back(
            {shared.offset * sizeof(T), sizeof(T)});

        else

          metadata.shared_indices[s].back()[1] += sizeof(T);
      }
    }

    for(int rank = 0; rank < mpiSize; ++rank) {
      for(auto const & ind : metadata.ghost_indices[rank])
        metadata.ghost_field_sizes[rank] += ind[1];
      for(auto const & ind : metadata.shared_indices[rank])
        metadata.shared_field_sizes[rank] += ind[1];
    }

    field_metadata.insert({fid, metadata});
#endif
  }

  /*!
   Create MPI datatypes use for ghost copy by inspecting shared regions,
   and ghost owners, to compute origin and target lengths and displacements
   for MPI windows.
   */
  template<typename T>
  void register_sparse_field_metadata(const field_id_t fid,
    const coloring_info_t & coloring_info,
    const index_coloring_t & index_coloring) {
    sparse_field_metadata_t metadata;
#if !defined(FLECSI_USE_AGGCOMM)

    register_field_metadata_<T>(metadata, fid, coloring_info, index_coloring,
      metadata.compact_origin_lengs, metadata.compact_origin_disps,
      metadata.compact_target_lengs, metadata.compact_target_disps);

#else
    // compute ghost and shared indicies
    size_t ghost_count = 0;
    for(auto const & ghost : index_coloring.ghost) {
      metadata.ghost_indices[static_cast<int>(ghost.rank)].push_back(
        ghost_count);
      ++ghost_count;
    }
    for(auto const & shared : index_coloring.shared) {
      for(auto const & s : shared.shared) {
        metadata.shared_indices[static_cast<int>(s)].push_back(shared.offset);
      }
    }

    // allocate ghost_row_sizes
    metadata.ghost_row_sizes.resize(index_coloring.ghost.size());
#endif

    auto it = sparse_field_data.find(fid);
    auto rows = &it->second.rows[0];
    auto num_total = &it->second.num_total;
    metadata.deleter = [=]() {
      using vector_t = typename ragged_data_handle_u<T>::vector_t;
      auto vec = reinterpret_cast<vector_t *>(rows);
      for(size_t i = 0; i < *num_total; ++i)
        vec[i].clear();
    };

    sparse_field_metadata.insert({fid, metadata});
  }

  /*!
   Compute MPI datatypes, compacted length and displacement for ghost copy
   with MPI window.
   */
  template<typename T, typename MD>
  void register_field_metadata_(MD & metadata,
    const field_id_t fid,
    const coloring_info_t & coloring_info,
    const index_coloring_t & index_coloring,
    std::map<int, std::vector<int>> & compact_origin_lengs,
    std::map<int, std::vector<int>> & compact_origin_disps,
    std::map<int, std::vector<int>> & compact_target_lengs,
    std::map<int, std::vector<int>> & compact_target_disps) {
    // The group for MPI_Win_post are the "origin" processes, i.e.
    // the peer processes calling MPI_Get to get our shared cells. Thus
    // granting access of local window to these processes. This is the set
    // coloring_info_t::shared_users
    // On the other hand, the group for MPI_Win_start are the 'target'
    // processes, i.e. the peer processes this rank is going to get ghost
    // cells from. This is the set coloring_info_t::ghost_owners.
    // Since both shared_users and ghost_owners are std::set, we have copy
    // them to std::vector be passed to MPI.

    // initialize explicitly to avoid integral conversion warnings
    std::vector<int> shared_users;
    shared_users.reserve(coloring_info.shared_users.size());
    for(auto const v : coloring_info.shared_users)
      shared_users.push_back(static_cast<int>(v));

    std::vector<int> ghost_owners;
    ghost_owners.reserve(coloring_info.ghost_owners.size());
    for(auto const v : coloring_info.ghost_owners)
      ghost_owners.push_back(static_cast<int>(v));

    if(metadata.comm_grp == MPI_GROUP_NULL) {
      MPI_Comm_group(MPI_COMM_WORLD, &metadata.comm_grp);

      MPI_Group_incl(metadata.comm_grp, static_cast<int>(shared_users.size()),
        shared_users.data(), &metadata.shared_users_grp);
      MPI_Group_incl(metadata.comm_grp, static_cast<int>(ghost_owners.size()),
        ghost_owners.data(), &metadata.ghost_owners_grp);
    }

    std::map<int, std::vector<int>> origin_lens;
    std::map<int, std::vector<int>> origin_disps;
    std::map<int, std::vector<int>> target_lens;
    std::map<int, std::vector<int>> target_disps;

    for(auto ghost_owner : ghost_owners) {
      origin_lens.insert({ghost_owner, {}});
      origin_disps.insert({ghost_owner, {}});
      target_lens.insert({ghost_owner, {}});
      target_disps.insert({ghost_owner, {}});
    }

    int origin_index = 0;
    for(const auto & ghost : index_coloring.ghost) {
      int rank = static_cast<int>(ghost.rank);
      origin_lens[rank].push_back(1);
      origin_disps[rank].push_back(origin_index++);
      target_lens[rank].push_back(1);
      target_disps[rank].push_back(static_cast<int>(ghost.offset));
    }

    int my_color;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_color);

// This should only be uncommented for debugging (outputs info
// during tutorial runs). Consider changing this to use clog
#if 0
    if (my_color == 0) {
      for (auto ghost_owner : ghost_owners) {
        std::cout << "ghost owner: " << ghost_owner << std::endl;
        std::cout << "\torigin length: ";
        for(auto len : origin_lens[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;
        std::cout << "\torigin disp: ";
        for(auto len : origin_disps[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;
        std::cout << "\ttarget length: ";
        for(auto len : target_lens[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;

        std::cout << "\ttarget disp: ";
        for(auto len : target_disps[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;
      } // for
    } // if
#endif

    for(auto owner : ghost_owners) {
      int ghost_owner = static_cast<int>(owner);
      if(origin_disps.size() == 0)
        break;

      int count = 0;
      compact_origin_lengs[ghost_owner].push_back(1);
      compact_origin_disps[ghost_owner].push_back(origin_disps[ghost_owner][0]);

      for(int i = 1; i < origin_disps[ghost_owner].size(); i++) {
        if(origin_disps[ghost_owner][i] - origin_disps[ghost_owner][i - 1] ==
           1) {
          compact_origin_lengs[ghost_owner].back() =
            compact_origin_lengs[ghost_owner].back() + 1;
        }
        else {
          compact_origin_lengs[ghost_owner].push_back(1);
          compact_origin_disps[ghost_owner].push_back(
            origin_disps[ghost_owner][i]);
        }
      }
    }

#if 0
    if (my_color == 0) {
      for (auto ghost_owner : ghost_owners) {
        std::cout << "ghost owner: " << ghost_owner << std::endl;
        std::cout << "source compacted length: ";
        for(auto len : compact_origin_lengs[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;
        std::cout << "source compacted disps: ";
        for(auto disp : compact_origin_disps[ghost_owner]) {
          std::cout << disp << " ";
        }
        std::cout << std::endl;
      } // for
    } // if
#endif

    for(auto owner : ghost_owners) {
      int ghost_owner = static_cast<int>(owner);
      if(target_disps.size() == 0)
        break;

      int count = 0;
      compact_target_lengs[ghost_owner].push_back(1);
      compact_target_disps[ghost_owner].push_back(target_disps[ghost_owner][0]);

      for(int i = 1; i < target_disps[ghost_owner].size(); i++) {
        if(target_disps[ghost_owner][i] - target_disps[ghost_owner][i - 1] ==
           1) {
          compact_target_lengs[ghost_owner].back() =
            compact_target_lengs[ghost_owner].back() + 1;
        }
        else {
          compact_target_lengs[ghost_owner].push_back(1);
          compact_target_disps[ghost_owner].push_back(
            target_disps[ghost_owner][i]);
        }
      }
    }

#if 0
    if (my_color == 0) {
      for (auto ghost_owner : ghost_owners) {
        std::cout << "ghost owner: " << ghost_owner << std::endl;

        std::cout << "compacted target length: ";
        for(auto len : compact_target_lengs[ghost_owner]) {
          std::cout << len << " ";
        }
        std::cout << std::endl;
        std::cout << "compacted target disps: ";
        for(auto disp : compact_target_disps[ghost_owner]) {
          std::cout << disp << " ";
        }
        std::cout << std::endl;
      } // for
    } // if
#endif
  } // register_field_metadata_

  std::map<field_id_t, field_metadata_t> & registered_field_metadata() {
    return field_metadata;
  };

  /*!
   Register new field data, i.e. allocate a new buffer for the specified field
   ID.
   */
  void register_field_data(field_id_t fid, size_t size) {
    // TODO: VERSIONS
    auto it = field_data.find(fid);
    if(it == field_data.end()) {
      field_data.insert({fid, std::vector<uint8_t>(size)});
      field_futures.insert({fid, execution::hpx_future_u<void>{}});
    }
    else {
      it->second.resize(size);
    }
  }

  std::map<field_id_t, std::vector<uint8_t>> & registered_field_data() {
    return field_data;
  }

  std::map<field_id_t, execution::hpx_future_u<void>> &
  registered_field_futures() {
    return field_futures;
  }

  /*!
   Register new sparse field data, i.e. allocate a new buffer for the
   specified field ID. Sparse data consists of a buffer of offsets
   (start + length) and entry id / value pairs and associated metadata about
   this field.
   */

  void register_sparse_field_data(field_id_t fid,
    size_t type_size,
    const coloring_info_t & coloring_info,
    size_t max_entries_per_index) {
    // TODO: VERSIONS
    sparse_field_data_t new_field(type_size, coloring_info.exclusive,
      coloring_info.shared, coloring_info.ghost, max_entries_per_index);
    auto it = sparse_field_data.find(fid);
    if(it == sparse_field_data.end()) {
      sparse_field_data.emplace(fid, std::move(new_field));
    }
    else {
      it->second = std::move(new_field);
    }
  }

  std::map<field_id_t, sparse_field_data_t> & registered_sparse_field_data() {
    return sparse_field_data;
  }

  std::map<field_id_t, sparse_field_metadata_t> &
  registered_sparse_field_metadata() {
    return sparse_field_metadata;
  };

  std::map<size_t, MPI_Op> & reduction_operations() {
    return reduction_ops_;
  } // reduction_types

  void finalize() {
#if !defined(FLECSI_USE_AGGCOMM)
    for(auto & md : field_metadata) {
      for(auto & ty : md.second.origin_types)
        MPI_Type_free(&ty.second);
      for(auto & ty : md.second.target_types)
        MPI_Type_free(&ty.second);
      MPI_Type_free(&md.second.data_type);
      MPI_Group_free(&md.second.ghost_owners_grp);
      MPI_Group_free(&md.second.shared_users_grp);
      MPI_Group_free(&md.second.comm_grp);
      if(md.second.win != MPI_WIN_NULL)
        MPI_Win_free(&md.second.win);
    }
#endif
    for(auto & md : sparse_field_metadata) {
#if !defined(FLECSI_USE_AGGCOMM)
      for(auto & ty : md.second.origin_types)
        MPI_Type_free(&ty.second);
      for(auto & ty : md.second.target_types)
        MPI_Type_free(&ty.second);
      MPI_Group_free(&md.second.ghost_owners_grp);
      MPI_Group_free(&md.second.shared_users_grp);
      MPI_Group_free(&md.second.comm_grp);
      if(md.second.win != MPI_WIN_NULL)
        MPI_Win_free(&md.second.win);
#endif
      md.second.deleter();
    }
  }

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    Register a task with the runtime.

    @param key       The task hash key.
    @param name      The task name string.
    @param callback The registration call back function.
   */

  bool register_task(size_t key,
    processor_type_t processor,
    launch_t launch,
    std::string & name,
    const registration_function_t & callback) {
    clog(info) << "Registering task callback " << name << " with key " << key
               << std::endl;

    clog_assert(task_registry_.find(key) == task_registry_.end(),
      "task key already exists");

    task_registry_[key] = std::make_tuple(
      unique_tid_t::instance().next(), processor, launch, name, callback);

    return true;
  } // register_task

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  template<size_t KEY>
  task_info_t & task_info() {
    auto task_entry = task_registry_.find(KEY);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << KEY << " does not exist");

    return task_entry->second;
  } // task_info

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  task_info_t & task_info(size_t key) {
    auto task_entry = task_registry_.find(key);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << key << " does not exist");

    return task_entry->second;
  } // task_info

#define task_info_template_method(name, return_type, index)                    \
  template<size_t KEY>                                                         \
  return_type name() {                                                         \
    {                                                                          \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << KEY << std::endl;      \
    }                                                                          \
    return std::get<index>(task_info<KEY>());                                  \
  }

  /*!
    FIXME

    @param key The task hash key.
   */

#define task_info_method(name, return_type, index)                             \
  return_type name(size_t key) {                                               \
    {                                                                          \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << key << std::endl;      \
    }                                                                          \
    return std::get<index>(task_info(key));                                    \
  }

  /*!
    FIXME

    @param key The task hash key.
   */

  task_info_template_method(task_id, task_id_t, 0);
  task_info_method(task_id, task_id_t, 0);
  task_info_template_method(processor_type, processor_type_t, 1);
  task_info_method(processor_type, processor_type_t, 1);

protected:
  // Helper function for HPX start-up and shutdown
  FLECSI_EXPORT int
  hpx_main(void (*driver)(int, char *[]), int argc, char * argv[]);

  // Start the HPX runtime system,
  FLECSI_EXPORT int
  start_hpx(void (*driver)(int, char *[]), int argc, char * argv[]);

public:
  int rank;

  // private:
  int color_ = 0;
  int colors_ = 0;

  std::map<field_id_t, std::vector<uint8_t>> field_data;
  std::map<field_id_t, field_metadata_t> field_metadata;
  std::map<field_id_t, execution::hpx_future_u<void>> field_futures;

  std::map<size_t, index_space_data_t> index_space_data_map_;
  std::map<size_t, index_subspace_data_t> index_subspace_data_map_;

  std::map<field_id_t, sparse_field_data_t> sparse_field_data;
  std::map<field_id_t, sparse_field_metadata_t> sparse_field_metadata;

  std::map<size_t, MPI_Op> reduction_ops_;

  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
  std::map<size_t, task_info_t> task_registry_;
}; // struct hpx_context_policy_t

} // namespace execution
} // namespace flecsi
