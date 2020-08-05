/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef box_utils_hh
#define box_utils_hh

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include <algorithm>
#include <cassert>
#include <cmath>

namespace flecsi {
namespace topo {
namespace structured_impl {

util::span<const int>
bid2dim(int dim) {

  static const int map_1d[] = {0, 1, 0};
  static const int map_2d[] = {0, 1, 0, 1, 2, 1, 0, 1, 0};
  static const int map_3d[] = {0,
    1,
    0,
    1,
    2,
    1,
    0,
    1,
    0,
    1,
    2,
    1,
    2,
    3,
    2,
    1,
    2,
    1,
    0,
    1,
    0,
    1,
    2,
    1,
    0,
    1,
    0};

  switch(dim) {
    case 1:
      return map_1d;
    case 2:
      return map_2d;
    case 3:
      return map_3d;
    default:
      return {};
  }
} // bid2dim

util::span<const int>
dim2bid(int dim, int edim) {
  static const int map_d1_ed0[] = {0, 2};
  static const int map_d2_ed0[] = {0, 2, 6, 8};
  static const int map_d2_ed1[] = {1, 3, 5, 7};
  static const int map_d3_ed0[] = {0, 2, 6, 8, 18, 20, 24, 26};
  static const int map_d3_ed1[] = {1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25};
  static const int map_d3_ed2[] = {4, 10, 12, 14, 16, 22};
  switch(dim) {
    case 1:
      if(edim == 0)
        return map_d1_ed0;
      else
        return {};
    case 2:
      if(edim == 0)
        return map_d2_ed0;
      else if(edim == 1)
        return map_d2_ed1;
      else
        return {};
    case 3:
      if(edim == 0)
        return map_d3_ed0;
      else if(edim == 1)
        return map_d3_ed1;
      else if(edim == 2)
        return map_d3_ed2;
      else
        return {};
    default:
      return {};
  }
} // dim2bid

util::span<const int>
bid2dir(int dim, int dir) {
  static const int map_d1_d0[] = {0, 2, 1};
  static const int map_d2_d0[] = {0, 2, 1, 0, 2, 1, 0, 2, 1};
  static const int map_d2_d1[] = {0, 0, 0, 2, 2, 2, 1, 1, 1};
  static const int map_d3_d0[] = {0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1,
    0,
    2,
    1};
  static const int map_d3_d1[] = {0,
    0,
    0,
    2,
    2,
    2,
    1,
    1,
    1,
    0,
    0,
    0,
    2,
    2,
    2,
    1,
    1,
    1,
    0,
    0,
    0,
    2,
    2,
    2,
    1,
    1,
    1};
  static const int map_d3_d2[] = {0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1};
  switch(dim) {
    case 1:
      if(dir == 0)
        return map_d1_d0;
      else
        return {};
    case 2:
      if(dir == 0)
        return map_d2_d0;
      else if(dir == 1)
        return map_d2_d1;
      else
        return {};
    case 3:
      if(dir == 0)
        return map_d3_d0;
      else if(dir == 1)
        return map_d3_d1;
      else if(dir == 2)
        return map_d3_d2;
      else
        return {};
    default:
      return {};
  }
} // bid2dir

auto
dim2bounds(int dim, int edim) {
  std::vector<std::vector<int>> map;
  switch(dim) {
    case 1:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1, 0, 2}};
      }
      break;
    case 2:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1, 1, 0, 2, 6, 8, 1, 3, 5, 7}};
      }
      else if(edim == 1) {
        map = std::vector<std::vector<int>>{{1, 0, 3, 5}, {0, 1, 1, 7}};
      }
      break;
    case 3:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1,
          1,
          1,
          0,
          2,
          6,
          8,
          18,
          20,
          24,
          26,
          1,
          3,
          5,
          7,
          9,
          11,
          15,
          17,
          19,
          21,
          23,
          25,
          4,
          10,
          12,
          14,
          16,
          22}};
      }
      else if(edim == 1) {
        map = std::vector<std::vector<int>>{
          {1, 1, 0, 9, 11, 15, 17, 10, 12, 14, 16},
          {0, 1, 1, 1, 7, 19, 25, 4, 10, 16, 22},
          {1, 0, 1, 3, 5, 21, 23, 4, 12, 14, 22}};
      }
      else if(edim == 2) {
        map = std::vector<std::vector<int>>{
          {1, 0, 0, 12, 14}, {0, 1, 0, 10, 16}, {0, 0, 1, 4, 22}};
      }
      break;
  }

  return map;
} // dim2bounds

} // namespace structured_impl
} // namespace topo
} // namespace flecsi
#endif // box_utils_hh
