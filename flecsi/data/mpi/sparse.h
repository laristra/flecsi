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

#ifndef flecsi_mpi_sparse_h
#define flecsi_mpi_sparse_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE mpi
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/data/common/data_types.h"
#include "flecsi/data/common/privilege.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/sparse_data_handle.h"
#include "flecsi/data/mutator_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/index_space.h"

#include <algorithm>
#include <memory>

///
/// \file
/// \date Initial file creation: Oct 05, 2017
///

namespace flecsi {
namespace data {
namespace mpi {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense handle.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Dense accessor.
//----------------------------------------------------------------------------//

///
/// \brief dense_accessor_t provides logically array-based access to data
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
struct sparse_handle_t : public sparse_data_handle__<T, EP, SP, GP>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using base = sparse_data_handle__<T, EP, SP, GP>;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  // FIXME: calling to base class constructor?
  ///
  /// Default constructor.
  ///
  sparse_handle_t() {}

	///
  /// Copy constructor.
	///
//	sparse_handle_t(
//    const sparse_handle_t & a
//  )
//  :
//    label_(a.label_)
//  {}

  template<size_t EP2, size_t SP2, size_t GP2>
  sparse_handle_t(const sparse_handle_t<T, EP2, SP2, GP2> & h)
    : base(reinterpret_cast<const base&>(h)),
      label_(h.label_)
  {}

  sparse_handle_t(
    size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost
  )
  : base(num_exclusive, num_shared, num_ghost){}

  template<typename, size_t, size_t, size_t>
  friend class sparse_handle_t;

private:
  std::string label_ = "";
}; // struct sparse_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

///
/// FIXME: Dense storage type.
///
template<>
struct storage_type__<sparse>
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
  using handle_t = sparse_handle_t<T, EP, SP, GP>;

  template<
    typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION
  >
  static
  handle_t<DATA_TYPE, 0, 0, 0>
  get_handle(
    const data_client_t & data_client
  )
  {

    auto& context = execution::context_t::instance();
  
    using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

    // get field_info for this data handle
    auto& field_info =
      context.get_field_info(
        typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    auto& registered_sparse_field_data = 
      context.registered_sparse_field_data();
    auto fieldDataIter = registered_sparse_field_data.find(field_info.fid);
    if (fieldDataIter == registered_sparse_field_data.end()) {
      // get color_info for this field.
      auto& color_info = (context.coloring_info(field_info.index_space)).at(context.color());
      auto &index_coloring = context.coloring(field_info.index_space);

      // TODO: these parameters need to be passed in field
      // registration, or defined elsewhere
      const size_t max_entries_per_index = 5;
      const size_t reserve_chunk = 8192;

      // TODO: deal with VERSION
      context.register_sparse_field_data(field_info.fid, field_info.size,
        color_info, max_entries_per_index, reserve_chunk);

      context.register_sparse_field_metadata<DATA_TYPE>(
        field_info.fid, color_info, index_coloring);
    }

    auto& fd = registered_sparse_field_data[field_info.fid];

    handle_t<DATA_TYPE, 0, 0, 0> 
      h(fd.num_exclusive, fd.num_shared, fd.num_ghost);

    auto &hb = dynamic_cast<sparse_data_handle__<DATA_TYPE, 0, 0, 0>&>(h);
    
    hb.fid = field_info.fid;
    hb.index_space = field_info.index_space;
    hb.data_client_hash = field_info.data_client_hash;
    
    hb.entries = 
      reinterpret_cast<sparse_entry_value__<DATA_TYPE>*>(&fd.entries[0]);
   
    hb.offsets = &fd.offsets[0];
    hb.max_entries_per_index = fd.max_entries_per_index;
    hb.reserve = fd.reserve;
    hb.num_exclusive_entries = fd.num_exclusive_entries;

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
  mutator_handle__<DATA_TYPE>
  get_mutator(
    const data_client_t & data_client,
    size_t slots
  )
  {
    auto& context = execution::context_t::instance();
    
    using client_type = typename DATA_CLIENT_TYPE::type_identifier_t;

    // get field_info for this data handle
    auto& field_info =
      context.get_field_info(
        typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    auto& registered_sparse_field_data = 
      context.registered_sparse_field_data();
    auto fieldDataIter = registered_sparse_field_data.find(field_info.fid);
    if (fieldDataIter == registered_sparse_field_data.end()) {

      // get color_info for this field.
      auto& color_info = (context.coloring_info(field_info.index_space)).at(context.color());
      auto &index_coloring = context.coloring(field_info.index_space);

      // TODO: these parameters need to be passed in field
      // registration, or defined elsewhere
      const size_t max_entries_per_index = 5;
      const size_t reserve_chunk = 8192;

      // TODO: deal with VERSION
      context.register_sparse_field_data(field_info.fid, field_info.size,
        color_info, max_entries_per_index, reserve_chunk);

      context.register_sparse_field_metadata<DATA_TYPE>(
        field_info.fid, color_info, index_coloring);
    }

    auto& fd = registered_sparse_field_data[field_info.fid];

    mutator_handle__<DATA_TYPE> h(fd.num_exclusive, fd.num_shared, 
      fd.num_ghost, fd.max_entries_per_index, slots);
    h.offsets = &fd.offsets;
    h.entries = &fd.entries;
    h.reserve = &fd.reserve;
    h.reserve_chunk = fd.reserve_chunk;
    h.num_exclusive_entries = &fd.num_exclusive_entries;

    return h;
  }

}; // struct storage_type_t

} // namespace mpi
} // namespace data
} // namespace flecsi

#endif // flecsi_mpi_sparse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
