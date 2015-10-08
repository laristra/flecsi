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

#ifndef flexi_burton_types_h
#define flexi_burton_types_h

#include "../utils/common.h"
#include "../mesh/mesh_topology.h"
#include "../geometry/point.h"
#include "../geometry/space_vector.h"

/*!
 * \file burton_types.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

struct burton_mesh_traits_t {
  static constexpr size_t dimension = 2;

  using real_t = double;

}; // struct burton_mesh_traits_t

/*!---------------------------------------------------------------------------*
 * \class burton_mesh_types_t burton_types.h
 * \brief The burton_mesh_types_t is a collection of type information needed
 *        to specialize the flexi low-level mesh infrastructure for
 *        ALE methods.
 *----------------------------------------------------------------------------*/

struct burton_mesh_types_t {

  static constexpr size_t dimension = burton_mesh_traits_t::dimension;
  using real_t = burton_mesh_traits_t::real_t;

  using point_t = point<real_t, dimension>;
  using vector_t = space_vector<real_t, dimension>;

  /*!-------------------------------------------------------------------------*
   * \class burton_vertex_t burton_types.h
   * \brief The burton_vertex_t type provides an interface for managing and
   *        geometry and state associated with mesh vertices.
   *--------------------------------------------------------------------------*/

  class burton_vertex_t : public MeshEntity<0> {
  public:

    //! Constructor
    burton_vertex_t()
      : precedence_(0) {}

    //! Constructor
    burton_vertex_t(const point_t& coordinates)
      : precedence_(0), coordinates_(coordinates) {}

    void setRank(uint8_t rank){
      precedence_ = 1 << (63 - rank);
    }

    uint64_t precedence() const{
      return precedence_;
    }
    
    void setCoordinates(const point_t& coordinates){
      coordinates_ = coordinates;
    }

    const point_t& coordinates() const{
      return coordinates_;
    }
        
  private:

    uint64_t precedence_;
    point_t coordinates_;

  }; // struct burton_vertex_t

  /*!-------------------------------------------------------------------------*
   * \class burton_edge_t burton_types.h
   * \brief The burton_edge_t type provides an interface for managing and
   *        geometry and state associated with mesh edges.
   *--------------------------------------------------------------------------*/

  struct burton_edge_t : public MeshEntity<1> {
  }; // struct burton_edge_t

  class burton_corner_t;
  class burton_wedge_t;

  /*!-------------------------------------------------------------------------*
   * \class burton_cell_t burton_types.h
   * \brief The burton_cell_t type provides an interface for managing and
   *        geometry and state associated with mesh cells.
   *--------------------------------------------------------------------------*/

  class burton_cell_t : public MeshEntity<2>
  {
  public:

    void addCorner(burton_corner_t* c){
      corners_.add(c);
    }

    EntityGroup<burton_corner_t> & corners() {
      return corners_;
    } // getSides

    void addWedge(burton_wedge_t* w){
      wedges_.add(w);
    }

    EntityGroup<burton_wedge_t> & wedges() {
      return wedges_;
    } // getSides

  private:

    EntityGroup<burton_corner_t> corners_;
    EntityGroup<burton_wedge_t> wedges_;

  }; // class burton_cell_t

  /*!-------------------------------------------------------------------------*
   * \class burton_wedge_t burton_types.h
   * \brief The burton_wedge_t type provides an interface for managing and
   *        geometry and state associated with mesh wedges.
   *--------------------------------------------------------------------------*/

  class burton_wedge_t : public MeshEntity<2>
  {
  public:

    void setCorner(burton_corner_t* corner){
      corner_ = corner;
    }

    burton_corner_t* corner(){
      return corner_;
    }

    vector_t side_facet_normal();
    vector_t cell_facet_normal();

  private:

    burton_corner_t* corner_;

  }; // struct burton_wedge_t

  /*!-------------------------------------------------------------------------*
   * \class burton_corner_t burton_types.h
   * \brief The burton_corner_t type provides an interface for managing and
   *        geometry and state associated with mesh corners.
   *--------------------------------------------------------------------------*/

  class burton_corner_t : public MeshEntity<0> {
  public:

    void addWedge(burton_wedge_t* w){
      wedges_.add(w);
      w->setCorner(this);
    }

    EntityGroup<burton_wedge_t> & wedges() {
      return wedges_;
    } // wedges

  private:

    EntityGroup<burton_wedge_t> wedges_;

  }; // class burton_corner_t

  /*!-------------------------------------------------------------------------*
   * Specify mesh parameterizations.
   *--------------------------------------------------------------------------*/

  using EntityTypes =
    std::tuple<burton_vertex_t, burton_edge_t, burton_cell_t>;

  /*!-------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t numEntitiesPerCell(size_t dim) {
    switch(dim){
      case 0:
        return 4;
      case 1:
        return 4;
      case 2:
        return 1;
      default:
        assert(false && "invalid dimension");
    } // switch
  } // numEntitiesPerCell

  /*!-------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static constexpr size_t verticesPerCell() {
    return 4;
  } // verticesPerCell

  /*!-------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static size_t numVerticesPerEntity(size_t dim) {
    switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 4;
      default:
        assert(false && "iznvalid dimension");
    } // switch
  } // numVerticesPerEntity

  /*!-------------------------------------------------------------------------*
   * FIXME
   *--------------------------------------------------------------------------*/

  static void createEntities(size_t dim, std::vector<id_t>& e,
    burton_vertex_t** v) {
    assert(dim = 1);
    assert(e.size() == 8);

    struct Edge_{
      Edge_(burton_vertex_t* v1, burton_vertex_t* v2)
        : v1(v1), v2(v2){}
            
      burton_vertex_t* v1;
      burton_vertex_t* v2;

      uint64_t precedence() const{
        return v1->precedence() | v2->precedence(); 
      }
    };

    std::vector<Edge_> es;
    es.emplace_back(Edge_(v[0], v[1]));
    es.emplace_back(Edge_(v[1], v[3]));
    es.emplace_back(Edge_(v[0], v[1]));
    es.emplace_back(Edge_(v[2], v[3]));

    std::sort(es.begin(), es.end(), [](const Edge_& e1, const Edge_& e2){
      return e1.precedence() > e2.precedence();
    }); 

    e[0] = es[0].v1->id();
    e[1] = es[0].v2->id();

    e[2] = es[1].v1->id();
    e[3] = es[1].v2->id();

    e[4] = es[2].v1->id();
    e[5] = es[2].v2->id();    

    e[6] = es[3].v1->id();
    e[7] = es[3].v2->id();
  } // createEntities

}; // struct burton_mesh_types_t

// FIXME
 class burton_dual_mesh_types_t {
  public:
    static constexpr size_t dimension = burton_mesh_traits_t::dimension;

    using real_t = burton_mesh_traits_t::real_t;

    using point_t = point<real_t, dimension>;

    using burton_vertex_t = burton_mesh_types_t::burton_vertex_t;

    using burton_edge_t = burton_mesh_types_t::burton_edge_t;

    using burton_wedge_t = burton_mesh_types_t::burton_wedge_t;
  
    using EntityTypes =
      std::tuple<burton_vertex_t, burton_edge_t, burton_wedge_t>;

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 3;
      case 1:
        return 3;
      case 2:
        return 1;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 4;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 3;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<id_t>& e,
      burton_vertex_t** v){
      assert(dim = 1);
      assert(e.size() == 6);
    
      e[0] = v[0]->id();
      e[1] = v[2]->id();
    
      e[2] = v[1]->id();
      e[3] = v[3]->id();
    
      e[4] = v[0]->id();
      e[5] = v[1]->id();
    }
  }; // burton_dual_mesh_t

} // namespace flexi

#endif // flexi_burton_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
