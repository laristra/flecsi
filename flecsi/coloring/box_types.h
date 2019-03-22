/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef box_types_h
#define box_types_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//
#include <array>
#include <bitset>
#include <cmath>
#include <iostream>
#include <set> 
#include <unordered_map>
#include <vector>

namespace flecsi {
namespace coloring {

/*!
   Type for specifying bounds of a box
 */

struct box_t {
  size_t box_dim; 
  size_t box_size = 0;  
  std::vector<size_t> lowerbnd;
  std::vector<size_t> upperbnd;
 
  box_t(){};   
  box_t(size_t dim)
  {
    box_dim = dim; 
    lowerbnd.resize(box_dim,0);
    upperbnd.resize(box_dim,0);
  }
 
  size_t dim()
  {
    return box_dim;
  } 

  size_t size()
  {
    size_t count = 1;
    for (size_t i = 0; i < box_dim; i++)
     count *= upperbnd[i]-lowerbnd[i]+1;
    box_size = count; 
    return box_size;
  }

  bool isempty()
  {
    return (lowerbnd.empty() && upperbnd.empty()); 
  }
 
  void resize(size_t dim)
  {
    box_dim = dim; 
    lowerbnd.resize(box_dim,0);
    upperbnd.resize(box_dim,0);
  }
}; // class box_t

/*!
   Type for a tagged box: this allows tagging the region 
   boundaries of the box to specify if they satisfy a certain
   condition or not.
   The size of the tag array is fixed at 26, which is the maximum
   number of region boundaries(faces, edges, vertices) of a box in 3D. 
   Making this a vector will need  properly setting the size of the 
   vector depending on the dimension of the mesh.
 */
struct box_tag_t {
  box_t box;
  std::vector<bool> tag{std::vector<bool>(26,true)};
}; // class box_tag_t

/*!
   Type for a colored box
 */
struct box_color_t {
  box_tag_t domain;
  std::vector<size_t> colors;
}; // class box_color_t

/*!
   Type to specify primary box info
 */
struct box_info_t {
  box_t box;
  size_t nhalo;
  size_t nhalo_domain;
  size_t thru_dim;
  std::vector<bool> onbnd; 
  //std::vector<size_t> strides;
}; // class box_info_t

/*!
   Type for collecting aggregate of various colored boxes
 */
struct box_coloring_t  
{
  // The primary flag is true if the partition is on the index-space 
  // of the primary entity. Ex. the cells or the vertices of the mesh.
  bool primary = false;       
  size_t primary_dim = 0; 
  size_t num_boxes = 1; 
 
  //! The box info for partitioned box
  std::vector<box_info_t> partition;

  //! The exclusive box owned by current rank
  std::vector<box_color_t> exclusive;

  //! The aggregate of shared boxes
  std::vector<std::vector<box_color_t>> shared;

  //! The aggregate of ghost boxes
  std::vector<std::vector<box_color_t>> ghost;

  //! The aggregate of domain-halo boxes
  std::vector<std::vector<box_tag_t>> domain_halo;
  
  //! The bounding box covering exclusive+shared+ghost+domain-halo boxes
  std::vector<box_t> overlay;
  std::vector<std::vector<size_t>> strides;

  //! Resize 
  void resize()
  {
    partition.resize(num_boxes);
    exclusive.resize(num_boxes);
    shared.resize(num_boxes);
    ghost.resize(num_boxes);
    domain_halo.resize(num_boxes);
    overlay.resize(num_boxes);
  } //resize

  void resize(size_t dim)
  {
    // resize the outer bounds first
    partition.resize(num_boxes);
    exclusive.resize(num_boxes);
    shared.resize(num_boxes);
    ghost.resize(num_boxes);
    domain_halo.resize(num_boxes);
    overlay.resize(num_boxes);
    strides.resize(num_boxes); 

    // resize the inner vectors for ghost and shared
    size_t sz = pow(3,dim);
    for (size_t i = 0; i < num_boxes; i++){ 
      ghost[i].resize(sz);
      shared[i].resize(sz);
      strides[i].resize(dim); 
    }
  }

}; // class box_coloring_t

struct box_aggregate_info_t
{
  //! The number of exclusive indices.
  size_t exclusive;

  //! The number of shared indices.
  size_t shared;

  //! The number of ghost indices.
  size_t ghost;

  //! The aggregate set of colors that depend on our shared indices
  std::set<size_t> shared_users;
 
  //! The aggregate set of colors that we depend on for ghosts
  std::set<size_t> ghost_owners;

  //! The overlay boxes from ghost owners
  std::unordered_map<size_t, std::vector<box_t>> ghost_overlays; 

}; // class box_aggregate_info_t

} // namespace coloring
} // namespace flecsi

#endif // box_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
