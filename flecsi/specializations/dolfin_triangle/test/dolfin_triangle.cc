/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/specializations/dolfin_triangle/dolfin_triangle_mesh.h"
#include "flecsi/specializations/dolfin_triangle/test/dolfin_triangle_fixture.h"

using namespace flecsi;

TEST_F(A_Dolfin_Triangle, number_of_vertices_should_be_10) {
  ASSERT_EQ(10, dolfin.num_vertices());
}

TEST_F(A_Dolfin_Triangle, number_of_cells_should_be_10) {
  ASSERT_EQ(10, dolfin.num_cells());
}

TEST_F(A_Dolfin_Triangle, number_of_edges_should_be_19) {
  ASSERT_EQ(19, dolfin.num_edges());
}

TEST_F(A_Dolfin_Triangle, dump_should_match_the_blessed_file) {
  CINCH_CAPTURE() << "vertex to vertex connectivities:\n";
  auto vs = dolfin.vertices();
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto v1 : dolfin.vertices(v)) {
      CINCH_CAPTURE() << v1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to edge connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto e: dolfin.edges(v)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to cell connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto c: dolfin.cells(v)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to vertex connectivities:\n";
  auto edges = dolfin.edges();
  for (auto e : edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto v: dolfin.vertices(e)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to edge connectivities:\n";
  for (auto e0: edges) {
    CINCH_CAPTURE() << e0.id() << ": ";
    for (auto e1 : dolfin.edges(e0)) {
      CINCH_CAPTURE() << e1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to cell connectivities:\n";
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto c: dolfin.cells(e)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to vertex connectivities:\n";
  auto cells = dolfin.cells();
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto v: dolfin.vertices(c)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to edge connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto e: dolfin.edges(c)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to cell connectivities:\n";
  for (auto c0: cells) {
    CINCH_CAPTURE() << c0.id() << ": ";
    for (auto c1: dolfin.cells(c0)) {
      CINCH_CAPTURE() << c1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("dolfin_triangle.blessed"));
}

