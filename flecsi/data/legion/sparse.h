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
#include <flecsi/data/mutator_handle.h>
#include <flecsi/data/sparse_data_handle.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/index_space.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {
namespace legion {

  //+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
  // Helper type definitions.
  //+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

  //----------------------------------------------------------------------------//
  // Sparse handle.
  //----------------------------------------------------------------------------//

  //----------------------------------------------------------------------------//
  // Sparse accessor.
  //----------------------------------------------------------------------------//

  ///
  /// \brief Sparse_accessor_t provides logically array-based access to data
  ///        variables that have been registered in the data model.
  ///
  /// \tparam T The type of the data variable. If this type is not
  ///           consistent with the type used to register the data, bad things
  ///           can happen. However, it can be useful to reinterpret the type,
  ///           e.g., when writing raw bytes. This class is part of the
  ///           low-level \e flecsi interface, so it is assumed that you
  ///           know what you are doing...
  ///
  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  struct sparse_handle_u : public sparse_data_handle_u<T, EP, SP, GP>
  {
    //--------------------------------------------------------------------------//
    // Type definitions.
    //--------------------------------------------------------------------------//

    using base = sparse_data_handle_u<T, EP, SP, GP>;

    //--------------------------------------------------------------------------//
    // Constructors.
    //--------------------------------------------------------------------------//

    sparse_handle_u(){}

    template<typename, size_t, size_t, size_t>
    friend class sparse_handle_u;
  }; // struct sparse_handle_u

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
  struct storage_class_u<sparse>
  {
    //--------------------------------------------------------------------------//
    // Type definitions.
    //--------------------------------------------------------------------------//

    template<
      typename T,
      size_t EP,
      size_t SP,
      size_t GP
    >
    using handle_u = sparse_handle_u<T, EP, SP, GP>;

    template<
      typename DATA_CLIENT_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE,
      size_t NAME,
      size_t VERSION
    >
    static
    handle_u<DATA_TYPE, 0, 0, 0>
    get_handle(
      const data_client_t & data_client
    )
    {
      static_assert(
          VERSION < utils::hash::field_max_versions,
          "max field version exceeded");

      auto& context = execution::context_t::instance();

      using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

      // get field_info for this data handle
      auto& field_info =
        context.get_field_info_from_name(
          typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
        utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

      size_t index_space = field_info.index_space;
      auto & ism = context.index_space_data_map();

      auto& im = context.sparse_index_space_info_map();
      auto iitr = im.find(index_space);
      clog_assert(iitr != im.end(),
        "sparse index space info not registered for index space: " <<
        index_space);

      const size_t max_entries_per_index = iitr->second.max_entries_per_index;
      const size_t exclusive_reserve = iitr->second.exclusive_reserve;

      handle_u<DATA_TYPE, 0, 0, 0> h;

      h.reserve = exclusive_reserve;
      h.max_entries_per_index = max_entries_per_index;

      h.offsets_entire_region = ism[index_space].entire_region;
      h.offsets_exclusive_lp = ism[index_space].exclusive_lp;
      h.offsets_shared_lp = ism[index_space].shared_lp;
      h.offsets_ghost_lp = ism[index_space].ghost_lp;

      // TODO: formalize sparse offset
      constexpr size_t sparse_offset = 8192;

      h.entries_entire_region = ism[index_space + sparse_offset].entire_region;
      h.entries_exclusive_lp = ism[index_space + sparse_offset].exclusive_lp;
      h.entries_shared_lp = ism[index_space + sparse_offset].shared_lp;
      h.entries_ghost_lp = ism[index_space + sparse_offset].ghost_lp;

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

      h.ghost_owners_offsets_lp = 
        ism[index_space].ghost_owners_lp;
      
      h.ghost_owners_offsets_lp = 
        ism[index_space].ghost_owners_lp;

      h.ghost_owners_entries_lp = 
        ism[index_space + sparse_offset].ghost_owners_lp;

      h.entries_color_parition = ism[index_space + sparse_offset].primary_lp;
      
      h.global_to_local_color_map_ptr =
          &ism[index_space].global_to_local_color_map;

      h.fid = field_info.fid;

      h.index_space = index_space;
      h.data_client_hash = field_info.data_client_hash;

      return h;
    }

    template<
      typename DATA_CLIENT_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE,
      size_t NAME,
      size_t VERSION
    >
    static
    mutator_handle_u<DATA_TYPE>
    get_mutator(
      const data_client_t & data_client,
      size_t slots
    )
    {
      auto& context = execution::context_t::instance();

      using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

      // get field_info for this data handle
      auto& field_info =
        context.get_field_info_from_name(
          typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
        utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

      size_t index_space = field_info.index_space;
      auto & ism = context.index_space_data_map();

      auto& im = context.sparse_index_space_info_map();
      auto iitr = im.find(index_space);
      clog_assert(iitr != im.end(),
        "sparse index space info not registered for index space: " <<
        index_space);

      const size_t max_entries_per_index = iitr->second.max_entries_per_index;
      const size_t exclusive_reserve = iitr->second.exclusive_reserve;

      mutator_handle_u<DATA_TYPE> h(max_entries_per_index, slots);

      h.offsets_entire_region = ism[index_space].entire_region;
      h.offsets_exclusive_lp = ism[index_space].exclusive_lp;
      h.offsets_shared_lp = ism[index_space].shared_lp;
      h.offsets_ghost_lp = ism[index_space].ghost_lp;

      // TODO: formalize sparse offset
      constexpr size_t sparse_offset = 8192;

      h.entries_entire_region = ism[index_space + sparse_offset].entire_region;
      h.entries_exclusive_lp = ism[index_space + sparse_offset].exclusive_lp;
      h.entries_shared_lp = ism[index_space + sparse_offset].shared_lp;
      h.entries_ghost_lp = ism[index_space + sparse_offset].ghost_lp;

      h.metadata_entire_region = context.sparse_metadata().entire_region;
      h.metadata_lp = context.sparse_metadata().color_partition;     
 
      h.ghost_is_readable = &(ism[index_space].ghost_is_readable[field_info.fid]);
      h.write_phase_started =
          &(ism[index_space].write_phase_started[field_info.fid]);


      h.ghost_owners_offsets_lp =
        ism[index_space].ghost_owners_lp;

//      h.ghost_owners_offsets_subregions =
//        ism[index_space].ghost_owners_subregions;

      h.ghost_owners_entries_lp =
        ism[index_space + sparse_offset].ghost_owners_lp;

      h.global_to_local_color_map_ptr =
          &ism[index_space].global_to_local_color_map;

      h.fid = field_info.fid;
      h.index_space = index_space;
      h.data_client_hash = field_info.data_client_hash;
      h.slots = slots;
      h.num_exclusive_insertions = new size_t(0);

      return h;
    }

  }; // struct storage_class_t

  template<>
  struct storage_class_u<ragged>
  {
    //--------------------------------------------------------------------------//
    // Type definitions.
    //--------------------------------------------------------------------------//

    template<
      typename T,
      size_t EP,
      size_t SP,
      size_t GP
    >
    using handle_u = sparse_handle_u<T, EP, SP, GP>;

    template<
      typename DATA_CLIENT_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE,
      size_t NAME,
      size_t VERSION
    >
    static
    auto
    get_handle(
      const data_client_t & data_client
    )
    {
      return storage_class_u<sparse>::get_handle<
        DATA_CLIENT_TYPE,
        DATA_TYPE,
        NAMESPACE,
        NAME,
        VERSION
      >(data_client);
    }

    template<
      typename DATA_CLIENT_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE,
      size_t NAME,
      size_t VERSION
    >
    static
    auto
    get_mutator(
      const data_client_t & data_client,
      size_t slots
    )
    {
      return storage_class_u<sparse>::get_mutator<
        DATA_CLIENT_TYPE,
        DATA_TYPE,
        NAMESPACE,
        NAME,
        VERSION
    >(data_client, slots);
    }
  }; // struct storage_class_t

} // namespace legion
} // namespace data
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
