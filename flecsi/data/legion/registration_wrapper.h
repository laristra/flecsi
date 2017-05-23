/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_registration_wrapper_h
#define flecsi_data_registration_wrapper_h

///
/// \file
/// \date Initial file creation: Apr 10, 2017
///

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/topology/mesh_topology.h"

clog_register_tag(registration);

namespace flecsi {
namespace data {

///
/// \class registration_wrapper_t registration_wrapper.h
/// \brief registration_wrapper_t provides...
///
template<
  typename DATA_CLIENT_TYPE,
  size_t STORAGE_TYPE,
  typename DATA_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH,
  size_t INDEX_SPACE,
  size_t VERSIONS
>
struct registration_wrapper_t
{
  using field_id_t = LegionRuntime::HighLevel::FieldID;

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog_tag_guard(registration);
    clog(info) << "In register_callback" << std::endl;
    
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
  } // register_callback

}; // class registration_wrapper_t

///
/// \class registration_wrapper_t registration_wrapper.h
/// \brief registration_wrapper_t provides...
///
template<
  typename DATA_CLIENT_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct client_registration_wrapper_t
{
#if 0
  using field_id_t = LegionRuntime::HighLevel::FieldID;

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog(info) << "In client_register_callback" << std::endl;
    // Do stuff
  } // register_callback

#endif
}; // class client_registration_wrapper_t

template<
  typename POLICY_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct client_registration_wrapper_t<
  flecsi::topology::mesh_topology_t<POLICY_TYPE>,
  NAMESPACE_HASH,
  NAME_HASH
>
{
  using field_id_t = LegionRuntime::HighLevel::FieldID;

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

}; // class client_registration_wrapper_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
