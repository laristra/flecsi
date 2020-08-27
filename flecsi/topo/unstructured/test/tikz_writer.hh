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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/topo/unstructured/types.hh"
#include <mpi.h>

namespace flecsi {
namespace supplemental {

using palette_data_t = std::tuple<std::string, std::string, std::string>;

const std::vector<palette_data_t> palette = {
  std::make_tuple("blue", "blue!40!white", "blue!5!white"),
  std::make_tuple("green!60!black", "green!60!white", "green!10!white"),
  std::make_tuple("black", "black!40!white", "black!5!white"),
  std::make_tuple("red", "red!40!white", "red!5!white"),
  std::make_tuple("violet", "violet!40!white", "violet!5!white"),
  std::make_tuple("cyan", "cyan!40!white", "cyan!5!white")};

struct tikz_writer {

  using entity_map_t =
    std::unordered_map<size_t, topo::unstructured_base::entity_info>;

  using entity_set_t = std::unordered_map<size_t, std::set<size_t>>;

  static void write_primary(size_t M,
    size_t N,
    entity_set_t const & cell_colors,
    entity_set_t const & vertex_colors) {
    std::stringstream texname;
    texname << "primary-" << M << "x" << N << ".tex";
    std::ofstream tex(texname.str(), std::ofstream::out);

    // Document header and start
    tex << "% Mesh visualization" << std::endl;
    tex << "\\documentclass[tikz,border=7mm]{standalone}" << std::endl;
    tex << std::endl;

    tex << "\\begin{document}" << std::endl;
    tex << std::endl;

    // Picture start
    tex << "\\begin{tikzpicture}" << std::endl;
    tex << std::endl;

    // Draw the grid
    tex << "\\draw[step=1cm,black] (0, 0) grid (" << M << ", " << N << ");"
        << std::endl;

    for(auto c : cell_colors) {
      const size_t round_robin(c.first % palette.size());

      for(size_t id : c.second) {
        write_node(tex, id % N + 0.5, id / N + 0.5, id, round_robin, true);
      } // for
    } // for

    for(auto c : vertex_colors) {
      const size_t round_robin(c.first % palette.size());

      for(size_t id : c.second) {
        write_node(tex, id % (N + 1), id / (N + 1), id, round_robin);
      } // for
    } // for

    // Picture end
    tex << "\\end{tikzpicture}" << std::endl;
    tex << std::endl;

    // Document end
    tex << "\\end{document}" << std::endl;
  } // write_primary

  static void write_color(size_t const color,
    size_t const M,
    size_t const N,
    entity_map_t const & exclusive_cells,
    entity_map_t const & shared_cells,
    entity_map_t const & ghost_cells,
    entity_map_t const & exclusive_vertices,
    entity_map_t const & shared_vertices,
    entity_map_t const & ghost_vertices) {
    const size_t round_robin = color % palette.size();

    std::stringstream texname;
    texname << "color-" << color << "-" << M << "x" << N << ".tex";
    std::ofstream tex(texname.str(), std::ofstream::out);

    // Document header and start
    tex << "% Mesh visualization" << std::endl;
    tex << "\\documentclass[tikz,border=7mm]{standalone}" << std::endl;
    tex << std::endl;

    tex << "\\begin{document}" << std::endl;
    tex << std::endl;

    // Picture start
    tex << "\\begin{tikzpicture}" << std::endl;
    tex << std::endl;

    tex << "\\draw[step=1cm,black] (0, 0) grid (" << M << ", " << N << ");"
        << std::endl;

    // Content
    size_t cell(0);
    for(size_t j(0); j < M; ++j) {
      double yoff(0.5 + j);
      for(size_t i(0); i < M; ++i) {
        double xoff(0.5 + i);

        // Cells
        auto ecell = exclusive_cells.find(cell);
        auto scell = shared_cells.find(cell);
        auto gcell = ghost_cells.find(cell);

        if(ecell != exclusive_cells.end()) {
          write_node(tex, xoff, yoff, cell, round_robin);
        }
        else if(scell != shared_cells.end()) {
          write_node(tex, xoff, yoff, cell, round_robin, false, true);
        }
        else if(gcell != ghost_cells.end()) {
          const size_t off_round_robin = gcell->second.rank % palette.size();
          write_node(tex, xoff, yoff, cell, off_round_robin, false, true);
        } // if

        ++cell;
      } // for
    } // for

    size_t vertex(0);
    for(size_t j(0); j < M + 1; ++j) {
      double yoff(j);
      for(size_t i(0); i < N + 1; ++i) {
        double xoff(i);

        auto evertex = exclusive_vertices.find(vertex);
        auto svertex = shared_vertices.find(vertex);
        auto gvertex = ghost_vertices.find(vertex);

        if(evertex != exclusive_vertices.end()) {
          write_node(tex, xoff, yoff, vertex, round_robin);
        }
        else if(svertex != shared_vertices.end()) {
          write_node(tex, xoff, yoff, vertex, round_robin, false, true);
        }
        else if(gvertex != ghost_vertices.end()) {
          const size_t off_round_robin = gvertex->second.rank % palette.size();
          write_node(tex, xoff, yoff, vertex, off_round_robin, false, true);
        } // if

        ++vertex;
      } // for
    } // for

    // Picture end
    tex << "\\end{tikzpicture}" << std::endl;
    tex << std::endl;

    // Document end
    tex << "\\end{document}" << std::endl;
  } // write_owned

private:
  static void write_node(std::ofstream & stream,
    double xoff,
    double yoff,
    size_t id,
    size_t color,
    bool fill = false,
    bool shared = false) {
    stream << "\\node[";

    if(shared) {
      stream << std::get<1>(palette[color]);
    }
    else {
      stream << std::get<0>(palette[color]);
    } // if

    if(fill) {
      stream << ","
             << "fill=" << std::get<2>(palette[color]);
    } // if

    stream << "] at (" << xoff << ", " << yoff << ") {" << id << "};"
           << std::endl;
  } // write_node

}; // struct tikz_writer

template<class Policy>
void
write_closure(topo::unstructured_base::coloring & closure, MPI_Comm comm) {
  auto & cells = closure.index_colorings[0];
  auto & vertices = closure.index_colorings[1];
  const size_t M(16), N(16);
  typename Policy::communicator communicator(comm);
  auto rank = communicator.rank();

  // Maps for output
  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    shared_cells_map;
  for(auto i : cells.shared) {
    shared_cells_map[i.id] = i;
  } // for
  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    exclusive_cells_map;
  for(auto i : cells.exclusive) {
    exclusive_cells_map[i.id] = i;
  } // for

  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    ghost_cells_map;
  for(auto i : cells.ghost) {
    ghost_cells_map[i.id] = i;
  } // for

  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    exclusive_vertices_map;
  for(auto i : vertices.exclusive) {
    exclusive_vertices_map[i.id] = i;
    vertices.primary.insert(i.id);
  } // for

  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    shared_vertices_map;
  for(auto i : vertices.shared) {
    shared_vertices_map[i.id] = i;
    vertices.primary.insert(i.id);
  } // for

  std::unordered_map<size_t, topo::unstructured_base::entity_info>
    ghost_vertices_map;
  for(auto i : vertices.ghost) {
    ghost_vertices_map[i.id] = i;
  } // for

  { // debug
    if(rank == 0) {
      std::cout << "primary size: " << vertices.exclusive.size() << std::endl;
    }
  }

  // Gather primary partitions
  auto primary_cells = communicator.get_entity_reduction(cells.primary);
  auto primary_vertices = communicator.get_entity_reduction(vertices.primary);

  if(rank == 0) {
    supplemental::tikz_writer::write_primary(
      M, N, primary_cells, primary_vertices);
  } // if

  supplemental::tikz_writer::write_color(rank,
    M,
    N,
    exclusive_cells_map,
    shared_cells_map,
    ghost_cells_map,
    exclusive_vertices_map,
    shared_vertices_map,
    ghost_vertices_map);
}

} // namespace supplemental
} // namespace flecsi
