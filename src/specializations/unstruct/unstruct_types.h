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

///////////////////////////////////////////////////////////////////////////////
//! \file unstruct_types.h
//!
//! \brief This file includes the main type declarations required by the
//!        mesh_topology class.
///////////////////////////////////////////////////////////////////////////////

// include guard
#pragma once

// includes
#include "flexi/geometry/point.h"
#include "flexi/geometry/space_vector.h"
#include "flexi/state/state.h"
#include "flexi/utils/bitfield.h"
#include "flexi/utils/common.h"

namespace flexi {

//=============================================================================
//! \class unstruct_mesh_traits_t
//!
//! \brief the main mesh traits
//!
//! You only need to specify the number of dimensions and real type
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename T, int D> struct unstruct_mesh_traits_t {

  //! \brief the number of dimensions
  static constexpr size_t dimension = D;

  //! \brief we are using double precision
  using real_t = T;

  //! \brief the different attachment sites
  enum class attachment_site_t : size_t {
    vertices,
    edges,
    faces,
    cells
  };

  //! \brief state attributes
  enum class state_attribute_t : bitfield_t::field_type_t {
    persistent = 0
  };


  //---------------------------------------------------------------------------
  //! \brief private meta data to use for create fields
  //---------------------------------------------------------------------------
  struct private_state_meta_data_t {

    //! \brief initialize the field with attachment site and attributes
    //! \param [in] site_ the attachement site
    //! \param [in] attributes_ a bit field that defines the attributes
    void initialize( attachment_site_t site_,
                     bitfield_t::field_type_t attributes_ ) {
      site = site_;
      attributes = attributes_;
    }

    //! the attachment site
    attachment_site_t site;

    //! the attributes
    bitfield_t::field_type_t attributes;

  };
  
  //! define a storage policy
#ifndef MESH_STORAGE_POLICY
  using mesh_state_t = state_t<private_state_meta_data_t>;
#else
  using mesh_state_t =
    state_t<private_state_meta_data_t, MESH_STORAGE_POLICY>;
#endif


};

//=============================================================================
//! \class cell_t unstruct_types.h
//! \brief The cell_t type provides an interface for managing and
//!        geometry and state associated with mesh cells.
//=============================================================================
template<int D>
class unstruct_cell_t : public mesh_entity<D> {
public :

  //! \brief set the precedence of the dimension
  //! \param [in] dim the dimension in question
  //! \param [in] precedence the precedence to set
  void set_precedence(size_t dim, uint64_t precedence) 
  {}
  
  //! \brief create the entities from the list of vertex ids
  //! \param [in] dim ???
  //! \param [in] e ???
  //! \param [in] v the list of vertex ids
  //! \param [in] vertex_count The number of vertices
  std::pair<size_t,size_t> create_entities( size_t dim, 
                                            std::vector<flexi::id_t> &e, 
                                            id_t *v, 
                                            size_t vertex_count) 
  {
    assert( false && " this needs specialization" );
  }
  
};



//=============================================================================
//! \brief create the entities from the list of vertex ids
//! \param [in] dim ???
//! \param [in] e ???
//! \param [in] v the list of vertex ids
//! \param [in] vertex_count The number of vertices
//!
//! \remark this is a 2d specialization
//=============================================================================
template<>
std::pair<size_t,size_t> 
 unstruct_cell_t<2>::create_entities( size_t dim, 
                                      std::vector<flexi::id_t> &e, 
                                      id_t *v, 
                                      size_t vertex_count) 
{
  e.resize(8);
  
  e[0] = v[0];
  e[1] = v[1];
  
  e[2] = v[2];
  e[3] = v[3];
  
  e[4] = v[0];
  e[5] = v[3];
  
  e[6] = v[1];
  e[7] = v[2];
  
  return {4, 2};
}

//=============================================================================
//! \brief create the entities from the list of vertex ids
//! \param [in] dim ???
//! \param [in] e ???
//! \param [in] v the list of vertex ids
//! \param [in] vertex_count The number of vertices
//!
//! \remark this is a 2d specialization
//=============================================================================
template<>
std::pair<size_t,size_t> 
 unstruct_cell_t<3>::create_entities( size_t dim, 
                                      std::vector<flexi::id_t> &e, 
                                      id_t *v, 
                                      size_t vertex_count) 
{

  // want edges
  if ( dim == 1 ) {
  
    e.resize(12*2);
    
    e[ 0] = v[0];
    e[ 1] = v[1];
    
    e[ 2] = v[1];
    e[ 3] = v[2];
    
    e[ 4] = v[2];
    e[ 5] = v[3];
    
    e[ 6] = v[3];
    e[ 7] = v[0];
    
    e[ 8] = v[4];
    e[ 9] = v[5];
    
    e[10] = v[5];
    e[11] = v[6];
    
    e[12] = v[6];
    e[13] = v[7];
    
    e[14] = v[7];
    e[15] = v[5];
    
    e[16] = v[0];
    e[17] = v[4];
    
    e[18] = v[1];
    e[19] = v[5];
    
    e[20] = v[2];
    e[21] = v[6];
    
    e[22] = v[3];
    e[23] = v[7];
    
    return {12, 2};
  }
  // want faces
  else if ( dim == 2 ) {
  
    e.resize(6*4);
    
    e[ 0] = v[ 0];
    e[ 1] = v[ 1];
    e[ 2] = v[ 4];
    e[ 3] = v[ 5];
    
    e[ 4] = v[ 1];
    e[ 5] = v[ 2];
    e[ 6] = v[ 6];
    e[ 7] = v[ 5];
    
    e[ 8] = v[ 2];
    e[ 9] = v[ 3];
    e[10] = v[ 7];
    e[11] = v[ 6];
    
    e[12] = v[3];
    e[13] = v[0];
    e[14] = v[4];
    e[15] = v[7];
    
    e[16] = v[0];
    e[17] = v[3];
    e[18] = v[2];
    e[19] = v[1];

    e[20] = v[4];
    e[21] = v[5];
    e[22] = v[6];
    e[23] = v[7];
        
    return {6, 4};
  }

}


  

//=============================================================================
//! \class unstruct_mesh_types_t
//!
//! \brief A collection of the type information needed to specialize the flexi
//!        low-level mesh infastructure.
//!
//! This specialization is for a general multi-dimensional unstructured mesh.
//!
//! \tparam D the number of dimensions
//! \tparam T the real type
//=============================================================================
template <typename T, int D> 
struct unstruct_mesh_types_t {

  //---------------------------------------------------------------------------
  // Public Types
  //---------------------------------------------------------------------------

  //! \brief the mesh traits
  using traits_t = unstruct_mesh_traits_t<T, D>;

  //! \brief the number of dimensions
  static constexpr auto dimension = traits_t::dimension;

  //! \brief the real type
  using real_t = typename traits_t::real_t;

  //! \brief the point type depends on dimensions and precision
  using point_t = point<real_t, dimension>;

  //! \brief a spacial vector
  using vector_t = space_vector<real_t, dimension>;

  //! \brief there is no special attachment sites
  static constexpr size_t num_domains = 1;

  //---------------------------------------------------------------------------
  //! \class unstruct_vertex_t
  //! \brief The vertex_t type provides an interface for managing and
  //!        geometry and state associated with mesh vertices.
  //---------------------------------------------------------------------------
  class vertex_t : public mesh_entity<0> {
  public:

    //! alias the state type
    using state_t = typename traits_t::mesh_state_t;

    //! \brief default constructor
    vertex_t() : precedence_(0) {}

    //! \brief Constructor with vertex
    //! \param[in] coordinates the coordinates to set
    //! \param[in] state the state model pointer for storage
    vertex_t(const point_t &coordinates, 
                      state_t * state) 
      : precedence_(0), coordinates_(coordinates) 
    {}

    //! \brief return the vertex precedence
    uint64_t precedence() const { return 1 << (63 - info()); }

    //! \brief set the coordinates
    //! \param[in] coordinates the coordinates to set
    void set_coordinates(const point_t &coordinates) {
      auto c = state_->template accessor<point_t>("coordinates");
      coordinates_ = coordinates;
    }

    //! \brief extract the coordinates
    const auto & get_coordinates() const { return coordinates_; }

  private:
    //! unused precedence
    uint64_t precedence_;
    //! the coordinates of this vertex
    point_t coordinates_;
    //! the state model pointer
    state_t * state_;
  };

  //---------------------------------------------------------------------------
  //! \class edge_t
  //! \brief The edge_t type provides an interface for managing and
  //!        geometry and state associated with mesh edges.
  //---------------------------------------------------------------------------
  struct edge_t : public mesh_entity<1> {};

  //---------------------------------------------------------------------------
  //! \class face_t unstruct_types.h
  //! \brief The face_t type provides an interface for managing and
  //!        geometry and state associated with mesh faces.
  //---------------------------------------------------------------------------
  struct face_t : public mesh_entity<D-1> {};

  //---------------------------------------------------------------------------
  //! \class cell_t unstruct_types.h
  //! \brief The cell_t type provides an interface for managing and
  //!        geometry and state associated with mesh cells.
  //---------------------------------------------------------------------------
  using cell_t = unstruct_cell_t<D>;
  

  //---------------------------------------------------------------------------
  //! \brief Specify mesh parameterizations.
  //---------------------------------------------------------------------------

  using entity_types =
       std::tuple<std::pair<domain_<0>, vertex_t>,
                  std::pair<domain_<0>, edge_t>,
                  std::pair<domain_<0>, face_t>,
                  std::pair<domain_<0>, cell_t>>;

  using connectivities = 
    std::tuple<std::tuple<domain_<0>, vertex_t, edge_t>,
               std::tuple<domain_<0>, vertex_t, cell_t>,
               std::tuple<domain_<0>, edge_t,   vertex_t>,
               std::tuple<domain_<0>, edge_t,   cell_t>,
               std::tuple<domain_<0>, cell_t,   vertex_t>,
               std::tuple<domain_<0>, cell_t,   edge_t>>;
};


} // namespace flexi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
