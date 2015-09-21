/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_burton_h
#define jali_burton_h

#include "../base/mesh_topology.h"

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace jali {

  template<class T, size_t D>
  class Vector{
  public:
    Vector(){}

    Vector(const Vector& v)
      : v_(v.v_){}

    Vector(std::initializer_list<T> v){
      assert(v.size() == D && "vector size mismatch");

      std::copy(v.begin(), v.end(), v_.begin()); 
    }

    T& operator[](size_t i){
      return v_[i];
    }

    const T& operator[](size_t i) const{
      return v_[i];
    }

    Vector& operator=(const Vector& v) const{
      v_ = v;
    }

    std::array<T, D> v_;
  };

  class burton_mesh_type{
  public:
    static constexpr size_t Dimension = 2;

    using Id = uint64_t;

    using Vec = Vector<double, Dimension>;
    
    // Vertex type
    struct burton_vertex_t : public MeshEntity {
      burton_vertex_t(size_t id) : MeshEntity(id) {}

      burton_vertex_t(size_t id, const Vec& pos) :
        MeshEntity(id),
        pos(pos){}
      
      Vec pos;
    }; // struct burton_vertex_t

    // Edge type
    struct burton_edge_t : public MeshEntity {
      burton_edge_t(size_t id) : MeshEntity(id) {}
    }; // struct burton_edge_t

    // Cell type
    class burton_cell_t : public MeshEntity {
    public:

      burton_cell_t(size_t id) : MeshEntity(id) {}

      EntityGroup<2> & getSides() {
        return sides_;
      } // getSides

    private:

      EntityGroup<2> sides_;

    }; // struct burton_cell_t


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

 class dual_mesh_type{
  public:
    static constexpr size_t Dimension = 2;

    using Id = uint64_t;

    using Vec = Vector<double, Dimension>;

    using burton_vertex_t = burton_mesh_type::burton_vertex_t;

    using burton_edge_t = burton_mesh_type::burton_edge_t;

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
  \class burton burton.h
  \brief burton provides...
 */
class burton
{
public:
  using MT = burton_mesh_type;
  

  using vertex_t = MT::burton_vertex_t;
  using edge_t = MT::burton_edge_t;
  using cell_t = MT::burton_cell_t;

  using Vec = burton_mesh_type::Vec;

  //! Default constructor
  burton() {}

  //! Copy constructor (disabled)
  burton(const burton &) = delete;

  //! Assignment operator (disabled)
  burton & operator = (const burton &) = delete;

  //! Destructor
  ~burton() {}

  vertex_t* create_vertex(const Vec& pos){
    auto v = new vertex_t(nextVertexId_++, pos);
    mesh_.addVertex(v);
    return v;
  }

  cell_t* create_cell(std::initializer_list<vertex_t*> verts){
    assert(verts.size() == MT::verticesPerCell() && "vertices size mismatch");
    auto c = new cell_t(nextCellId_++);
    mesh_.addCell(c, verts);
    return c;
  }

private:
  using mesh_t = Mesh<burton_mesh_type>;

  mesh_t mesh_;
  size_t nextVertexId_ = 0;
  size_t nextCellId_ = 0;

}; // class burton

} // namespace jali

#endif // jali_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
