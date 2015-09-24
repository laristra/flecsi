/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_burton_h
#define flexi_burton_h

#include "../mesh/mesh_topology.h"
#include "../geometry/point.h"

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

  class burton_mesh_types_t{
  public:
    static constexpr size_t dimension = 2;

    using Id = uint64_t;

    using point_t = point<double, dimension>;

    // Vertex type
    struct burton_vertex_t : public MeshEntity {
      static const size_t dimension = 0;

      burton_vertex_t(size_t id) : MeshEntity(id) {}

      burton_vertex_t(size_t id, const point_t& pos) :
        MeshEntity(id),
        pos(pos){}
      
      point_t pos;
    }; // struct burton_vertex_t

    // Edge type
    struct burton_edge_t : public MeshEntity {
      static const size_t dimension = 1;

      burton_edge_t(size_t id) : MeshEntity(id) {}
    }; // struct burton_edge_t

    // Cell type
    class burton_cell_t : public MeshEntity {
    public:
      static const size_t dimension = 2;

      burton_cell_t(size_t id) : MeshEntity(id) {}

      EntityGroup<0> & corners() {
        return corners_;
      } // getSides

      EntityGroup<2> & wedges() {
        return wedges_;
      } // getSides

    private:

      EntityGroup<0> corners_;
      EntityGroup<2> wedges_;

    }; // class burton_cell_t
    
    class burton_corner_t;

    // Wedge type
    class burton_wedge_t : public MeshEntity {
      static const size_t dimension = 2;

      burton_wedge_t(size_t id) : MeshEntity(id) {}

    private:
      burton_corner_t* corner_;
    }; // struct burton_wedge_t

    // Corner type
    class burton_corner_t : public MeshEntity {
    public:
      static const size_t dimension = 0;

      burton_corner_t(size_t id) : MeshEntity(id) {}

      EntityGroup<2> & wedges() {
        return wedges_;
      } // wedges

    private:

      EntityGroup<2> wedges_;

    }; // class burton_corner_t

    using EntityTypes =
      std::tuple<burton_vertex_t, burton_edge_t, burton_cell_t>;

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 4;
      case 1:
        return 4;
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
        return 4;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim,
                               std::vector<Id>& e,
                               burton_vertex_t** v){
      assert(dim = 1);
      assert(e.size() == 8);
    
      e[0] = v[0]->id();
      e[1] = v[2]->id();
    
      e[2] = v[1]->id();
      e[3] = v[3]->id();
    
      e[4] = v[0]->id();
      e[5] = v[1]->id();
    
      e[6] = v[2]->id();
      e[7] = v[3]->id();
    }
  };

 class dual_mesh_types_t{
  public:
    static constexpr size_t dimension = burton_mesh_types_t::dimension;

    using Id = uint64_t;

    using point_t = point<double, burton_mesh_types_t::dimension>;

    using burton_vertex_t = burton_mesh_types_t::burton_vertex_t;

    using burton_edge_t = burton_mesh_types_t::burton_edge_t;

    // Cell type
    class burton_wedge_t : public MeshEntity {
    public:

      burton_wedge_t(size_t id) : MeshEntity(id) {}

      EntityGroup<2> & getSides() {
        return sides_;
      } // getSides

    private:

      EntityGroup<2> sides_;

    }; // struct burton_cell_t


    using EntityTypes =
      std::tuple<burton_vertex_t, burton_edge_t, burton_wedge_t>;

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 4;
      case 1:
        return 4;
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
        return 4;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim,
                               std::vector<Id>& e,
                               burton_vertex_t** v){
      assert(dim = 1);
      assert(e.size() == 8);
    
      e[0] = v[0]->id();
      e[1] = v[2]->id();
    
      e[2] = v[1]->id();
      e[3] = v[3]->id();
    
      e[4] = v[0]->id();
      e[5] = v[1]->id();
    
      e[6] = v[2]->id();
      e[7] = v[3]->id();
    }
  };


/*!
  \class burton_mesh_t burton.h
  \brief burton_mesh_t provides...
 */
class burton_mesh_t
{
private:

  using private_mesh_t = Mesh<burton_mesh_types_t>;
  using private_dual_mesh_t = Mesh<dual_mesh_types_t>;

public:

  using point_t = point<double, burton_mesh_types_t::dimension>;

  using vertex_t = burton_mesh_types_t::burton_vertex_t;
  using edge_t = burton_mesh_types_t::burton_edge_t;
  using cell_t = burton_mesh_types_t::burton_cell_t;
  using wedge_t = burton_mesh_types_t::burton_wedge_t;
  using corner_t = burton_mesh_types_t::burton_corner_t;

  // Iterator types
  using vertex_iterator_t = private_mesh_t::VertexIterator;
  using edge_iterator_t = private_mesh_t::EdgeIterator;
  using cell_iterator_t = private_mesh_t::CellIterator;

  using wedges_at_corner_iterator_t = private_dual_mesh_t::CellIterator;
  using wedges_at_face_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_vertex_iterator_t = private_mesh_t::CellIterator;
  using wedges_at_cell_iterator_t = private_mesh_t::CellIterator;

  //! Default constructor
  burton_mesh_t() {}

  //! Copy constructor (disabled)
  burton_mesh_t(const burton_mesh_t &) = delete;

  //! Assignment operator (disabled)
  burton_mesh_t & operator = (const burton_mesh_t &) = delete;

  //! Destructor
  ~burton_mesh_t() {}

  /*--------------------------------------------------------------------------*
   * Primary mesh iterators
   *--------------------------------------------------------------------------*/

  //! Vertex iterator
  vertex_iterator_t vertices() {
    vertex_iterator_t iterator(mesh_);
    return iterator;
  } // vertices

  //! Cell iterator
  cell_iterator_t cells() {
    cell_iterator_t iterator(mesh_);
    return iterator;
  } // cells

  //! Edge iterator
  edge_iterator_t edges() {
    edge_iterator_t iterator(mesh_);
    return iterator;
  } // edges

  /*--------------------------------------------------------------------------*
   * Dual mesh iterators
   *--------------------------------------------------------------------------*/

  wedges_at_corner_iterator_t wedges(corner_t & corner) {
    wedges_at_corner_iterator_t iterator(dual_mesh_, corner.wedges());
    return iterator;
  } // wedges

  /*--------------------------------------------------------------------------*
   *--------------------------------------------------------------------------*/

  vertex_t* create_vertex(const point_t& pos){
    auto v = new vertex_t(nextVertexId_++, pos);
    mesh_.addVertex(v);
    return v;
  }

  cell_t* create_cell(std::initializer_list<vertex_t*> verts){
    assert(verts.size() == burton_mesh_types_t::verticesPerCell() && 
           "vertices size mismatch");
    auto c = new cell_t(nextCellId_++);
    mesh_.addCell(c, verts);
    return c;
  }

  void init(){
    for(auto c : mesh_.cells()){
      ndump(c.id());
      for(auto v : mesh_.verticesOf(c)){
        ndump(v.id());
      }
    }
  }

private:

  private_mesh_t mesh_;
  private_dual_mesh_t dual_mesh_;
  size_t nextVertexId_ = 0;
  size_t nextCellId_ = 0;

}; // class burton_mesh_t

using mesh_t = burton_mesh_t;

} // namespace flexi

#endif // flexi_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
