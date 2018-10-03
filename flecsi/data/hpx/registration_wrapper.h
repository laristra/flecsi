/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_hpx_registration_wrapper_h
#define flecsi_data_hpx_registration_wrapper_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 27, 2017
//----------------------------------------------------------------------------//

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/topology/mesh_topology.h"

#include "flecsi/utils/tuple_walker.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t INDEX_SPACE,
    size_t VERSIONS>
struct hpx_field_registration_wrapper__ {
  using field_id_t = size_t;

  static void register_callback(field_id_t fid) {
    clog(info) << "In register_callback" << std::endl;
    // Do stuff
  } // register_callback

}; // class hpx_field_registration_wrapper_t

///
/// \class registration_wrapper_t registration_wrapper.h
/// \brief registration_wrapper_t provides...
///
template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct hpx_client_registration_wrapper__ {
}; // class hpx_client_registration_wrapper_t

template<typename POLICY_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct hpx_client_registration_wrapper__<
    flecsi::topology::mesh_topology__<POLICY_TYPE>,
    NAMESPACE_HASH,
    NAME_HASH> {
  using field_id_t = size_t;
  // using mesh_topology_t = flecsi::topology::mesh_topology_t<POLICY_TYPE>;

  static void register_callback(field_id_t fid) {
    clog_tag_guard(registration);
    clog(info) << "In client_register_callback" << std::endl;
    // Do stuff
  } // register_callback

  template<typename TUPLE_ENTRY_TYPE>
  struct type_walker__
      : public flecsi::utils::tuple_walker__<type_walker__<TUPLE_ENTRY_TYPE>> {
    void handle(TUPLE_ENTRY_TYPE const & entry) {
      std::cout << "adding with domain: " << std::get<1>(entry) << std::endl;
    } // handle
  }; // struct type_walker__

  static void register_data() {
    // walk types
    type_walker__<typename POLICY_TYPE::entity_types> type_wakler;

  } // register_data

}; // struct hpx_client_registration_wrapper_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_hpx_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
