/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_burton_mesh_traits_h
#define flexi_burton_mesh_traits_h

#include "flexi/utils/bitfield.h"
#include "flexi/state/state.h"
#include "flexi/geometry/point.h"
#include "flexi/geometry/space_vector.h"

/*!
 * \file burton_mesh_traits.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flexi {

struct burton_mesh_traits_t {

/*!
  Compile-time selection of mesh dimension.  Valid selections are 2 or 3.
 */
#ifndef FLEXI_MESH_DIMENSION
#define FLEXI_MESH_DIMENSION 2
#endif // FLEXI_MESH_DIMENSION

  //! The dimension of the burton mesh
  static constexpr size_t dimension = FLEXI_MESH_DIMENSION;

  //! The number of mesh domains in the burton mesh
  static constexpr size_t num_domains = 2;

  //! The type for floating-point values
  using real_t = double;

  //! A point type with real_t data and mesh dimension
  using point_t = point<real_t, dimension>;

  //! A space_vector type of real_t data and mesh dimension
  using vector_t = space_vector<real_t, dimension>;

  //! Enumeration of the locations on the mesh where data may be sited
  enum class attachment_site_t : size_t {
    vertices,
#if FLEXI_MESH_DIMENSION >= 2
    edges,
#endif
#if FLEXI_MESH_DIMENSION == 3
    faces,
#endif
    cells,
    corners
  }; // enum class attachment_site_t

  //! Enumeration of the available attributes on state data
  enum class state_attribute_t : bitfield_t::field_type_t {
    persistent = 0,
    temporary = 1 // just a placeholder for now
  }; // enum class state_attribute_t

  /*--------------------------------------------------------------------------*
   * State type definitions
   *--------------------------------------------------------------------------*/

  /*!
    \brief Struct to hold meta data for private state. Meta data includes site
      and attributes.
   */
  struct private_state_meta_data_t {

    void initialize(attachment_site_t site_,
      bitfield_t::field_type_t attributes_) {
      site = site_;
      attributes = attributes_;
    } // initialize

    attachment_site_t site;
    bitfield_t::field_type_t attributes;

  }; // struct private_state_meta_data_t
  
  //! A type definition of state_t based on the storage policy for the mesh
#ifndef MESH_STORAGE_POLICY
  using mesh_state_t = state_t<private_state_meta_data_t>;
#else
  using mesh_state_t =
    state_t<private_state_meta_data_t, MESH_STORAGE_POLICY>;
#endif

}; // struct burton_mesh_traits_t

} // namespace flexi

#endif // flexi_burton_mesh_traits_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
