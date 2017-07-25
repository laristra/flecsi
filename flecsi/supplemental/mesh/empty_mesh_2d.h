/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_empty_mesh_h
#define flecsi_execution_empty_mesh_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 10, 2017
//----------------------------------------------------------------------------//

#include "flecsi/topology/mesh.h"
#include "flecsi/topology/mesh_topology.h"

namespace flecsi {
namespace supplemental {

  class vertex : public topology::mesh_entity_t<0, 1>{
  public:
    template<size_t M>
    uint64_t precedence() const { return 0; }
    vertex() = default;

  };

  class cell : public topology::mesh_entity_t<2, 1>{
  public:

    using id_t = flecsi::utils::id_t;

    std::vector<size_t>
    create_entities(id_t cell_id, size_t dim,
                    topology::domain_connectivity<2> & c, id_t * e){
      return {2, 2, 2, 2};
    }

  };

  class empty_mesh_types_t{
  public:
    static constexpr size_t num_dimensions = 2;

    static constexpr size_t num_domains = 1;

    using entity_types = std::tuple<
      std::tuple<topology::index_space_<0>, topology::domain_<0>, cell>/*
      std::tuple<topology::index_space_<1>, topology::domain_<0>, vertex>*/>;

    using connectivities = std::tuple<>;

    using bindings = std::tuple<>;

    template<size_t M, size_t D, typename ST>
    static topology::mesh_entity_base_t<num_domains>*
    create_entity(topology::mesh_topology_base_t<ST>* mesh,
                  size_t num_vertices){
      switch(M){
        case 0:{
          switch(D){
            default:
              assert(false && "invalid topological dimension");
          }
          break;
        }
        default:
          assert(false && "invalid domain");
      }
    }
  };

  using empty_mesh_t = topology::mesh_topology_t<empty_mesh_types_t>;
  using empty_mesh_2d_t = topology::mesh_topology_t<empty_mesh_types_t>;
} // namespace supplemental
} // namespace flecsi

#endif // flecsi_execution_empty_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
