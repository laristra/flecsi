/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_legion_registration_wrapper_h
#define flecsi_data_legion_registration_wrapper_h

///
/// \file
/// \date Initial file creation: Apr 10, 2017
///

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/topology/mesh_topology.h"
#include "flecsi/utils/tuple_walker.h"

clog_register_tag(registration);

namespace flecsi {
namespace data {

///
/// \class legion_registration_wrapper_t registration_wrapper.h
/// \brief legion_registration_wrapper_t provides...
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
struct legion_registration_wrapper__
{
  using field_id_t = Legion::FieldID;

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

}; // class legion_registration_wrapper__

///
/// \class legion_registration_wrapper_t registration_wrapper.h
/// \brief legion_registration_wrapper_t provides...
///
template<
  typename DATA_CLIENT_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct legion_client_registration_wrapper__
{
}; // class legion_client_registration_wrapper__

template<
  typename POLICY_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH
>
struct legion_client_registration_wrapper__<
  flecsi::topology::mesh_topology_t<POLICY_TYPE>,
  NAMESPACE_HASH,
  NAME_HASH
>
{
  using field_id_t = Legion::FieldID;

  //

  struct entity_walker_t :
    public flecsi::utils::tuple_walker__<entity_walker_t>
  {

    template<
      typename TUPLE_ENTRY_TYPE
    >
    void
    handle_type()
    {
      using CLIENT_TYPE =
        typename flecsi::topology::mesh_topology_t<POLICY_TYPE>;
      using DOMAIN_TYPE =
        typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      std::cerr << "namespace hash: " <<  NAMESPACE_HASH << std::endl;
      std::cerr << "name hash: " << NAME_HASH << std::endl;
      std::cerr << "domain: " << DOMAIN_TYPE::value << std::endl;
      std::cerr << "dimension: " << ENTITY_TYPE::dimension << std::endl;

      
      // temporary until interface change
      const size_t INDEX_SPACE = ENTITY_TYPE::dimension;     

      // FIXME: pack all of the hash information into one key.
      // This is a hack! Need to fix this in the storage interface.
      const size_t entity_hash =
        (NAMESPACE_HASH | NAME_HASH) << 6 | 
        INDEX_SPACE << 4 |
        DOMAIN_TYPE::value << 2 |
        ENTITY_TYPE::dimension;

#if 0
      flecsi::data::storage_t::instance().register_data<
        CLIENT_TYPE,
        flecsi::data::dense,
        ENTITY_TYPE,
        entity_hash,
        0,
        INDEX_SPACE,
        1
      >();
#endif

    } // handle_type

  }; // struct entity_walker_t

  //

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog_tag_guard(registration);
    clog(error) << "In client_register_callback" << std::endl;
    // Do stuff

    using entity_types_t = typename POLICY_TYPE::entity_types;

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types_t>();
  } // register_callback

}; // class legion_client_registration_wrapper__

} // namespace data
} // namespace flecsi

#endif // flecsi_data_legion_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
