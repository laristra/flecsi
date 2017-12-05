/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_structured_mesh_types_h
#define flecsi_topology_structured_mesh_types_h

#include <array>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#include "flecsi/data/data_client.h"
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/topology/structured_index_space.h"
#include "flecsi/topology/types.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation:
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {  

//----------------------------------------------------------------------------//
//! The structured_mesh_entity_ type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

class structured_mesh_topology_base_t;

class structured_mesh_entity_base_{
public:
 using sm_id_t = size_t;
};


template <size_t N>
class structured_mesh_entity_base_t : public structured_mesh_entity_base_ 
{
 public:
  structured_mesh_entity_base_t(){};

  virtual ~structured_mesh_entity_base_t() {}
  
  template<size_t M>
  size_t id() const
  {
    return id_[M];
  }

  size_t id(size_t domain) const
  {
    return id_[domain];
  }

  template<size_t M>
  void set_id(const size_t &id)
  {
    id_[M] = id;
  }

  void set_id(const size_t &id, size_t domain)
  {
    id_[domain] = id;
  }

  template <class MT>
  friend class structured_mesh_topology_t;

 private:
  std::array<sm_id_t,N> id_;
}; // class structured_mesh_entity_base_t


template <size_t D, size_t N>
class structured_mesh_entity_t : public structured_mesh_entity_base_t<N>
{
 public:
  static const size_t dimension = D;

  structured_mesh_entity_t() : structured_mesh_entity_base_t<N>(){};
  virtual ~structured_mesh_entity_t() {}
}; // class mesh_entity_t

/******************************************************************************
 *                       Structured Mesh Storage                              *
 ******************************************************************************/
 /*
 * D = num_dimensions, NM = num_domains
 */
template <size_t D, size_t NM>
struct structured_mesh_storage_t {
  using index_spaces_t = 
    std::array<structured_index_space<structured_mesh_entity_base_*,D>, D+1>;

  std::array<index_spaces_t, NM> index_spaces;
}; // struct mesh_storage_t


//----------------------------------------------------------------------------//
//! The structured_mesh_topology_base_t type...
//!
//! @ingroup
//----------------------------------------------------------------------------//
class structured_mesh_topology_base_t : public data::data_client_t
{
public:
  // Default constructor
  structured_mesh_topology_base_t() = default;

  // Don't allow the mesh to be copied or copy constructed
  structured_mesh_topology_base_t(const structured_mesh_topology_base_t &) = delete;
  structured_mesh_topology_base_t & operator=(const structured_mesh_topology_base_t &) = delete;

  /// Allow move operations
  structured_mesh_topology_base_t(structured_mesh_topology_base_t &&) = default;

  //! override default move assignement
  structured_mesh_topology_base_t & operator=(structured_mesh_topology_base_t && o)
  {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };

  /*!
    Return the number of entities in for a specific domain and topology dim.
   */
  virtual size_t num_entities(size_t dim, size_t domain) const = 0;

}; // structured_mesh_topology_base_t


} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_structured_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
