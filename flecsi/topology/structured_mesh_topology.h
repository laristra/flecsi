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
  structured_mesh_topology__(sm_id_array_t global_lbnds, 
                             sm_id_array_t global_ubnds,
                             sm_id_array_t global_strides, 
                             sm_id_t primary_dim,  
			     storage_t * ms = nullptr) : base_t(ms)
  {
     if (ms != nullptr)
     {
       initialize_storage(global_lbnds, global_ubnds, global_strides, primary_dim);
     } 
  }

  // mesh destructor
  virtual ~structured_mesh_topology__()
  {
    //delete qt;
  }
  
 //--------------------------------------------------------------------------//
 //! initialization routine to set up correct lower and upper bounds
 //!
 //! @param global_lbnds   The lower bounds of the global box. 
 //! @param global_ubnds   The upper bounds of the global box.
 //! @param global_strides The strides along each direction in the global box
 //! @param primary_dim    The dimension of the entities that this box represents. 
 //!                       It can only be the lowest or highest dimension.
 //!                       That is, the input can represent bounds for 
 //!                       either vertices or cells. 
 //--------------------------------------------------------------------------//
  void initialize_storage(sm_id_array_t global_lbnds, 
                          sm_id_array_t global_ubnds,
                          sm_id_array_t global_strides,
                          sm_id_t primary_dim)
  {
      meshdim_ = MESH_TYPE::num_dimensions;  

      //primary_dim can only be the lowest or highest dimension.      
      assert(primary_dim == 0 || primary_dim == meshdim_); 
      primary_dim_ = primary_dim; 

      for (size_t i = 0; i < meshdim_; ++i)
      {
        globalbnds_low_[i] = global_lbnds[i];
        globalbnds_up_[i]  = global_ubnds[i];
        global_strides_[i] = global_strides[i]; 
      }

      //bounds info for cells/verts
      std::vector<int> bnds_info[2][3][4] = {
                           //cells
                           {{{{1}}, {{0}}, {{}}, {{}}},
                            {{{1,1}}, {{1,0,0,1}}, {{0,0}}, {{}}},
                            {{{1,1,1}}, {{1,0,1,0,1,1,1,1,0}}, 
                            {{1,0,0,0,1,0,0,0,1}}, {{0,0,0}}}}, 
                           //verts
                           {{{{0}}, {{-1}}, {{}}, {{}}},
                            {{{0,0}}, {{0,-1,-1,0}}, {{-1,-1}}, {{}}},
                            {{{0,0,0}}, {{0,-1,0,-1,0,0,0,0,-1}}, 
                            {{0,-1,-1,-1,0,-1,-1,-1,0}}, {{-1,-1,-1}}}}, 
                          };

      bool primary;
      for (size_t i = 0; i <= meshdim_; ++i)
      {
        if ( i != primary_dim_)
             primary = false;
        else 
             primary = true; 
    
        base_t::ms_->index_spaces[0][i].init(primary, primary_dim_,  
        globalbnds_low_, globalbnds_up_, global_strides_, 
        (primary_dim_ == 0) ? bnds_info[1][meshdim_-1][i]: 
        bnds_info[0][meshdim_-1][i]);
      } 

     //create query table once
     //qt = new query::QueryTable<MESH_TYPE::num_dimensions, 
     //                           MESH_TYPE::num_dimensions+1,
     //                           MESH_TYPE::num_dimensions, 
     //                           MESH_TYPE::num_dimensions+1>(); 
    
  }
 
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
 //! Return lower bounds of the primary IS of the global mesh. 
 //--------------------------------------------------------------------------//
  auto primary_lower_bounds()
  {
    return globalbnds_low_;
  }//primary_lower_bounds

 //--------------------------------------------------------------------------//
 //! Return upper bounds of the primary IS of the global mesh. 
 //--------------------------------------------------------------------------//
  auto primary_upper_bounds()
  {
    return globalbnds_up_;
  }//primary_upper_bounds 

  //--------------------------------------------------------------------------//
 //! Return strides of the primary IS of the global mesh. 
 //--------------------------------------------------------------------------//
  auto primary_strides()
  {
    return global_strides_;
  }//primary_strides

//--------------------------------------------------------------------------//
 //! Global to global 
 //--------------------------------------------------------------------------//
 
//--------------------------------------------------------------------------//
 //! Return the global box offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_offset_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_global_offset(global_offset);
  }//get_global_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM, class E>
  auto get_global_box_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_global_offset(e->id(0)); 
  }//get_global_box_offset_from_global_offset

//--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto get_global_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_global_offset_from_global_box_offset

//--------------------------------------------------------------------------//
 //! Return the global box indices given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto get_global_box_indices_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_global_box_indices_from_global_box_offset

//--------------------------------------------------------------------------//
 //! Return the global box offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_global_box_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_global_box_offset_from_global_box_indices


 //--------------------------------------------------------------------------//
 //! Return the global box indices given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_global_box_indices_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_global_offset(global_offset);
  }//get_global_box_indices_from_global_offset

  //--------------------------------------------------------------------------//
 //! Return the global box indices given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM, class E>
  auto get_global_box_indices_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_global_offset(e->id(0)); 
  }//get_global_box_indices_from_global_offset

//--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_global_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_global_offset_from_global_box_indices

//--------------------------------------------------------------------------//
 //! Local to local 
 //--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//
 //! Return the local box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto get_local_box_offset_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_local_offset(local_offset);
  }//get_local_box_offset_from_local_offset

//--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_local_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_local_offset_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_local_box_indices_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_local_box_indices_from_local_box_offset

//--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template<size_t DIM,size_t DOM = 0 >
  auto get_local_box_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_local_box_offset_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0 >
  auto get_local_box_indices_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_local_offset(local_offset);
  }//get_local_box_indices_from_local_offset  

//--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_local_offset_from_local_box_indices


 //--------------------------------------------------------------------------//
 //! Global to local and local to global
 //--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_indices_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_local_box_indices
                                           (local_box_id, local_box_indices);
  } //get_global_box_indices_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_indices_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_box_indices_from_global_box_indices


//--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_box_offset_from_local_box_offset

//--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_box_offset_from_global_box_offset

//--------------------------------------------------------------------------//
 //! Return the global offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_offset_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_offset_from_local_offset(local_offset);
  }//get_global_offset_from_local_offset    

//--------------------------------------------------------------------------//
 //! Return the local offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM, class E >
  auto get_local_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_global_offset(e->id(0)); 
  }//get_local_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_offset_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_global_offset(global_offset);
  }//get_local_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_indices_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_box_indices_from_local_box_offset

   //--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_box_offset_from_global_box_indices
  
//--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_global_box_offset_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_indices_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_box_indices_from_global_box_offset

  //--------------------------------------------------------------------------//
 //! Return the global box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_offset_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_offset_from_local_offset(local_offset);
  }//get_global_box_offset_from_local_offset

//--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and offset w.r.t to 
 //! the box. 
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_global_box_offset
                                           (global_box_id, global_box_offset);
  }//get_local_offset_from_global_box_offset

//--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and offset w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_offset_from_local_box_offset
                                           (local_box_id, local_box_offset);
  }//get_global_offset_from_local_box_offset


 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_offset_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_global_offset(global_offset);
  }//get_local_box_offset_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM, class E >
  auto get_local_box_offset_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].local_box_offset_from_global_offset(e->id(0)); 
  }//get_local_box_offset_from_global_offset

//--------------------------------------------------------------------------//
 //! Return the global box indices given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_box_indices_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_box_indices_from_local_offset(local_offset);
  }//get_global_box_indices_from_local_offset

//--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and indices w.r.t to 
 //! the box. 
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_offset_from_global_box_indices
                                           (global_box_id, global_box_indices);
  }//get_local_offset_from_global_box_indices

//--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and indices w.r.t to 
 //! the box. 
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_global_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].global_offset_from_local_box_indices
                                           (local_box_id, local_box_indices);
  }//get_global_offset_from_local_box_indices

  //--------------------------------------------------------------------------//
 //! Return the local box indices given a global offset. 
 //! @param global_offset Global offset of an entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_local_box_indices_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_global_offset(global_offset);
  }//get_local_box_indices_from_global_offset

//--------------------------------------------------------------------------//
 //! Return the local box indices given an entity instance.  
 //! @param e Entity instance
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM, class E >
  auto get_local_box_indices_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].local_box_indices_from_global_offset(e->id(0)); 
  }//get_local_box_indices_from_global_offset

//--------------------------------------------------------------------------//
 //! Box ids from global/local 
 //--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//
 //! Return the id of the global box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param global_offset   global offset of the entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0 >
  auto get_box_id_from_global_offset(const sm_id_t& global_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].find_box_id_from_global_offset(global_offset);
  } //get_box_id_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the id of the global box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param e   the entity instance
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM, class E >
  auto get_box_id_from_global_offset(E* e)
  {
    return base_t::ms_->index_spaces[DOM][DIM].find_box_id_from_global_offset(e->id(0)); 
  } //get_box_id_from_global_offset

 //--------------------------------------------------------------------------//
 //! Return the id of the local box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //! 
 //! @param local_offset   local offset of the entity 
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0  >
  auto get_box_id_from_local_offset(const sm_id_t& local_offset)
  { 
    return base_t::ms_->index_spaces[DOM][DIM].find_box_id_from_local_offset(local_offset);
  } //get_box_id_from_local_offset



 /******************************************************************************
 *                      Query Methods for Cartesian Block                      *
 * ****************************************************************************/ 

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
    size_t domain = 0
  ) const override
  {
    return base_t::ms_->index_spaces[domain][dim].size();
  } // num_entities
 
  template<
    size_t DIM,
    size_t DOM = 0
    >
  decltype(auto)
  num_entities() const
  {
    return base_t::ms_->index_spaces[DOM][DIM].size();
  } // num_entities

 //--------------------------------------------------------------------------//
 //! Return the an iterable instance for entities in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  template< size_t DIM, size_t DOM = 0>
  auto entities()
  {
    using etype = entity_type<DIM, DOM>;
    return base_t::ms_->index_spaces[DOM][DIM].template iterate_all<etype>(); 
  } // entities
 
  template< size_t DIM, size_t DOM = 0 >
  auto entities() const
  {
    using etype = entity_type<DIM, DOM>;
    return base_t::ms_->index_spaces[DOM][DIM].template iterate_all<etype>();
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
  template<size_t TO_DIM, 
           size_t FROM_DOMAIN , 
           size_t TO_DOMAIN = FROM_DOMAIN,  
           class E>
  auto entities(E* e)
  {
    size_t FROM_DIM = E::dimension;
    assert(FROM_DIM != TO_DIM);
    sm_id_t id = e->id(0);

    size_t box_id = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM]. 
                    find_box_id_from_global_offset(id);
    auto indices = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM]. 
                   local_box_indices_from_global_offset(id);

    using etype = entity_type<TO_DIM,TO_DOMAIN>;
    return base_t::ms_->index_spaces[TO_DOMAIN][TO_DIM].template 
           iterate_local_adjacency<TO_DIM,etype>(FROM_DIM, box_id, indices);
           //iterate_local_adjacency<TO_DIM,etype>(FROM_DIM, BD, indices, qt);
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
  size_t FROM_DOMAIN,  
  class E>
  auto stencil_entity(E* e)
  {
    //This assertion check is needed to distinguish between the case where the 
    //stencil is the zero vector from the case when the stencil results in an 
    //out of bound entity in which case the id returned is that of the origin.  
    assert (!(xoff == 0)); 

    size_t FROM_DIM = E::dimension;
    size_t global_offset = e->id(0); 
    size_t box_id = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM]. 
                    find_box_id_from_global_offset(global_offset);
    auto indices = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                   local_box_indices_from_global_offset(global_offset);
    auto local_offset = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                        local_offset_from_local_box_indices(box_id, indices);                
  
    if(base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
       check_local_index_limits<0>(box_id,xoff+indices[0]))
    {
      local_offset += xoff;
    }
    return base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
           global_offset_from_local_offset(local_offset);
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
  size_t FROM_DOMAIN, 
  class E>
  auto stencil_entity(E* e)
  {
    //This assertion check is needed to distinguish between the case where the 
    //stencil is the zero vector from the case when the stencil results in an 
    //out of bound entity in which case the id returned is that of the origin.  
    assert(!((xoff == 0) && (yoff == 0)));

    size_t FROM_DIM = E::dimension;
    size_t global_offset = e->id(0);
    size_t box_id = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                    find_box_id_from_global_offset(global_offset);
    auto indices = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                   local_box_indices_from_global_offset(global_offset);
    auto local_offset = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                        local_offset_from_local_box_indices(box_id, indices); 
 
    if((base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
       check_local_index_limits<0>(box_id,xoff+indices[0]))&& 
       (base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
       check_local_index_limits<1>(box_id,yoff+indices[1])))
    {
      size_t nx = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
                  get_local_size_in_direction<0>(box_id);
      local_offset += xoff + nx*yoff;
    }

   return base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
           global_offset_from_local_offset(local_offset); 
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
  size_t FROM_DOMAIN, 
  class E>
  auto stencil_entity(E* e)
  {
    //This assertion check is needed to distinguish between the case where the 
    //stencil is the zero vector from the case when the stencil results in an 
    //out of bound entity in which case the id returned is that of the origin.  
    assert (!((xoff == 0) && (yoff == 0) && (zoff == 0)));

    size_t FROM_DIM = E::dimension;
    size_t global_offset = e->id(0);
    size_t box_id = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                    find_box_id_from_global_offset(global_offset);
    auto indices = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                   local_box_indices_from_global_offset(global_offset);
    auto local_offset = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
                        local_offset_from_local_box_indices(box_id, indices); 

    if((base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
       check_local_index_limits<0>(box_id,xoff+indices[0])) && 
       (base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
       check_local_index_limits<1>(box_id,yoff+indices[1])) &&
       (base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template
       check_local_index_limits<2>(box_id,zoff+indices[2])))
    {
      size_t nx = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
                  get_local_size_in_direction<0>(box_id);
      size_t ny = base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].template 
                  get_local_size_in_direction<1>(box_id);
      local_offset += xoff + nx*yoff + nx*ny*zoff;
    }

    return base_t::ms_->index_spaces[FROM_DOMAIN][FROM_DIM].
           global_offset_from_local_offset(local_offset);  
  } //stencil_entity 

private:

  //Mesh dimension
  size_t meshdim_; 

  //Global Mesh Bounds and Strides
  size_t primary_dim_; 
  sm_id_array_t globalbnds_low_;
  sm_id_array_t globalbnds_up_;
  sm_id_array_t global_strides_; 


  //Helper struct for traversal routines 
  //query::QueryTable<MESH_TYPE::num_dimensions, MESH_TYPE::num_dimensions+1, 
  //                  MESH_TYPE::num_dimensions, MESH_TYPE::num_dimensions+1>  *qt; 


}; // class structured_mesh_topology__

} // namespace topology
} // namespace flecsi

