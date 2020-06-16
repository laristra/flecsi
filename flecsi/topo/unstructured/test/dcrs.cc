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

#include "flecsi/topo/unstructured/dcrs_utils.hh"
#include "flecsi/topo/unstructured/simple_definition.hh"
#include "flecsi/util/graph/parmetis_colorer.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/execution.hh>

using namespace flecsi;

int
naive_coloring() {
  UNIT {
    topo::unstructured_impl::simple_definition sd("simple2d-16x16.msh");

    auto naive = topo::unstructured_impl::naive_coloring<2, 2>(sd);
  };
} // naive_coloring

int
simple2d_8x8() {
  UNIT {
    topo::unstructured_impl::simple_definition sd("simple2d-8x8.msh");

    auto dcrs = topo::unstructured_impl::make_dcrs(sd);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
      // Note: These assume that this test is run with 5 ranks.
      const std::vector<size_t> offsets = {
        0, 2, 5, 8, 11, 14, 17, 20, 22, 25, 29, 33, 37};

      const std::vector<size_t> indices = {1,
        8,
        0,
        2,
        9,
        1,
        3,
        10,
        2,
        4,
        11,
        3,
        5,
        12,
        4,
        6,
        13,
        5,
        7,
        14,
        6,
        15,
        0,
        9,
        16,
        1,
        8,
        10,
        17,
        2,
        9,
        11,
        18,
        3,
        10,
        12,
        19};

      ASSERT_EQ(dcrs.offsets, offsets);
      ASSERT_EQ(dcrs.indices, indices);
    } // if
  };
} // simple2d_8x8

int
dcrs_driver() {
  UNIT {
    // TODO: use test<> when reduction works for MPI tasks
    execute<naive_coloring, mpi>();
    execute<simple2d_8x8, mpi>();
  };
} // simple2d_8x8

flecsi::unit::driver<dcrs_driver> driver;
