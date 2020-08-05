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

#include "flecsi/util/color_map.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

int
interface() {
  UNIT {
    // Assumed that the test is run with 3 threads and 8 colors
    ASSERT_EQ(processes(), 3lu);

    util::color_map cm(3, 8, 3700);

    std::stringstream ss;
    ss << "distribution: ";
    for(auto d : cm.distribution()) {
      ss << d << " ";
    }
    flog_devel(info) << ss.str() << std::endl;

    ASSERT_EQ(cm.domain_size(), 3lu);

    ASSERT_EQ(cm.color_offset(0), 0lu);
    ASSERT_EQ(cm.color_offset(1), 3lu);
    ASSERT_EQ(cm.color_offset(2), 6lu);

    ASSERT_EQ(cm.colors(0), 3lu);
    ASSERT_EQ(cm.colors(1), 3lu);
    ASSERT_EQ(cm.colors(2), 2lu);

    ASSERT_EQ(cm.index_offset(0, 0), 0lu);
    ASSERT_EQ(cm.index_offset(0, 1), 463lu);
    ASSERT_EQ(cm.index_offset(0, 2), 926lu);
    ASSERT_EQ(cm.index_offset(1, 0), 1389lu);
    ASSERT_EQ(cm.index_offset(1, 1), 1852lu);
    ASSERT_EQ(cm.index_offset(1, 2), 2314lu);
    ASSERT_EQ(cm.index_offset(2, 0), 2776lu);
    ASSERT_EQ(cm.index_offset(2, 1), 3238lu);

    ASSERT_EQ(cm.index_color(100), 0lu);
    ASSERT_EQ(cm.index_color(600), 1lu);
    ASSERT_EQ(cm.index_color(1100), 2lu);
    ASSERT_EQ(cm.index_color(1600), 3lu);
    ASSERT_EQ(cm.index_color(2100), 4lu);
    ASSERT_EQ(cm.index_color(2600), 5lu);
    ASSERT_EQ(cm.index_color(3100), 6lu);
    ASSERT_EQ(cm.index_color(3600), 7lu);

    ASSERT_EQ(cm.process(0), 0lu);
    ASSERT_EQ(cm.process(1), 0lu);
    ASSERT_EQ(cm.process(2), 0lu);
    ASSERT_EQ(cm.process(3), 1lu);
    ASSERT_EQ(cm.process(4), 1lu);
    ASSERT_EQ(cm.process(5), 1lu);
    ASSERT_EQ(cm.process(6), 2lu);
    ASSERT_EQ(cm.process(7), 2lu);

    ASSERT_EQ(cm.indices(), 3700ul);

    ASSERT_EQ(cm.indices(0, 0), 463ul);
    ASSERT_EQ(cm.indices(0, 1), 463ul);
    ASSERT_EQ(cm.indices(0, 2), 463ul);
    ASSERT_EQ(cm.indices(1, 0), 463ul);
    ASSERT_EQ(cm.indices(1, 1), 462ul);
    ASSERT_EQ(cm.indices(1, 2), 462ul);
    ASSERT_EQ(cm.indices(2, 0), 462ul);
    ASSERT_EQ(cm.indices(2, 1), 462ul);
  };
}

int
color_map() {
  UNIT { execute<interface, mpi>(); };
} // color_map

unit::driver<color_map> color_map_driver;
