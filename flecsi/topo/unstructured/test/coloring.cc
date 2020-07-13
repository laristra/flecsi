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

#define __FLECSI_PRIVATE__
#include "flecsi/execution.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/unstructured/interface.hh"
#include "flecsi/topo/unstructured/simple_definition.hh"
#include "flecsi/util/parmetis_colorer.hh"
#include "flecsi/util/unit.hh"

using namespace flecsi;

int
naive_coloring() {
  UNIT {
    topo::unstructured_impl::simple_definition sd("simple2d-16x16.msh");
    ASSERT_EQ(sd.dimension(), 2lu);
    ASSERT_EQ(sd.num_entities(0), 289lu);
    ASSERT_EQ(sd.num_entities(2), 256lu);

    auto naive = topo::unstructured_impl::naive_coloring(sd, 2, 1);

    std::vector<size_t> distribution = { 0, 52, 103, 154, 205, 256 };

    switch(process()) {
      case 0:
        {
        std::vector<size_t> offsets = {
          0, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 46, 49,
          53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 108, 111,
          115, 119, 123, 127, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167,
          170, 173, 177, 181, 185
        };
        std::vector<size_t> indices = {
          1, 16, 0, 2, 17, 1, 3, 18, 2, 4, 19, 3, 5, 20, 4, 6, 21, 5, 7, 22, 6,
          8, 23, 7, 9, 24, 8, 10, 25, 9, 11, 26, 10, 12, 27, 11, 13, 28, 12,
          14, 29, 13, 15, 30, 14, 31, 0, 17, 32, 1, 16, 18, 33, 2, 17, 19, 34,
          3, 18, 20, 35, 4, 19, 21, 36, 5, 20, 22, 37, 6, 21, 23, 38, 7, 22,
          24, 39, 8, 23, 25, 40, 9, 24, 26, 41, 10, 25, 27, 42, 11, 26, 28, 43,
          12, 27, 29, 44, 13, 28, 30, 45, 14, 29, 31, 46, 15, 30, 47, 16, 33,
          48, 17, 32, 34, 49, 18, 33, 35, 50, 19, 34, 36, 51, 20, 35, 37, 52,
          21, 36, 38, 53, 22, 37, 39, 54, 23, 38, 40, 55, 24, 39, 41, 56, 25,
          40, 42, 57, 26, 41, 43, 58, 27, 42, 44, 59, 28, 43, 45, 60, 29, 44,
          46, 61, 30, 45, 47, 62, 31, 46, 63, 32, 49, 64, 33, 48, 50, 65, 34,
          49, 51, 66, 35, 50, 52, 67
        };

        ASSERT_EQ(naive.distribution, distribution);
        ASSERT_EQ(naive.offsets, offsets);
        ASSERT_EQ(naive.indices, indices);
        }
        break;
      case 1:
        {
        std::vector<size_t> offsets = {
          0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 47, 50, 54, 58, 62, 66,
          70, 74, 78, 82, 86, 90, 94, 98, 102, 106, 109, 112, 116, 120, 124,
          128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 171, 174, 178,
          182, 186, 190, 194, 198
        };
        std::vector<size_t> indices = {
          36, 51, 53, 68, 37, 52, 54, 69, 38, 53, 55, 70, 39, 54, 56, 71, 40,
          55, 57, 72, 41, 56, 58, 73, 42, 57, 59, 74, 43, 58, 60, 75, 44, 59,
          61, 76, 45, 60, 62, 77, 46, 61, 63, 78, 47, 62, 79, 48, 65, 80, 49,
          64, 66, 81, 50, 65, 67, 82, 51, 66, 68, 83, 52, 67, 69, 84, 53, 68,
          70, 85, 54, 69, 71, 86, 55, 70, 72, 87, 56, 71, 73, 88, 57, 72, 74,
          89, 58, 73, 75, 90, 59, 74, 76, 91, 60, 75, 77, 92, 61, 76, 78, 93,
          62, 77, 79, 94, 63, 78, 95, 64, 81, 96, 65, 80, 82, 97, 66, 81, 83,
          98, 67, 82, 84, 99, 68, 83, 85, 100, 69, 84, 86, 101, 70, 85, 87,
          102, 71, 86, 88, 103, 72, 87, 89, 104, 73, 88, 90, 105, 74, 89, 91,
          106, 75, 90, 92, 107, 76, 91, 93, 108, 77, 92, 94, 109, 78, 93, 95,
          110, 79, 94, 111, 80, 97, 112, 81, 96, 98, 113, 82, 97, 99, 114, 83,
          98, 100, 115, 84, 99, 101, 116, 85, 100, 102, 117, 86, 101, 103, 118
        };

        ASSERT_EQ(naive.distribution, distribution);
        ASSERT_EQ(naive.offsets, offsets);
        ASSERT_EQ(naive.indices, indices);
        }
        break;
      case 2:
        {
        std::vector<size_t> offsets = {
          0, 4, 8, 12, 16, 20, 24, 28, 32, 35, 38, 42, 46, 50, 54, 58, 62, 66,
          70, 74, 78, 82, 86, 90, 94, 97, 100, 104, 108, 112, 116, 120, 124,
          128, 132, 136, 140, 144, 148, 152, 156, 159, 162, 166, 170, 174, 178,
          182, 186, 190, 194, 198
        };
        std::vector<size_t> indices = {
          87, 102, 104, 119, 88, 103, 105, 120, 89, 104, 106, 121, 90, 105,
          107, 122, 91, 106, 108, 123, 92, 107, 109, 124, 93, 108, 110, 125,
          94, 109, 111, 126, 95, 110, 127, 96, 113, 128, 97, 112, 114, 129, 98,
          113, 115, 130, 99, 114, 116, 131, 100, 115, 117, 132, 101, 116, 118,
          133, 102, 117, 119, 134, 103, 118, 120, 135, 104, 119, 121, 136, 105,
          120, 122, 137, 106, 121, 123, 138, 107, 122, 124, 139, 108, 123, 125,
          140, 109, 124, 126, 141, 110, 125, 127, 142, 111, 126, 143, 112, 129,
          144, 113, 128, 130, 145, 114, 129, 131, 146, 115, 130, 132, 147, 116,
          131, 133, 148, 117, 132, 134, 149, 118, 133, 135, 150, 119, 134, 136,
          151, 120, 135, 137, 152, 121, 136, 138, 153, 122, 137, 139, 154, 123,
          138, 140, 155, 124, 139, 141, 156, 125, 140, 142, 157, 126, 141, 143,
          158, 127, 142, 159, 128, 145, 160, 129, 144, 146, 161, 130, 145, 147,
          162, 131, 146, 148, 163, 132, 147, 149, 164, 133, 148, 150, 165, 134,
          149, 151, 166, 135, 150, 152, 167, 136, 151, 153, 168, 137, 152, 154,
          169
        };

        ASSERT_EQ(naive.distribution, distribution);
        ASSERT_EQ(naive.offsets, offsets);
        ASSERT_EQ(naive.indices, indices);
        }
        break;
      case 3:
        {
        std::vector<size_t> offsets = {
          0, 4, 8, 12, 16, 20, 23, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66,
          70, 74, 78, 82, 85, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
          128, 132, 136, 140, 144, 147, 150, 154, 158, 162, 166, 170, 174, 178,
          182, 186, 190, 194, 198
        };
        std::vector<size_t> indices = {
          138, 153, 155, 170, 139, 154, 156, 171, 140, 155, 157, 172, 141, 156,
          158, 173, 142, 157, 159, 174, 143, 158, 175, 144, 161, 176, 145, 160,
          162, 177, 146, 161, 163, 178, 147, 162, 164, 179, 148, 163, 165, 180,
          149, 164, 166, 181, 150, 165, 167, 182, 151, 166, 168, 183, 152, 167,
          169, 184, 153, 168, 170, 185, 154, 169, 171, 186, 155, 170, 172, 187,
          156, 171, 173, 188, 157, 172, 174, 189, 158, 173, 175, 190, 159, 174,
          191, 160, 177, 192, 161, 176, 178, 193, 162, 177, 179, 194, 163, 178,
          180, 195, 164, 179, 181, 196, 165, 180, 182, 197, 166, 181, 183, 198,
          167, 182, 184, 199, 168, 183, 185, 200, 169, 184, 186, 201, 170, 185,
          187, 202, 171, 186, 188, 203, 172, 187, 189, 204, 173, 188, 190, 205,
          174, 189, 191, 206, 175, 190, 207, 176, 193, 208, 177, 192, 194, 209,
          178, 193, 195, 210, 179, 194, 196, 211, 180, 195, 197, 212, 181, 196,
          198, 213, 182, 197, 199, 214, 183, 198, 200, 215, 184, 199, 201, 216,
          185, 200, 202, 217, 186, 201, 203, 218, 187, 202, 204, 219, 188, 203,
          205, 220
        };

        ASSERT_EQ(naive.distribution, distribution);
        ASSERT_EQ(naive.offsets, offsets);
        ASSERT_EQ(naive.indices, indices);
        }
        break;
      case 4:
        {
        std::vector<size_t> offsets = {
          0, 4, 8, 11, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 66,
          70, 73, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
          128, 132, 135, 137, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167,
          170, 173, 176, 179, 181
        };
        std::vector<size_t> indices = {
          189, 204, 206, 221, 190, 205, 207, 222, 191, 206, 223, 192, 209, 224,
          193, 208, 210, 225, 194, 209, 211, 226, 195, 210, 212, 227, 196, 211,
          213, 228, 197, 212, 214, 229, 198, 213, 215, 230, 199, 214, 216, 231,
          200, 215, 217, 232, 201, 216, 218, 233, 202, 217, 219, 234, 203, 218,
          220, 235, 204, 219, 221, 236, 205, 220, 222, 237, 206, 221, 223, 238,
          207, 222, 239, 208, 225, 240, 209, 224, 226, 241, 210, 225, 227, 242,
          211, 226, 228, 243, 212, 227, 229, 244, 213, 228, 230, 245, 214, 229,
          231, 246, 215, 230, 232, 247, 216, 231, 233, 248, 217, 232, 234, 249,
          218, 233, 235, 250, 219, 234, 236, 251, 220, 235, 237, 252, 221, 236,
          238, 253, 222, 237, 239, 254, 223, 238, 255, 224, 241, 225, 240, 242,
          226, 241, 243, 227, 242, 244, 228, 243, 245, 229, 244, 246, 230, 245,
          247, 231, 246, 248, 232, 247, 249, 233, 248, 250, 234, 249, 251, 235,
          250, 252, 236, 251, 253, 237, 252, 254, 238, 253, 255, 239, 254
        };

        ASSERT_EQ(naive.distribution, distribution);
        ASSERT_EQ(naive.offsets, offsets);
        ASSERT_EQ(naive.indices, indices);
        }
        break;
    } // switch
  };
} // naive_coloring

int
parmetis_colorer() {
  UNIT {
    topo::unstructured_impl::simple_definition sd("simple2d-16x16.msh");

    // Coloring with 5 colors with MPI_COMM_WORLD
    {
    const size_t colors{5};
    auto naive = topo::unstructured_impl::naive_coloring(sd, 2, 1);
    auto raw = util::parmetis::color(naive, colors);
    {
    std::stringstream ss;
    ss << "raw: " << std::endl;
    for(auto r: raw) {
      ss << r << " ";
    }
    ss << std::endl;
    flog_devel(warn) << ss.str();
    } // scope

    auto coloring = util::parmetis::distribute(naive, colors, raw);

    {
    std::stringstream ss;
    size_t color{0};
    for(auto c: coloring) {
      ss << "color " << color++ << ":" << std::endl;
      for(auto i: c) {
        ss << i << " ";
      }
      ss << std::endl;
    }
    ss << std::endl;
    flog_devel(warn) << ss.str();
    } // scope
    } // scope

    // Coloring with 5 colors with custom communicator with 2 processes
    {
    MPI_Comm group_comm;
    MPI_Comm_split(MPI_COMM_WORLD, process() < 2 ? 0 : MPI_UNDEFINED, 0,
      &group_comm);

    if(process() < 2) {
      auto naive = topo::unstructured_impl::naive_coloring(sd, 2, 1,
        process(), 2);
      auto raw = util::parmetis::color(naive, 5, group_comm);

      {
      std::stringstream ss;
      ss << "raw: " << std::endl;
      for(auto r: raw) {
        ss << r << " ";
      }
      ss << std::endl;
      flog_devel(warn) << ss.str();
      } // scope

      auto coloring = util::parmetis::distribute(naive, 5,
        raw, group_comm);

      {
      std::stringstream ss;
      size_t color{0};
      for(auto c: coloring) {
        ss << "color " << color++ << ":" << std::endl;
        for(auto i: c) {
          ss << i << " ";
        }
        ss << std::endl;
      }
      ss << std::endl;
      flog_devel(warn) << ss.str();
      } // scope
    } // if
    } // scope
  };
} // parmetis_colorer

int
coloring_driver() {
  UNIT {
    // TODO: use test<> when reduction works for MPI tasks
    execute<naive_coloring, mpi>();
    execute<parmetis_colorer, mpi>();
  };
} // simple2d_8x8

flecsi::unit::driver<coloring_driver> driver;
