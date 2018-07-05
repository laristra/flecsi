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
#include <vector>


namespace flecsi {
namespace coloring {

/*!
   Type for specifying bounds of a box
 */

struct box_t {
  size_t dim_; 
  std::vector<size_t> lowerbnd;
  std::vector<size_t> upperbnd;

  box_t(){};   
  box_t(size_t dim)
  {
    dim_ = dim; 
    lowerbnd.resize(dim_,0);
    upperbnd.resize(dim_,0);
  }

  size_t dim()
  {
    return dim_;
  } 

  size_t size()
  {
    size_t count = 1;
    for (size_t i = 0; i < dim_; i++)
     count *= upperbnd[i]-lowerbnd[i]+1;
    return count;
  }

}; // class box_t

/*!
   Type for a colored box
 */
struct box_color_t {
  box_t box;
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
  //std::bitset<D * 2> onbnd;
}; // class box_info_t

/*!
   Type for collecting aggregate of various colored boxes
 */
struct box_coloring_t  
{
  //! The primary box
  box_info_t primary;

  //! The exclusive box owned by current rank
  box_color_t exclusive;

  //! The aggregate of shared boxes
  std::vector<box_color_t> shared;

  //! The aggregate of ghost boxes
  std::vector<box_color_t> ghost;

  //! The aggregate of domain-halo boxes
  std::vector<box_t> domain_halo;
}; // class box_coloring_info_t


} // namespace coloring
} // namespace flecsi

#endif // box_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
