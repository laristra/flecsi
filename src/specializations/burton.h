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

  class burton_mesh_type{
  public:
    using Id = uint64_t;

    static constexpr size_t topologicalDimension(){
      return 2;
    }

    // Vertex type
    struct burton_vertex_t : public MeshEntity {
      burton_vertex_t(size_t id) : MeshEntity(id) {}
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

    // Side type
    struct burton_side_t : public MeshEntity {
      burton_side_t(size_t id) : MeshEntity(id) {}
    }; // struct burton_side_t

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

/*!
  \class burton burton.h
  \brief burton provides...
 */
class burton
{
public:

  using burton_vertex_t = burton_mesh_type::burton_vertex_t;
  using burton_edge_t = burton_mesh_type::burton_edge_t;
  using burton_cell_t = burton_mesh_type::burton_cell_t;
  using burton_side_t = burton_mesh_type::burton_side_t;

  //! Default constructor
  burton() {}

  //! Copy constructor (disabled)
  burton(const burton &) = delete;

  //! Assignment operator (disabled)
  burton & operator = (const burton &) = delete;

  //! Destructor
  ~burton() {}

private:
  using mesh_t = Mesh<burton_mesh_type>;

  mesh_t mesh_;

}; // class burton

} // namespace jali

#endif // jali_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
