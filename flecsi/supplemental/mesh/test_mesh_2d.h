/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <algorithm>

#include <cinchlog.h>

#include <flecsi/data/data_client_handle.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/topology/mesh.h>
#include <flecsi/topology/mesh_topology.h>

//----------------------------------------------------------------------------//
// Enumeration to name index spaces
//----------------------------------------------------------------------------//

enum index_spaces : size_t {
  vertices,
  edges,
  cells,
  cells_to_vertices
}; // enum index_spaces

namespace flecsi {
namespace supplemental {

//----------------------------------------------------------------------------//
// Entity types
//----------------------------------------------------------------------------//

enum neighbor_id : size_t { left_ = 0, right_ = 1, above_ = 2, below_ = 3 };

using point_t = std::array<double, 2>;
using index_t = std::array<size_t, 2>;

struct vertex_t : public flecsi::topology::mesh_entity__<0, 1> {
  vertex_t(point_t const & p, index_t const & index) : p_(p), index_(index) {}

  point_t const & coordinates() const {
    return p_;
  }

  index_t const & index() const {
    return index_;
  }

private:
  point_t p_;
  index_t index_;
}; // struct vertex_t

struct edge_t : public flecsi::topology::mesh_entity__<1, 1> {
}; // struct edge_t

struct face_t : public flecsi::topology::mesh_entity__<1, 1> {
}; // struct face_t

struct cell_t : public flecsi::topology::mesh_entity__<2, 1> {
  using id_t = flecsi::utils::id_t;

  cell_t(const index_t & index) : index_(index) {
    neighbors_.fill(nullptr);
  }

  std::vector<size_t> create_entities(
      id_t cell_id,
      size_t dim,
      flecsi::topology::domain_connectivity__<2> & c,
      id_t * e) {
    clog_assert(false, "cell_t::create_entities is not implemented");
    return {};
  } // create_entities

  index_t const & index() const {
    return index_;
  }

  // global cell id
  size_t gid() const {
#ifdef FLECSI_8_8_MESH
    const size_t width{8};
#else
    const size_t width{16};
#endif
    return index_[0] * width + index_[1];
  }

  const auto & neighbors() const {
    return neighbors_;
  }
  auto left() const {
    return neighbors_[left_];
  };
  auto right() const {
    return neighbors_[right_];
  }
  auto above() const {
    return neighbors_[above_];
  }
  auto below() const {
    return neighbors_[below_];
  }
  auto get_neighbor(size_t neighbor_id) const {
    return neighbors_[neighbor_id];
  }

  void set_neighbor(size_t neighbor_id, const cell_t * neighbor) {
    neighbors_[neighbor_id] = neighbor;
  }

private:
  index_t index_;

  // neighbor information
  std::array<const cell_t *, 4> neighbors_;
}; // struct cell_t

//----------------------------------------------------------------------------//
// Mesh policy
//----------------------------------------------------------------------------//

struct test_mesh_2d_policy_t {
  using id_t = flecsi::utils::id_t;

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
      flecsi_entity_type(index_spaces::vertices, 0, vertex_t),
      // flecsi_entity_type(index_spaces::edges, 0, edge_t),
      flecsi_entity_type(index_spaces::cells, 0, cell_t));

  flecsi_register_connectivities(flecsi_connectivity(
      index_spaces::cells_to_vertices,
      0,
      cell_t,
      vertex_t));

  flecsi_register_bindings();

#ifdef FLECSI_TEST_MESH_INDEX_SUBSPACES
  using index_subspaces = std::tuple<std::tuple<
      flecsi::topology::index_space_<0>,
      flecsi::topology::index_subspace_<0>>>;
#endif

  template<size_t M, size_t D, typename ST>
  static flecsi::topology::mesh_entity_base__<num_domains> * create_entity(
      flecsi::topology::mesh_topology_base__<ST> * mesh,
      size_t num_vertices,
      id_t const & id) {
#if 0
    switch(M) {

      case 0:
      {
        switch(D) {
          case 1:
            return mesh->template make<edge_t>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        } // switch

        break;
      } // scope

      default:
        assert(false && "invalid domain");
    } // switch
#endif
    return nullptr;
  } // create_entity

}; // struct test_mesh_2d_t

//----------------------------------------------------------------------------//
// Mesh type
//----------------------------------------------------------------------------//

struct test_mesh_2d_t
    : public flecsi::topology::mesh_topology__<test_mesh_2d_policy_t> {

  auto cells() {
    return entities<2, 0>();
  } // cells

  auto cells(flecsi::partition_t p) {
    return entities<2, 0>(p);
  } // cells

  template<typename E, size_t M>
  auto vertices(flecsi::topology::domain_entity__<M, E> & e) {
    return entities<0, 0>(e);
  } // vertices
  /*
    template<
      typename E,
      size_t M
    >
    auto
    vertices(
      flecsi::topology::domain_entity__<M, E> & e
    )
    const
    {
      return entities<0, 0>(e);
    } // vertices
  */

  auto vertices() {
    return entities<0, 0>();
  } // cells

}; // struct test_mesh_2d_t

//----------------------------------------------------------------------------//
// Mesh coloring
//----------------------------------------------------------------------------//

void
do_test_mesh_2d_coloring() {
  coloring_map_t map{index_spaces::vertices, index_spaces::cells};
  flecsi_execute_mpi_task(add_colorings, flecsi::supplemental, map);

  auto & context{execution::context_t::instance()};
  auto & vinfo{context.coloring_info(index_spaces::vertices)};
  auto & cinfo{context.coloring_info(index_spaces::cells)};

  coloring::adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for (auto & itr : cinfo) {
    size_t color{itr.first};
    const coloring::coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  context.add_adjacency(ai);
}

//----------------------------------------------------------------------------//
// Initialize mesh
//----------------------------------------------------------------------------//

void
initialize_mesh(data_client_handle__<test_mesh_2d_t, wo> mesh) {
  auto & context = execution::context_t::instance();

  auto & vertex_map{context.index_map(index_spaces::vertices)};
  auto & reverse_vertex_map{context.reverse_index_map(index_spaces::vertices)};
  auto & cell_map{context.index_map(index_spaces::cells)};

  std::vector<vertex_t *> vertices;

#ifdef FLECSI_8_8_MESH
  const size_t width{8};
#else
  const size_t width{16};
#endif

  // make vertices
  for (auto & vm : vertex_map) {
    const size_t mid{vm.second};
    const size_t row{mid / (width + 1)};
    const size_t column{mid % (width + 1)};
    // printf("vertex %lu: (%lu, %lu)\n", mid, row, column);
    point_t point({{(double)row, (double)column}});
    index_t index({{row, column}});

    vertices.push_back(mesh.make<vertex_t>(point, index));
  } // for

  // make cells
  std::unordered_map<size_t, cell_t *> cells_vs_ids;

  for (auto & cm : cell_map) {
    const size_t mid{cm.second};

    const size_t row{mid / width};
    const size_t column{mid % width};

    const size_t v0{(column) + (row) * (width + 1)};
    const size_t v1{(column + 1) + (row) * (width + 1)};
    const size_t v2{(column + 1) + (row + 1) * (width + 1)};
    const size_t v3{(column) + (row + 1) * (width + 1)};

    const size_t lv0{reverse_vertex_map[v0]};
    const size_t lv1{reverse_vertex_map[v1]};
    const size_t lv2{reverse_vertex_map[v2]};
    const size_t lv3{reverse_vertex_map[v3]};

    auto c{mesh.make<cell_t>(index_t{{row, column}})};
    cells_vs_ids[mid] = c;
    mesh.init_cell<0>(
        c, {vertices[lv0], vertices[lv1], vertices[lv2], vertices[lv3]});
  } // for

  mesh.init<0>();

  // set neighbor cells on cells that are owned by this rank (exclusive and
  // shared), all the neighbors also exist on this rank, although some of them
  // are ghost cells
  auto & cell_coloring{context.coloring(index_spaces::cells)};

  auto exclusive_and_shared = cell_coloring.exclusive;
  exclusive_and_shared.insert(
      cell_coloring.shared.begin(), cell_coloring.shared.end());

  for (auto c : exclusive_and_shared) {
    auto this_cell = cells_vs_ids.at(c.id);
    size_t row = this_cell->index()[0];
    size_t column = this_cell->index()[1];

    // left
    if (column > 0) {
      size_t left_id = (column - 1) + row * width;
      this_cell->set_neighbor(left_, cells_vs_ids.at(left_id));
    }
    // right
    if (column < width - 1) {
      size_t right_id = (column + 1) + row * width;
      this_cell->set_neighbor(right_, cells_vs_ids.at(right_id));
    }
    // below
    if (row > 0) {
      size_t below_id = column + (row - 1) * width;
      this_cell->set_neighbor(below_, cells_vs_ids.at(below_id));
    }
    // above
    if (row < width - 1) {
      size_t above_id = column + (row + 1) * width;
      this_cell->set_neighbor(above_, cells_vs_ids.at(above_id));
    }
  } // for

} // initizlize_mesh

flecsi_register_task(initialize_mesh, flecsi::supplemental, loc, single);

} // namespace supplemental
} // namespace flecsi
