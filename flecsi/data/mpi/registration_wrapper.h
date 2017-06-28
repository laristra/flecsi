/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_mpi_registration_wrapper_h
#define flecsi_data_mpi_registration_wrapper_h

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
  size_t VERSIONS
>
struct mpi_field_registration_wrapper__
{
  using field_id_t = size_t;

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog(info) << "In register_callback, fid: " << fid << std::endl;

    // register field with 'fid' to the execution context.
    execution::context_t::field_info_t fi;
    fi.data_client_hash = typeid(DATA_CLIENT_TYPE).hash_code();
    fi.storage_type = STORAGE_TYPE;
    fi.size = sizeof(DATA_TYPE);
    fi.namespace_hash = NAMESPACE_HASH;
    fi.name_hash = NAME_HASH;
    fi.versions = VERSIONS;
    fi.index_space = INDEX_SPACE;
    fi.fid = fid;

    execution::context_t::instance().register_field_info(fi);

    // FIXME: TBD
    auto& context = execution::context_t::instance();
    auto& field_info = context.registered_fields()[0];

    clog(info) << "index space: " << field_info.index_space << std::endl;

    auto infos = context.coloring_info(field_info.index_space);
    for (auto pair : infos) {
      clog(info) << "number of exclusive: " << pair.second.exclusive << std::endl;
    }
    for (auto pair : infos) {
      clog(info) << "number of shared: " << pair.second.shared << std::endl;
    }
    for (auto pair : infos) {
      clog(info) << "number of ghost: " << pair.second.ghost << std::endl;
    }
  } // register_callback

}; // class mpi_field_registration_wrapper_t

///
/// \class registration_wrapper_t registration_wrapper.h
/// \brief registration_wrapper_t provides...
///
template<
  typename DATA_CLIENT_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct mpi_client_registration_wrapper__
{
}; // class mpi_client_registration_wrapper_t

template<
  typename POLICY_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct mpi_client_registration_wrapper__<
  flecsi::topology::mesh_topology_t<POLICY_TYPE>,
  NAMESPACE_HASH,
  NAME_HASH
>
{
  using field_id_t = size_t;
  //using mesh_topology_t = flecsi::topology::mesh_topology_t<POLICY_TYPE>;

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog_tag_guard(registration);
    clog(info) << "In client_register_callback" << std::endl;
    // Do stuff
  } // register_callback

  template<
    typename TUPLE_ENTRY_TYPE
  >
  struct type_walker__ :
    public flecsi::utils::tuple_walker__<type_walker__<TUPLE_ENTRY_TYPE>>
  {
    void
    handle(
      TUPLE_ENTRY_TYPE const & entry
    )
    {
      std::cout << "adding with domain: " << std::get<1>(entry) << std::endl;
    } // handle
  }; // struct type_walker__

  static
  void
  register_data()
  {
    // walk types
    type_walker__<typename POLICY_TYPE::entity_types> type_wakler;

  } // register_data

}; // struct mpi_client_registration_wrapper_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_mpi_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
