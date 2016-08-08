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

#include "flecsi/specializations/sagittarius/sagittarius_mesh.h"
#include "flecsi/specializations/sagittarius/test/sagittarius_fixture.h"

using namespace flecsi;

TEST_F(A_Sagittarius_Mesh, number_of_vertices_should_be_8) {
  ASSERT_EQ(8, constellation.num_vertices());
}

TEST_F(A_Sagittarius_Mesh, number_of_cells_should_be_4) {
  ASSERT_EQ(4, constellation.num_cells());
}

TEST_F(A_Sagittarius_Mesh, number_of_edges_should_be_11) {
  ASSERT_EQ(11, constellation.num_edges());
}

TEST_F(A_Sagittarius_Mesh, dump) {
  auto vs = constellation.vertices();
  CINCH_CAPTURE() << "vertex to vertex connectivities:\n";
  for (auto v0 : vs) {
    CINCH_CAPTURE() << v0.id() << ": ";
    for (auto v1 : constellation.vertices(v0)) {
      CINCH_CAPTURE() << v1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to edge connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto e: constellation.edges(v)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "vertex to cell connectivities:\n";
  for (auto v : vs) {
    CINCH_CAPTURE() << v.id() << ": ";
    for (auto c: constellation.cells(v)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to vertex connectivities:\n";
  auto edges = constellation.edges();
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto v: constellation.vertices(e)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to edge connectivities:\n";
  for (auto e0: edges) {
    CINCH_CAPTURE() << e0.id() << ": ";
    for (auto e1: constellation.edges(e0)) {
      CINCH_CAPTURE() << e1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "edge to cell connectivities:\n";
  for (auto e: edges) {
    CINCH_CAPTURE() << e.id() << ": ";
    for (auto c: constellation.cells(e)) {
      CINCH_CAPTURE() << c.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  auto cells = constellation.cells();
  CINCH_CAPTURE() << "cell to vertex connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto v: constellation.vertices(c)) {
      CINCH_CAPTURE() << v.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to edge connectivities:\n";
  for (auto c: cells) {
    CINCH_CAPTURE() << c.id() << ": ";
    for (auto e: constellation.edges(c)) {
      CINCH_CAPTURE() << e.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_CAPTURE() << "cell to cell connectivities:\n";
  for (auto c0: cells) {
    CINCH_CAPTURE() << c0.id() << ": ";
    for (auto c1: constellation.cells(c0)) {
      CINCH_CAPTURE() << c1.id() << " ";
    }
    CINCH_CAPTURE() << std::endl;
  }
  CINCH_CAPTURE() << std::endl;

  CINCH_ASSERT(TRUE, CINCH_EQUAL_BLESSED("sagittarius.blessed"));
}
