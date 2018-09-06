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

#include <array>
#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>
#include <functional>
#include <type_traits>

#include "flecsi/utils/common.h"
#include "flecsi/utils/static_verify.h"
#include "flecsi/topology/mesh_storage.h"
#include "flecsi/topology/structured_mesh_types.h"
#include "flecsi/topology/structured_querytable.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation:
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {
namespace verify_structmesh {

  FLECSI_MEMBER_CHECKER(num_dimensions);
  FLECSI_MEMBER_CHECKER(num_domains);
  FLECSI_MEMBER_CHECKER(entity_types);
} // namespace verify_structmesh


//----------------------------------------------------------------------------//
//! The structured_mesh_topology type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

template<class MESH_TYPE>
class structured_mesh_topology__ : public structured_mesh_topology_base__<
                                          structured_mesh_storage__<
                                          MESH_TYPE::num_dimensions,
                                          MESH_TYPE::num_domains>>
{
 
  /*
  * Verify the existence of following fields in the mesh policy MT
  * num_dimensions
  * num_domains
  * entity_types
  */
  // static verification of mesh policy

  static_assert(verify_structmesh::has_member_num_dimensions<MESH_TYPE>::value,
                "mesh policy missing num_dimensions size_t");
  
  static_assert(std::is_convertible<decltype(MESH_TYPE::num_dimensions),
    size_t>::value, "mesh policy num_dimensions must be size_t");              

  static_assert(verify_structmesh::has_member_num_domains<MESH_TYPE>::value,
                "mesh policy missing num_domains size_t");
  
  static_assert(std::is_convertible<decltype(MESH_TYPE::num_domains),
    size_t>::value, "mesh policy num_domains must be size_t");

  static_assert(verify_structmesh::has_member_entity_types<MESH_TYPE>::value,
                "mesh policy missing entity_types tuple");
  
  static_assert(utils::is_tuple<typename MESH_TYPE::entity_types>::value,
                "mesh policy entity_types is not a tuple");


public:
  // used to find the entity type of topological dimension D and domain M
  template<size_t D, size_t M = 0>
  using entity_type = typename find_entity_<MESH_TYPE, D, M>::type;
  
  using sm_id_t        = size_t; 
  using sm_id_array_t = std::array<size_t, MESH_TYPE::num_dimensions>;  
  using sm_id_vector_2d_t  = std::vector<std::vector<size_t>>;

  // storage type
  using storage_t = structured_mesh_storage__<
                    MESH_TYPE::num_dimensions,
                    MESH_TYPE::num_domains>;

  // base type for mesh
  using base_t = structured_mesh_topology_base__<storage_t>;

 //--------------------------------------------------------------------------//
  // This type definition is needed so that data client handles can be
  // specialized for particular data client types, e.g., mesh topologies vs.
  // tree topologies. It is also useful for detecting illegal usage, such as
  // when a user adds data members.
  //--------------------------------------------------------------------------//
  using type_identifier_t = structured_mesh_topology__; 

  // Don't allow the mesh to be copied 
  structured_mesh_topology__ & operator=(const structured_mesh_topology__ &)
  = delete;

  // Allow move operations
  structured_mesh_topology__(structured_mesh_topology__ && o) = default;
  structured_mesh_topology__ & operator=(structured_mesh_topology__ && o) 
  = default;

  // Copy constructor
  structured_mesh_topology__(const structured_mesh_topology__ & m) : base_t(m.ms_) {}

  //! Constructor
  structured_mesh_topology__(sm_id_array_t lower_bnds, 
                             sm_id_array_t upper_bnds, 
			     storage_t * ms = nullptr) : base_t(ms)
  {
     if (ms != nullptr)
     {
       initialize_storage(lower_bnds, upper_bnds);
     } 
  }

  // mesh destructor
  virtual ~structured_mesh_topology__()
  {
    delete qt;
  }
  
  void initialize_storage(sm_id_array_t global_lbnds, 
                          sm_id_array_t global_ubnds,
                          sm_id_array_t strides)
  {
      meshdim_ = MESH_TYPE::num_dimensions;  

      for (size_t i = 0; i < meshdim_; ++i)
      {
        globalbnds_low_[i] = global_lbnds[i];
        globalbnds_up_[i]  = global_ubnds[i];
        global_strides_[i] = strides[i]; 
      }

      for (size_t i = 0; i < meshdim_; ++i)
      {
        localbnds_low_[i] = 0;
        localbnds_up_[i]  = globalbnds_up_[i] - globalbnds_low_[i];
      }

      //bounds info
      std::vector<size_t> bnds_info[3][4] =
                           {{{{1}}, {{0}}, {{}}, {{}}},
                            {{{1,1}}, {{1,0,0,1}}, {{0,0}}, {{}}},
                            {{{1,1,1}}, {{1,0,1,0,1,1,1,1,0}}, 
                            {{1,0,0,0,1,0,0,0,1}}, {{0,0,0}}}};


      bool primary = false;
      for (size_t i = 0; i <= meshdim_; ++i)
      {
        if ( i == meshdim_) primary = true;
        base_t::ms_->index_spaces[0][i].init(primary, localbnds_low_, 
        localbnds_up_, bnds_info[meshdim_-1][i]);
      } 

     //create query table once
     qt = new query::QueryTable<MESH_TYPE::num_dimensions, 
                                MESH_TYPE::num_dimensions+1,
                                MESH_TYPE::num_dimensions, 
                                MESH_TYPE::num_dimensions+1>(); 
    
  }

 //--------------------------------------------------------------------------//
 //! return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  size_t
  num_entities(
    size_t dim,
    size_t domain=0
  ) const override
  {
    return num_entities_(dim, domain);
  } // num_entities

  template<
    size_t D,
    size_t M = 0
    >
  decltype(auto)
  num_entities() const
  {
    return base_t::ms_->index_spaces[M][D].size();
  } // num_entities
 
 /******************************************************************************
 * Methods to query various local/global, local to global and global to local *
 * details of the mesh representation.
 *
 * Local View: 
 * local_offset: Unique offset in the local index space  
 * local_box_offset: Offset w.r.t the local box to which the entity belongs
 * local_box_indices: Indices w.r.t the local box to which the entity belongs
 *
 * Global View: 
 * global_offset: Unique offset in the global index space 
 * global_box_offset: Offset w.r.t the global box to which the entity belongs
 * global_box_indices: Indices w.r.t the global box to which the entity belongs 
 * *****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Return the lower bounds of the local mesh. 
 //--------------------------------------------------------------------------//
  auto localmesh_lower_bounds()
  {
    return localbnds_low_;
  }//localmesh_lower_bounds

 //--------------------------------------------------------------------------//
 //! Return the lower bounds of the global mesh. 
 //--------------------------------------------------------------------------//
  auto globalmesh_lower_bounds()
  {
    return globalbnds_low_;
  }//globalmesh_lower_bounds

 //--------------------------------------------------------------------------//
 //! Return the upper bounds of the local mesh. 
 //--------------------------------------------------------------------------//
  auto localmesh_upper_bounds()
  {
    return localbnds_up_;
  }//localmesh_upper_bounds

 //--------------------------------------------------------------------------//
 //! Return the upper bounds of the global mesh. 
 //--------------------------------------------------------------------------//
  auto globalmesh_upper_bounds()
  {
    return globalbnds_up_;
  }//globalmesh_upper_bounds

 //--------------------------------------------------------------------------//
 //! From global_offset to local_offset, local_box_offset, local_box_indices, 
 //! global_box_offset, global_box_indices 
 //--------------------------------------------------------------------------//
 
 //--------------------------------------------------------------------------//
 //! Return the local offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_offset_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_offset_from_global_offset(global_offset);
  }//get_local_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_local_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].local_offset_from_global_offset(e->id(0)); 
  }//get_local_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_offset_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_global_offset(global_offset);
  }//get_local_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_local_box_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_global_offset(e->id(0)); 
  }//get_local_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local box indices given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_indices_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_indices_from_global_offset(global_offset);
  }//get_local_box_indices_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local box indices given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_local_box_indices_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].local_box_incides_from_global_offset(e->id(0)); 
  }//get_local_box_indices_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_offset_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_global_offset(global_offset);
  }//get_global_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_global_box_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_global_offset(e->id(0)); 
  }//get_global_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box indices given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_indices_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_global_offset(global_offset);
  }//get_global_box_indices_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box indices given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_global_box_indices_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_global_offset(e->id(0)); 
  }//get_global_box_indices_from_global_offset

  //! From global_box_offset to local_offset, local_box_offset, local_box_indices, 
  //! global_offset, global_box_indices 
  //

 //--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_offset_from_global_box_offset(sm_id_t global_box_id, sm_id_t global_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_offset_from_global_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_offset_from_global_box_offset(sm_id_t global_box_id, sm_id_t global_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_box_offset_from_global_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_indices_from_global_box_offset(sm_id_t global_box_id, sm_id_t global_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_indices_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_box_indices_from_global_box_offset

 //--------------------------------------------------------------------------//
 //! Return the global box indices given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_indices_from_global_box_offset(sm_id_t global_box_id, sm_id_t global_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_indices_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_global_box_indices_from_global_box_offset

 //--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_offset_from_global_box_offset(sm_id_t global_box_id, sm_id_t global_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_global_offset_from_global_box_offset

  //! From global_box_indices to local_offset, local_box_offset, local_box_indices, 
  //! global_box_offset, global_offset
  //
  
 //--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_offset_from_global_box_indices(sm_id_t global_box_id, sm_id_array_t global_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].local_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_offset_from_global_box_indices

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_offset_from_global_box_indices(sm_id_t global_box_id, sm_id_array_t global_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_box_offset_from_global_box_indices

 //--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_indices_from_global_box_indices(sm_id_t global_box_id, sm_id_array_t global_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_indices_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_box_indices_from_global_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_offset_from_global_box_indices(sm_id_t global_box_id, sm_id_array_t global_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_global_box_offset_from_global_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_offset_from_global_box_indices(sm_id_t global_box_id, sm_id_array_t global_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].global_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_global_offset_from_global_box_indices

  //! From local_offset to local_box_offset, local_box_indices, global_offset, 
  //! global_box_offset, global_box_indices 
  //
  
 //--------------------------------------------------------------------------//
 //! Return the local box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_offset_from_local_offset(sm_id_t local_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_local_offset(local_offset);
  }//get_local_box_offset_from_local_offset

 //--------------------------------------------------------------------------//
 //! Return the local box indices given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_indices_from_local_offset(sm_id_t local_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_indices_from_local_offset(local_offset);
  }//get_local_box_indices_from_local_offset

 //--------------------------------------------------------------------------//
 //! Return the global offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_offset_from_local_offset(sm_id_t local_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_offset_from_local_offset(local_offset);
  }//get_global_offset_from_local_offset

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_offset_from_local_offset(sm_id_t local_offset
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_local_offset(local_offset);
  }//get_global_box_offset_from_local_offset

 //--------------------------------------------------------------------------//
 //! Return the global box indices given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_indices_from_local_offset(sm_id_t local_offset
  { 
    return base_t::ms_->index_spaces[M][D].global_box_indices_from_local_offset(local_offset);
  }//get_global_box_indices_from_local_offset

  //! From local_box_offset to local_offset, local_box_indices, global_offset, 
  //! global_box_offset, global_box_indices 
  //

 //--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_offset_from_local_box_offset(sm_id_t local_box_id, sm_id_t local_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_local_offset_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_indices_from_local_box_offset(sm_id_t local_box_id, sm_id_t local_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_indices_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_local_box_indices_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_offset_from_local_box_offset(sm_id_t local_box_id, sm_id_t local_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_offset_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_offset_from_local_box_offset(sm_id_t local_box_id, sm_id_t local_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_box_offset_from_local_box_offset
  
 //--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_indices_from_local_box_offset(sm_id_t local_box_id, sm_id_t local_box_offset)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_indices_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_box_indices_from_local_box_offset

  //! From local_box_indices to local_offset, local_box_offset, global_offset, 
  //! global_box_offset, global_box_indices 
  //
  
 //--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_offset_from_local_box_indices(sm_id_t local_box_id, sm_id_array_t local_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].local_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_local_offset_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_local_box_offset_from_local_box_indices(sm_id_t local_box_id, sm_id_array_t local_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].local_box_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_local_box_offset_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_offset_from_local_box_indices(sm_id_t local_box_id, sm_id_array_t local_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].global_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_global_offset_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_offset_from_local_box_indices(sm_id_t local_box_id, sm_id_array_t local_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_global_box_offset_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_global_box_indices_from_local_box_indices(sm_id_t local_box_id, sm_id_array_t local_box_indices)
  { 
    return base_t::ms_->index_spaces[M][D].global_box_indices_from_local_box_indices
                                           (local_box_id, local_box_indices);
  } //get_global_box_indices_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Return the id of the global box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param global_offset   global offset of the entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_box_id_from_global_offset(sm_id_t global_offset)
  { 
    return base_t::ms_->index_spaces[M][D].find_box_id(global_offset);
  } //get_box_id_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the id of the global box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param e   the entity instance
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_box_id_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[M][D].find_box_id(e->id(0)); 
  } //get_box_id_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the id of the local box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param local_offset   local offset of the entity 
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_box_id_from_local_offset(sm_id_t local_offset)
  { 
    return base_t::ms_->index_spaces[M][D].find_box_id(local_offset);
  } //get_box_id_from_local_offset

 /******************************************************************************
 *                      Query Methods for Cartesian Block                      *
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Return the an iterable instance for entities in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0
  >
  auto
  entities()
  {
    using etype = entity_type<D,M>;
    return base_t::ms_->index_spaces[M][D].template iterate<etype>(); 
  } // entities
 
  template<
    size_t D,
    size_t M = 0
  >
  auto
  entities() const
  {
    using etype = entity_type<D,M>;
    return base_t::ms_->index_spaces[M][D].template iterate<etype>();
  } // entities

 //--------------------------------------------------------------------------//
 //! Return an iterable instance for adjacency query from entities in
 //! specified topological dimension and domain. Provides FEM type adjacency
 //! queries. Given an entity, find all entities of dimension TD incident 
 //! on it. Supports queries between entities of different dimensions, for 
 //! queries between entities of same dimension another finer level query 
 //! interface is provided.  
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template<size_t TD, size_t FM , size_t TM = FM,  class E>
  auto entities(E* e)
  {
    size_t FD = E::dimension;
    assert(FD != TD);
    sm_id_t id = e->id(0);
    size_t BD = base_t::ms_->index_spaces[FM][FD].template find_box_id(id);
    auto indices = base_t::ms_->index_spaces[FM][FD].template 
                   get_indices_from_offset(id);

    using etype = entity_type<TD,TM>;
    return base_t::ms_->index_spaces[TM][TD].template 
           traverse<TD,etype>(FD, BD, indices, qt);
  } //entities

 //--------------------------------------------------------------------------//
 //! Return an entity using the 1D stencil provided. This interface Provides 
 //! FD type or query between same dimensional entities. 
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template<
  std::intmax_t xoff, 
  size_t FM,  
  class E>
  auto stencil_entity(E* e)
  {
    assert(!(xoff == 0) ); 
    size_t FD = E::dimension;
    size_t value = e->id(0);
    size_t BD = base_t::ms_->index_spaces[FM][FD].template find_box_id(value);
    auto indices = base_t::ms_->index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if(base_t::ms_->index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0]))
    {
      value += xoff;
    }
    return value; 
  } //stencil_entity 

 //--------------------------------------------------------------------------//
 //! Return an entity using the 2D stencil provided. This interface Provides 
 //! FD type or query between same dimensional entities. 
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template<
  std::intmax_t xoff, 
  std::intmax_t yoff, 
  size_t FM, 
  class E>
  auto stencil_entity(E* e)
  {
    assert(!((xoff == 0) && (yoff == 0))); 
    size_t FD = E::dimension;
    size_t value = e->id(0);
    size_t BD = base_t::ms_->index_spaces[FM][FD].template find_box_id(value);
    auto indices = base_t::ms_->index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if((base_t::ms_->index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0]))&& 
       (base_t::ms_->index_spaces[FM][FD].template 
       check_index_limits<1>(BD,yoff+indices[1])))
    {
      size_t nx = base_t::ms_->index_spaces[FM][FD].template 
                  get_size_in_direction<0>(BD);
      value += xoff + nx*yoff;
    }
    return value; 
  } //stencil_entity

 //--------------------------------------------------------------------------//
 //! Return an entity using the 3D stencil provided. This interface Provides 
 //! FD type or query between same dimensional entities. 
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template<
  std::intmax_t xoff, 
  std::intmax_t yoff, 
  std::intmax_t zoff, 
  size_t FM, 
  class E>
  auto stencil_entity(E* e)
  {
    assert(!((xoff == 0) && (yoff == 0) && (zoff == 0))); 
    size_t FD = E::dimension;
    size_t value = e->id(0);
    size_t BD = base_t::ms_->index_spaces[FM][FD].template find_box_id(value);
    auto indices = base_t::ms_->index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if((base_t::ms_->index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0])) && 
       (base_t::ms_->index_spaces[FM][FD].template 
       check_index_limits<1>(BD,yoff+indices[1])) &&
       (base_t::ms_->index_spaces[FM][FD].template
       check_index_limits<2>(BD,zoff+indices[2])))
    {
      size_t nx = base_t::ms_->index_spaces[FM][FD].template 
                  get_size_in_direction<0>(BD);
      size_t ny = base_t::ms_->index_spaces[FM][FD].template 
                  get_size_in_direction<1>(BD);
      value += xoff + nx*yoff + nx*ny*zoff;
    }
    return value; 
  } //stencil_entity 

private:

  //Mesh dimension
  size_t meshdim_; 

  //Global Mesh Bounds and Strides
  size_t meshdim_; 
  sm_id_array_t globalbnds_low_;
  sm_id_array_t globalbnds_up_;
  sm_id_array_t global_strides_; 

  //Local Mesh Bounds
  sm_id_array_t localbnds_low_;
  sm_id_array_t localbnds_up_;

  //Helper struct for traversal routines 
  query::QueryTable<MESH_TYPE::num_dimensions, MESH_TYPE::num_dimensions+1, 
                    MESH_TYPE::num_dimensions, MESH_TYPE::num_dimensions+1>  *qt; 

  // Get the number of entities in a given domain and topological dimension
  size_t
  num_entities_(
    size_t dim,
    size_t domain=0
  ) const
  {
    return base_t::ms_->index_spaces[domain][dim].size();
  } // num_entities_

}; // class structured_mesh_topology__

} // namespace topology
} // namespace flecsi

