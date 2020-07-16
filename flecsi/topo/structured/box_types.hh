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
namespace topology { 
namespace structured_impl {

/*!
   Type for specifying bounds of a box
 */

struct box_t {
  size_t dim; 
  size_t bsize = 0;  
  std::vector<size_t> lowerbnd;
  std::vector<size_t> upperbnd;
 
  box_t(){};   
  box_t(size_t in_dim)
  {
    dim = in_dim; 
    lowerbnd.resize(dim,0);
    upperbnd.resize(dim,0);
  }
 
  size_t dimension()
  {
    return dim;
  }
 
  size_t size()
  {
    if (bsize == 0)
     bsize = compute_size(); 
    return bsize;
  }

  size_t compute_size()
  {
    size_t count = 0; 
    if ((lowerbnd.size() != 0 ) && (upperbnd.size() != 0)) {
      count = 1;
      for (size_t i = 0; i < dim; i++)
        count *= upperbnd[i]-lowerbnd[i]+1;
    }
    return count;
  }

  bool isempty()
  {
    return (compute_size() == 0); 
  }
 
  void resize(size_t in_dim)
  {
    dim = in_dim; 
    lowerbnd.resize(dim,0);
    upperbnd.resize(dim,0);
  }
}; // class box_t

/*!
   Type for a colored box:
   domain: a box_t type specifying bounds 
   colors: used to represent ranks this is communicated with
   tag: tag boundaries of the box as false if it doesn't satisfy 
        the criterion this box is supposed to satisfy. For example,
        when this type is used to represent an exclusive box, 
        the boundary entities which are not exclusive are tagged 
        as false. 
 */
struct box_color_t {

  box_color_t () {}; 
  box_color_t(int dim) 
  {  
    domain.resize(dim);
    int nbid = pow(3, dim); 
    tag.resize(nbid,true);  
  }

  box_t domain;
  std::vector<size_t> colors;
  std::vector<bool> tag;
}; // class box_color_t

/*!
   Type to specify primary box info
 */
struct box_info_t {
  box_t box;
  size_t nghost_layers;
  size_t ndomain_layers;
  size_t thru_dim;
  std::vector<bool> onbnd; 
}; // class box_info_t

/*!
   Type for collecting aggregate of various colored boxes
 */
struct box_coloring_t  
{
  // The primary flag is true if the partition is on the index-space 
  // of the primary entity. Ex. the cells or the vertices of the mesh.
  size_t mesh_dim = 0; 
  size_t entity_dim = 0; 
  size_t primary_dim = 0; 
  size_t num_boxes = 0; 
 
  //! The box info for partitioned box
  std::vector<box_info_t> partition;

  //! The exclusive box owned by current rank
  std::vector<box_color_t> exclusive;

  //! The aggregate of shared boxes
  std::vector<std::vector<box_color_t>> shared;

  //! The aggregate of ghost boxes
  std::vector<std::vector<box_color_t>> ghost;

  //! The aggregate of domain-halo boxes
  std::vector<std::vector<box_color_t>> domain_halo;
  
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
    strides.resize(num_boxes);  
  } //resize

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

} // namespace structured_impl
} // namespace topology
} // namespace flecsi

#endif // box_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
