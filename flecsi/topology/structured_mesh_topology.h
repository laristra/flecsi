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
#include "flecsi/topology/structured_mesh_types.h"
#include "flecsi/topology/structured_querytable.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation:
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {
namespace verify_mesh {

  FLECSI_MEMBER_CHECKER(num_dimensions);
  FLECSI_MEMBER_CHECKER(num_domains);
  FLECSI_MEMBER_CHECKER(lower_bounds);
  FLECSI_MEMBER_CHECKER(upper_bounds);
  FLECSI_MEMBER_CHECKER(entity_types);
} // namespace verify_mesh


//----------------------------------------------------------------------------//
//! The structured_mesh_topology type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

template<
  class MT
>
class structured_mesh_topology_t : public structured_mesh_topology_base_t
{
 
  /*
  * Verify the existence of following fields in the mesh policy MT
  * num_dimensions
  * num_domains
  * entity_types
  * mesh_bounds 
  */
  // static verification of mesh policy

  static_assert(verify_mesh::has_member_num_dimensions<MT>::value,
                "mesh policy missing num_dimensions size_t");
  
  static_assert(std::is_convertible<decltype(MT::num_dimensions),
    size_t>::value, "mesh policy num_dimensions must be size_t");              

  static_assert(verify_mesh::has_member_num_domains<MT>::value,
                "mesh policy missing num_domains size_t");
  
  static_assert(std::is_convertible<decltype(MT::num_domains),
    size_t>::value, "mesh policy num_domains must be size_t");

  static_assert(verify_mesh::has_member_lower_bounds<MT>::value,
                "mesh policy missing lower_bounds array");
  
  static_assert(std::is_convertible<decltype(MT::lower_bounds),
    std::array<size_t,MT::num_dimensions>>::value,
    "mesh policy lower_bounds is not an array");

  static_assert(verify_mesh::has_member_upper_bounds<MT>::value,
                "mesh policy missing upper_bounds array");
  
  static_assert(std::is_convertible<decltype(MT::upper_bounds),
    std::array<size_t,MT::num_dimensions>>::value,
    "mesh policy upper_bounds is not an array");

  static_assert(MT::lower_bounds.size() == MT::upper_bounds.size(),
     "mesh bounds have inconsistent sizes");

  static_assert(verify_mesh::has_member_entity_types<MT>::value,
                "mesh policy missing entity_types tuple");
  
  static_assert(utils::is_tuple<typename MT::entity_types>::value,
                "mesh policy entity_types is not a tuple");


public:
  // used to find the entity type of topological dimension D and domain M
  template<size_t D, size_t M = 0>
  using entity_type = typename find_entity_<MT, D, M>::type;
  
  using sm_id_t        = size_t; 
  using sm_id_array_t = std::array<size_t, MT::num_dimensions>;  
  using sm_id_vector_2d_t  = std::vector<std::vector<size_t>>;

  // Don't allow the mesh to be copied or copy constructed
  structured_mesh_topology_t(const structured_mesh_topology_t &) = delete;
  structured_mesh_topology_t & operator=(const structured_mesh_topology_t &)
  = delete;

  // Allow move operations
  structured_mesh_topology_t(structured_mesh_topology_t && o) = default;
  structured_mesh_topology_t & operator=(structured_mesh_topology_t && o) 
  = default;

  //! Constructor
  structured_mesh_topology_t()
  {
      meshdim_ = MT::num_dimensions;  

      for (size_t i = 0; i < meshdim_; ++i)
      {
        meshbnds_low_[i] = MT::lower_bounds[i];
        meshbnds_up_[i]  = MT::upper_bounds[i];
      }

      //Bounds info
      std::vector<size_t> bnds_info[3][4] =
                           {{{{1}}, {{0}}, {{}}, {{}}},
                            {{{1,1}}, {{1,0,0,1}}, {{0,0}}, {{}}},
                            {{{1,1,1}}, {{1,0,1,0,1,1,1,1,0}}, 
                            {{1,0,0,0,1,0,0,0,1}}, {{0,0,0}}}};


      bool primary = false;
      for (size_t i = 0; i <= meshdim_; ++i)
      {
        if ( i == meshdim_) primary = true;
        std::cout<<"bnd_info = "<<bnds_info[meshdim_-1][i].size()<<std::endl;
        ms_.index_spaces[0][i].init(primary, meshbnds_low_, meshbnds_up_, bnds_info[meshdim_-1][i]);
      }

     //create query table once
     qt = new query::QueryTable<MT::num_dimensions, MT::num_dimensions+1,
                         MT::num_dimensions, MT::num_dimensions+1>(); 
     
     query::qtable(qt);  
  }

  // mesh destructor
  virtual ~structured_mesh_topology_t()
  {
    delete qt;
  }
 
 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
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
    return ms_.index_spaces[M][D].size();
  } // num_entities
 
 /******************************************************************************
 *                Representation Methods for Cartesian Block                   *
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Returns lower bounds of basic/cartesian block 
  *  IN: 
  *  OUT: 
  */
  auto lower_bounds()
  {
    return meshbnds_low_;
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Returns upper bounds of basic/cartesian block 
  *  IN: 
  *  OUT: 
  */
  auto upper_bounds()
  {
    return meshbnds_up_;
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Given an entity id, returns the cartesian box
  *                      the entity belongs. For intermediate, the value
  *                      could be non-zero. 
  *  IN: 
  *  OUT: 
  */
  template<
    size_t D,
    size_t M = 0 
  >
  auto get_box_id(sm_id_t entity_id)
  { 
    return ms_.index_spaces[M][D].find_box_id(entity_id);
  }

  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_box_id(E* e)
  {
    return ms_.index_spaces[M][D].find_box_id(e->id(0)); 
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Returns upper bounds of basic/cartesian topology 
  *  IN: 
  *  OUT: 
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto get_indices(sm_id_t entity_id) 
  {
    return ms_.index_spaces[M][D].get_indices_from_offset(entity_id);
  }
  
  template<
    size_t D,
    size_t M,
    class E
  >
  auto get_indices(E* e)
  {
    return ms_.index_spaces[M][D].get_indices_from_offset(e->id(0));
  }
  
 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Returns upper bounds of basic/cartesian topology 
  *  IN: 
  *  OUT: 
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto get_global_offset(size_t box_id, sm_id_array_t &idv) 
  {
    return ms_.index_spaces[M][D].template get_global_offset_from_indices(box_id, idv);
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
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
  auto get_local_offset(size_t box_id, sm_id_array_t &idv) 
  {
    return ms_.index_spaces[M][D].template get_local_offset_from_indices(box_id, idv);
  }
  

 /******************************************************************************
 *                      Query Methods for Cartesian Block                      *
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Provides traversal over the entire index space for a 
  *                      given dimension e.g., cells of the mesh.
  *  IN: 
  *  OUT: 
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto
  entities()
  {
    using etype = entity_type<D,M>;
    return ms_.index_spaces[M][D].template iterate<etype>(); 
  }
 // entities
 
 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
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
  entities() const
  {
    using etype = entity_type<D,M>;
    return ms_.index_spaces[M][D].template iterate<etype>();
  } // entities


 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Provides FEM-type adjacency queries. Given an entity
  *                      ,find all entities incident on it from dimension TD.
  *                      Supports queries between non-equal dimensions. The 
  *                      queries for entities from the same dimension can be 
  *                      obtained through the more finer level queries for 
  *                      intra-index space. 
  *  IN: 
  *  OUT: 
  */

  template<size_t TD, size_t FM , size_t TM = FM,  class E>
  auto entities(E* e)
  {
    size_t FD = E::dimension;
    assert(FD != TD);
    sm_id_t id = e->id(0);
    size_t BD = ms_.index_spaces[FM][FD].template find_box_id(id);
    auto indices = ms_.index_spaces[FM][FD].template 
                   get_indices_from_offset(id);

    using etype = entity_type<TD,TM>;
    return ms_.index_spaces[TM][TD].template 
           traverse<TD,etype>(FD, BD, indices, qt);
  } //entities

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
 //!
 //! @param dim    The dimension of the entity for which the total number is 
 //!               requested
 //! @param domain The domain of the entity for which the total number is 
 //!               requested.
 //--------------------------------------------------------------------------//
  /* Method Description: Provides FD-type adjacency queries. Given an entity
  *                      ,find the entity using the stencil provided..
  *  IN: 
  *  OUT: 
  */
  
  template<
  std::intmax_t xoff, 
  size_t FM,  
  class E>
  auto stencil_entity(E* e)
  {
    assert(!(xoff == 0) ); 
    size_t FD = E::dimension;
    size_t value = e->id(0);
    size_t BD = ms_.index_spaces[FM][FD].template find_box_id(value);
    auto indices = ms_.index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if(ms_.index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0]))
    {
      value += xoff;
    }
    return value; 
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
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
    size_t BD = ms_.index_spaces[FM][FD].template find_box_id(value);
    auto indices = ms_.index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if((ms_.index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0]))&& 
       (ms_.index_spaces[FM][FD].template 
       check_index_limits<1>(BD,yoff+indices[1])))
    {
      size_t nx = ms_.index_spaces[FM][FD].template 
                  get_size_in_direction<0>(BD);
      value += xoff + nx*yoff;
    }
    return value; 
  }

 //--------------------------------------------------------------------------//
 //! Return the number of entities contained in specified topological 
 //! dimension and domain.
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
    size_t BD = ms_.index_spaces[FM][FD].template find_box_id(value);
    auto indices = ms_.index_spaces[FM][FD].template
                   get_indices_from_offset(value);
  
    if((ms_.index_spaces[FM][FD].template 
       check_index_limits<0>(BD,xoff+indices[0])) && 
       (ms_.index_spaces[FM][FD].template 
       check_index_limits<1>(BD,yoff+indices[1])) &&
       (ms_.index_spaces[FM][FD].template
       check_index_limits<2>(BD,zoff+indices[2])))
    {
      size_t nx = ms_.index_spaces[FM][FD].template 
                  get_size_in_direction<0>(BD);
      size_t ny = ms_.index_spaces[FM][FD].template 
                  get_size_in_direction<1>(BD);
      value += xoff + nx*yoff + nx*ny*zoff;
    }
    return value; 
  }

private:

  size_t meshdim_; 
  sm_id_array_t meshbnds_low_;
  sm_id_array_t meshbnds_up_;
  structured_mesh_storage_t<MT::num_dimensions, MT::num_domains> ms_;

  query::QueryTable<MT::num_dimensions, MT::num_dimensions+1, 
                    MT::num_dimensions, MT::num_dimensions+1>  *qt; 

  // Get the number of entities in a given domain and topological dimension
  size_t
  num_entities_(
    size_t dim,
    size_t domain=0
  ) const
  {
    return ms_.index_spaces[domain][dim].size();
  } // num_entities_

}; // class structured_mesh_topology_t

} // namespace topology
} // namespace flecsi

