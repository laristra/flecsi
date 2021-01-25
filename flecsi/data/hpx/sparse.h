/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE hpx
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

#include <flecsi/utils/const_string.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 05, 2017
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace hpx {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

/*!
  Sparse storage type. Sparse data is partitioned into exclusive, shared,
  ghost entries. It allows entries to be allocated sparsely per index.
  Within an index, entries are stored in sorted order. A sparse accessor
  can read and modify existing entries, but cannot allocate new entries.
  The mutator is used for this purpose. A sparse data handle is passed to a
  task which is then transformed to an accessor, likewise for a mutator.
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

    auto & registered_sparse_field_data =
      context.registered_sparse_field_data();
    auto fieldDataIter = registered_sparse_field_data.find(field_info.fid);
    if(fieldDataIter == registered_sparse_field_data.end()) {
      // get color_info for this field.
      auto & color_info =
        (context.coloring_info(field_info.index_space)).at(context.color());
      auto & index_coloring = context.coloring(field_info.index_space);

      auto & im = context.sparse_index_space_info_map();
      auto iitr = im.find(field_info.index_space);
      clog_assert(iitr != im.end(),
        "sparse index space info not registered for index space: "
          << field_info.index_space);

      // TODO: these parameters need to be passed in field
      // registration, or defined elsewhere
      const size_t max_entries_per_index = iitr->second.max_entries_per_index;

      // TODO: deal with VERSION
      context.register_sparse_field_data(
        field_info.fid, field_info.size, color_info, max_entries_per_index);
    }
    auto fieldMetaDataIter =
      context.registered_sparse_field_metadata().find(field_info.fid);
    if(fieldMetaDataIter == context.registered_sparse_field_metadata().end()) {
      auto & color_info =
        (context.coloring_info(field_info.index_space)).at(context.color());
      auto & index_coloring = context.coloring(field_info.index_space);

      context.register_sparse_field_metadata<DATA_TYPE>(
        field_info.fid, color_info, index_coloring);
    }

    auto & fd = registered_sparse_field_data[field_info.fid];

    handle_u<DATA_TYPE> h(fd.max_entries_per_index);
    h.init(fd.num_exclusive, fd.num_shared, fd.num_ghost);

    auto & hb = h;

    hb.fid = field_info.fid;
    hb.index_space = field_info.index_space;

    using vector_t = typename ragged_data_handle_u<DATA_TYPE>::vector_t;
    hb.rows = reinterpret_cast<vector_t *>(&fd.rows[0]);

    auto & ism = context.index_space_data_map();
    hb.ghost_is_readable =
      &(ism[field_info.index_space].ghost_is_readable[field_info.fid]);

    hb.future = &(ism[field_info.index_space].future[field_info.fid]);

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

} // namespace hpx
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
