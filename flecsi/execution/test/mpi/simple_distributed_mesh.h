//
// Created by ollie on 7/5/17.
//

#ifndef FLECSI_SIMPLE_DISTRIBUTED_MESH_H
#define FLECSI_SIMPLE_DISTRIBUTED_MESH_H


#include "flecsi/topology/index_space.h"
#include "flecsi/topology/mesh_topology.h"
#include "flecsi/coloring/coloring_types.h"
#include "flecsi/execution/execution.h"
#include "flecsi/io/simple_definition.h"

#include "simple_entity_types.h"
#include "simple_types.h"

enum simple_distributed_mesh_index_spaces_t : size_t {
  vertices,
  edges,
  faces,
  cells
}; // enum mesh_index_spaces_t

enum simple_distributed_cell_index_spaces_t : size_t {
  exclusive,
  shared,
  ghost
};

class simple_distributed_mesh_t : public flecsi::topology::mesh_topology_t<simple_types_t> {
public:
  using base_t = flecsi::topology::mesh_topology_t<simple_types_t>;

  using entry_info_t = flecsi::coloring::entity_info_t;

  using vertex_t = simple_vertex_t;

  simple_distributed_mesh_t(const char* filename) : sd(filename) {
    // TODO: move this into a factory that takes a simple_definition and out put a
    // a simple_distributed_mesh.
    auto &context = flecsi::execution::context_t::instance();

    auto vertex_coloring = context.coloring(1);//context.coloring_map().find(1);

    std::vector<vertex_t *> vs;
    std::map<size_t, size_t> vertex_global_to_local_map;
    size_t index = 0;
    for (auto vertex : vertex_coloring.exclusive) {
      // get the vertex coordinates from sd.
      vs.push_back(create_vertex(sd.vertex(vertex.id), vertex.id));
      vertex_global_to_local_map[vertex.id] = index++;
    }
    for (auto vertex : vertex_coloring.shared) {
      // get the vertex coordinates from sd.
      vs.push_back(create_vertex(sd.vertex(vertex.id), vertex.id));
      vertex_global_to_local_map[vertex.id] = index++;
    }
    for (auto vertex : vertex_coloring.ghost) {
      // get the vertex coordinates from sd.
      vs.push_back(create_vertex(sd.vertex(vertex.id), vertex.id));
      vertex_global_to_local_map[vertex.id] = index++;
    }

    auto cell_coloring = context.coloring(1);//context.coloring_map().find(0);
    for (auto cell : cell_coloring.exclusive) {
      auto vertices = sd.vertices(2, cell.id);
      create_cell({vs[vertex_global_to_local_map.at(vertices[0])],
                   vs[vertex_global_to_local_map.at(vertices[1])],
                   vs[vertex_global_to_local_map.at(vertices[2])],
                   vs[vertex_global_to_local_map.at(vertices[3])]},
                  exclusive,
                  cell.id);
    }
    for (auto cell : cell_coloring.shared) {
      auto vertices = sd.vertices(2, cell.id);
      create_cell({vs[vertex_global_to_local_map.at(vertices[0])],
                   vs[vertex_global_to_local_map.at(vertices[1])],
                   vs[vertex_global_to_local_map.at(vertices[2])],
                   vs[vertex_global_to_local_map.at(vertices[3])]},
                  exclusive,
                  cell.id);
    }
    // We do need cell to cell connectivity between primary and ghost cells
    // the only way (is it?) to do it with mesh_topology is by adding ghost
    // cells to the mesh as well. We also add two indice spaces, one for
    // primary and the other for ghost
    for (auto cell : cell_coloring.ghost) {
      auto vertices = sd.vertices(2, cell.id);
      create_cell({vs[vertex_global_to_local_map.at(vertices[0])],
                   vs[vertex_global_to_local_map.at(vertices[1])],
                   vs[vertex_global_to_local_map.at(vertices[2])],
                   vs[vertex_global_to_local_map.at(vertices[3])]},
                  ghost,
                  cell.id);
    }
  }
#if 0

    init();

    // TODO: maybe we don't even need these any more.
    //size_t index = 0;
    index = 0;
    for (const auto& cell : weaver.get_primary_cells()) {
      global_to_local_map[cell] = index++;
      local_to_global_map.push_back(cell);
    }
    for (const auto& cell : weaver.get_ghost_cells()) {
      global_to_local_map[cell.id] = index++;
      local_to_global_map.push_back(cell.id);
    }
  }

  void
  init()
  {
    // Initialize domain 0 of the mesh topology.
    base_t::init<0>();

    // Use a predicate function to create the interior cells
    // index space
    primary_cells_ =
      base_t::entities<2, 0>().filter([](auto cell) {return cell->type == primary;});

    // Use a predicate function to create the domain boundary cells
    // index space
    ghost_cells_ =
      base_t::entities<2, 0>().filter([](auto cell) {return cell->type == ghost;});
  } // init
#endif

  vertex_t *
  create_vertex(const flecsi::point<double, 2>& pos, size_t mesh_id)
  {
    auto v = base_t::make<vertex_t>(*this, pos);
    //base_t::add_entity<0, 0>(v, 0, mesh_id);
    base_t::add_entity<0, 0>(v, 0);
    return v;
  }

  simple_cell_t *
  create_cell(const std::initializer_list<vertex_t *> & vertices, size_t type, size_t mesh_id)
  {
    auto c = base_t::make<simple_cell_t>(*this, type);
    //base_t::add_entity<2, 0>(c, 0, mesh_id);
    base_t::add_entity<2, 0>(c, 0);
    base_t::init_entity<0, 2, 0>(c, vertices);
    return c;
  }
#if 0
  size_t indices(size_t index_space_id) const override {
    switch(index_space_id) {
      case simple_distributed_mesh_index_spaces_t::cells:
        return primary_cells_.size() + ghost_cells_.size();
      default:
        // FIXME: lookup user-defined index space
        clog_fatal("unknown index space");
        return 0;
    } // switch
  }

  auto
  cells()
  {
    return base_t::entities<2, 0>();
  } // cells

  auto
  cells(
    size_t is
  )
  {
    switch(is) {
      case simple_distributed_cell_index_spaces_t::primary:
        return primary_cells_;
      case simple_distributed_cell_index_spaces_t::ghost:
        return ghost_cells_;
      default:
        assert(false && "unknown index space");
    } // switch
  } // cells

  // convert a local cell id into a global cell id
  size_t global_cell_id(size_t cell_local_id) {
    if (cell_local_id > local_to_global_map.size()) {
      throw std::invalid_argument("invalid local cell id: " + std::to_string(cell_local_id));
    } else {
      return local_to_global_map[cell_local_id];
    }
  }

  // convert a global cell in into a local cell id
  size_t local_cell_id(size_t cell_global_id) {
    auto iter = global_to_local_map.find(cell_global_id);
    if (iter == global_to_local_map.end()) {
      throw std::invalid_argument("invalid global cell id");
    } else {
      return iter->second;
    }
  }

  // get number of primary cells
  size_t num_primary_cells() const {
    return primary_cells_.size();
  }

  // get number of ghost cells
  size_t num_ghost_cells() const {
    return ghost_cells_.size();
  }

  // Find the peers to form a MPI group. We need both peers that need our shared
  // cells and the peers that provides us ghost cells.
  std::vector<int> get_shared_peers() const {
    std::set<int> rank_set;
    for (auto cell : weaver.get_shared_cells()) {
      for (auto peer : cell.shared) {
        rank_set.insert(peer);
      }
    }
    for (auto cell : weaver.get_ghost_cells()) {
      rank_set.insert(cell.rank);
    }
    std::vector<int> peers(rank_set.begin(), rank_set.end());
    return peers;
  }

  // FIXME: Do we really need to expose these information?
  std::set <entry_info_t> get_ghost_cells_info() const {
    return weaver.get_ghost_cells();
  }
  std::set <entry_info_t> get_shared_cells_info() const {
    return weaver.get_shared_cells();
  }
  std::set <size_t> get_primary_cells() const {
    return weaver.get_primary_cells();
  }
#endif

private:
  // TODO: shoud we retain these two members?
  flecsi::io::simple_definition_t sd;

  std::map<size_t, size_t> global_to_local_map;
  std::vector<size_t> local_to_global_map;

  flecsi::topology::index_space<
    flecsi::topology::domain_entity<0, simple_cell_t>, false, true, false> primary_cells_;
  flecsi::topology::index_space<
    flecsi::topology::domain_entity<0, simple_cell_t>, false, true, false> ghost_cells_;
};
#endif //FLECSI_SIMPLE_DISTRIBUTED_MESH_H
