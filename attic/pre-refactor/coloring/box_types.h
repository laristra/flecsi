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

template<size_t D>
struct box_t {
  size_t lowerbnd[D];
  size_t upperbnd[D];
}; // class box_t

/*!
   Type for a colored box
 */
template<size_t D>
struct box_color_t {
  box_t<D> box;
  std::vector<size_t> colors;

}; // class box_color_t

/*!
   Type to specify primary box info
 */
template<size_t D>
struct box_info_t {
  box_t<D> box;
  size_t nhalo;
  size_t nhalo_domain;
  size_t thru_dim;
  std::bitset<D * 2> onbnd;
}; // class box_info_t

/*!
   Type for collecting aggregate of various colored boxes
 */
template<size_t D>
struct box_coloring_info_t {
  //! The primary box
  box_info_t<D> primary;

  //! The exclusive box owned by current rank
  box_color_t<D> exclusive;

  //! The aggregate of shared boxes
  std::vector<box_color_t<D>> shared;

  //! The aggregate of ghost boxes
  std::vector<box_color_t<D>> ghost;

  //! The aggregate of domain-halo boxes
  std::vector<box_t<D>> domain_halo;
}; // class box_coloring_info_t

} // namespace coloring
} // namespace flecsi

#endif // box_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
