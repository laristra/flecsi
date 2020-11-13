/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#include "fixed.hh"

#define __FLECSI_PRIVATE__
#include "flecsi/data.hh"
#include "flecsi/execution.hh"
#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

namespace global {
std::vector<std::size_t> cells;
std::vector<std::size_t> vertices;
} // namespace global

struct unstructured : topo::specialization<topo::unstructured, unstructured> {

  /*--------------------------------------------------------------------------*
    Structure
   *--------------------------------------------------------------------------*/

  enum index_space { vertices, edges, cells };
  using index_spaces = has<cells, vertices>;
  using connectivities =
    list<from<cells, to<vertices>>, from<vertices, to<cells>>>;

  enum entity_list { dirichlet, neumann, special_vertices };
  using entity_lists = list<entity<vertices, has<special_vertices>>>;

  /*--------------------------------------------------------------------------*
    Interface
   *--------------------------------------------------------------------------*/

  template<class B>
  struct interface : B {

    auto cells() {
      return B::template entities<index_space::cells>();
    }

    template<index_space From>
    auto cells(topo::id<From> from) {
      return B::template entities<index_space::cells>(from);
    }

    auto vertices() {
      return B::template entities<index_space::vertices>();
    }

    template<index_space From>
    auto vertices(topo::id<From> from) {
      return B::template entities<index_space::vertices>(from);
    }

    auto edges() {
      return B::template entities<index_space::edges>();
    }

    template<entity_list List>
    auto edges() {
      return B::template special_entities<unstructured::edges, List>();
    }

  }; // struct interface

  /*--------------------------------------------------------------------------*
    Coloring
   *--------------------------------------------------------------------------*/

  static coloring color() {
    return {processes(),
      idx_allocs[process()],
      idx_colorings[process()],
      cnx_allocs[process()],
      cnx_colorings[process()]};
  } // color

  /*--------------------------------------------------------------------------*
    Initialization
   *--------------------------------------------------------------------------*/

  static void init_c2v(field<util::id, data::ragged>::mutator c2v,
    topo::unstructured_impl::crs const & cnx,
    std::map<std::size_t, std::size_t> & map) {
    std::size_t off{0};

    for(std::size_t c{0}; c < cnx.offsets.size() - 1; ++c) {
      const std::size_t start = cnx.offsets[off];
      const std::size_t size = cnx.offsets[off + 1] - start;

      c2v[c].resize(size);

      for(std::size_t i{0}; i < size; ++i) {
        c2v[c][i] = map[cnx.indices[start + i]];
      }
      ++off;
    }
  }

  static void init_v2c(field<util::id, data::ragged>::mutator v2c,
    field<util::id, data::ragged>::accessor<ro> c2v) {
    for(std::size_t c{0}; c < c2v.size(); ++c) {
      for(std::size_t v{0}; v < c2v[c].size(); ++v) {
        v2c[c2v[c][v]].push_back(c);
      }
    }
  }

  static void initialize(data::topology_slot<unstructured> & s,
    coloring const & c) {

    auto cell_coloring = c.idx_colorings[0];
    global::cells.clear();
    for(auto e : cell_coloring.exclusive) {
      global::cells.push_back(e);
    }

    for(auto e : cell_coloring.shared) {
      global::cells.push_back(e.id);
    }

    for(auto e : cell_coloring.ghost) {
      global::cells.push_back(e.id);
    }

    topo::unstructured_impl::force_unique(global::cells);

    auto vertex_coloring = c.idx_colorings[1];
    global::vertices.clear();
    for(auto e : vertex_coloring.exclusive) {
      global::vertices.push_back(e);
    }

    for(auto e : vertex_coloring.shared) {
      global::vertices.push_back(e.id);
    }

    for(auto e : vertex_coloring.ghost) {
      global::vertices.push_back(e.id);
    }

    topo::unstructured_impl::force_unique(global::vertices);

    std::map<std::size_t, std::size_t> vertex_map;
    std::size_t off{0};
    for(auto e : global::vertices) {
      vertex_map[e] = off++;
    }

    auto & c2v =
      s->connect_.get<unstructured::cells>().get<unstructured::vertices>();
    execute<init_c2v, mpi>(c2v(s), c.cnx_colorings[0][0], vertex_map);

    auto & v2c =
      s->connect_.get<unstructured::vertices>().get<unstructured::cells>();
    execute<init_v2c, mpi>(v2c(s), c2v(s));
  } // initialize

}; // struct unstructured

unstructured::slot mesh;
unstructured::cslot coloring;

const field<std::size_t>::definition<unstructured, unstructured::cells> cids;
const field<std::size_t>::definition<unstructured, unstructured::vertices> vids;

void
init_ids(unstructured::accessor<ro> m,
  field<std::size_t>::accessor<wo> cids,
  field<std::size_t>::accessor<wo> vids) {
  for(auto c : m.cells()) {
    cids[c] = global::cells[c];
  }
  for(auto v : m.vertices()) {
    vids[v] = global::vertices[v];
  }
}

void
print(unstructured::accessor<ro> m,
  field<std::size_t>::accessor<ro> cids,
  field<std::size_t>::accessor<ro> vids) {
  for(auto c : m.cells()) {
    std::stringstream ss;
    ss << "cell(" << cids[c] << "): ";
    for(auto v : m.vertices(c)) {
      ss << vids[v] << " ";
    }
    flog(info) << ss.str() << std::endl;
  }

  for(auto v : m.vertices()) {
    std::stringstream ss;
    ss << "vertex(" << vids[v] << "): ";
    for(auto c : m.cells(v)) {
      ss << cids[c] << " ";
    }
    flog(info) << ss.str() << std::endl;
  }
}

int
fixed_driver() {
  UNIT {
    coloring.allocate();
    mesh.allocate(coloring.get());
    execute<init_ids>(mesh, cids(mesh), vids(mesh));
    execute<print>(mesh, cids(mesh), vids(mesh));
  };
} // unstructured_driver

flecsi::unit::driver<fixed_driver> driver;
