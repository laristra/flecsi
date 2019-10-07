/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class_u
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include <flecsi/data/storage_class.h>
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include <algorithm>
#include <memory>

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client.h>
#include <flecsi/data/sparse_data_handle.h>
#include <flecsi/execution/context.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace legion {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Sparse storage type.
//----------------------------------------------------------------------------//

/*!
  Sparse storage type. Sparse data is partitioned into exclusive, shared,
  ghost entries. It allows entries to be allocated sparsely per index.
  Within an index, entries are stored in sorted order. A sparse accessor
  can read and modify existing entries, but cannot allocate new entries.
  The mutator is used for this purpose. A sparse data handle is passed to a
  task which is then transformed to an accesor, likewise for a mutator.
  A mutator commits its data in its temporary buffers in the task epilog.
 */
template<>
struct storage_class_u<ragged> {
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<typename T>
  using handle_u = ragged_data_handle_u<T>;

  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static handle_u<DATA_TYPE> get_handle(const data_client_t & data_client) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    auto & context = execution::context_t::instance();

    using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

    // get field_info for this data handle
    auto & field_info = context.get_field_info_from_name(
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    size_t index_space = field_info.index_space;
    auto & ism = context.index_space_data_map();

    auto & im = context.sparse_index_space_info_map();
    auto iitr = im.find(index_space);
    clog_assert(iitr != im.end(),
      "sparse index space info not registered for index space: "
        << index_space);

    const size_t max_entries_per_index = iitr->second.max_entries_per_index;

    handle_u<DATA_TYPE> h(max_entries_per_index);

    h.offsets_entire_region = ism[index_space].entire_region;
    h.offsets_exclusive_lp = ism[index_space].exclusive_lp;
    h.offsets_shared_lp = ism[index_space].shared_lp;
    h.offsets_ghost_lp = ism[index_space].ghost_lp;

    h.metadata_entire_region = context.sparse_metadata().entire_region;
    h.metadata_lp = context.sparse_metadata().color_partition;

    //      h.pbarrier_as_owner_ptr =
    //          &ism[index_space].pbarriers_as_owner[field_info.fid];
    h.ghost_is_readable = &(ism[index_space].ghost_is_readable[field_info.fid]);
    h.write_phase_started =
      &(ism[index_space].write_phase_started[field_info.fid]);
    //      h.ghost_owners_pbarriers_ptrs.resize(0);

    //      const size_t _pb_size{
    //          ism[index_space].ghost_owners_pbarriers[field_info.fid].size()};

    //      for (size_t i = 0; i < _pb_size; i++) {
    //        h.ghost_owners_pbarriers_ptrs.push_back(
    //            &(ism[index_space].ghost_owners_pbarriers[field_info.fid][i]));
    //      } // for

    h.ghost_owners_offsets_lp = ism[index_space].ghost_owners_lp;

    h.global_to_local_color_map_ptr =
      &ism[index_space].global_to_local_color_map;

    h.fid = field_info.fid;

    h.index_space = index_space;
    h.data_client_hash = field_info.data_client_hash;

    return h;
  }

  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static handle_u<DATA_TYPE> get_mutator(const data_client_t & data_client,
    size_t) {
    return get_handle<DATA_CLIENT_TYPE, DATA_TYPE, NAMESPACE, NAME, VERSION>(
      data_client);
  }

}; // struct storage_class_t

template<>
struct storage_class_u<sparse> {
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<typename T>
  using entry_value_u = data::sparse_entry_value_u<T>;

  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static auto get_handle(const data_client_t & data_client) {
    return storage_class_u<ragged>::get_handle<DATA_CLIENT_TYPE,
      entry_value_u<DATA_TYPE>, NAMESPACE, NAME, VERSION>(data_client);
  }

  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static auto get_mutator(const data_client_t & data_client, size_t slots) {
    return storage_class_u<ragged>::get_mutator<DATA_CLIENT_TYPE,
      entry_value_u<DATA_TYPE>, NAMESPACE, NAME, VERSION>(data_client, slots);
  }
}; // struct storage_class_t

} // namespace legion
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
