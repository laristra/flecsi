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

auto
bid2dim(int dim) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      map = std::vector<int>{0, 1, 0};
      break;
    case 2:
      map = std::vector<int>{0, 1, 0, 1, 2, 1, 0, 1, 0};
      break;
    case 3:
      map = std::vector<int>{0,
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
      break;
  }
  return map;
} // bid2dim

auto
dim2bid(int dim, int edim) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      if(edim == 0) {
        map = std::vector<int>{0, 2};
      }
      break;
    case 2:
      if(edim == 0) {
        map = std::vector<int>{0, 2, 6, 8};
      }
      else if(edim == 1) {
        map = std::vector<int>{1, 3, 5, 7};
      }
      break;
    case 3:
      if(edim == 0) {
        map = std::vector<int>{0, 2, 6, 8, 18, 20, 24, 26};
      }
      else if(edim == 1) {
        map = std::vector<int>{1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25};
      }
      else if(edim == 2) {
        map = std::vector<int>{4, 10, 12, 14, 16, 22};
      }
      break;
  }
  return map;
} // dim2bid

auto
bid2dir(int dim, int dir) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      if(dir == 0) {
        map = std::vector<int>{0, 2, 1};
      }
      break;
    case 2:
      if(dir == 0) {
        map = std::vector<int>{0, 2, 1, 0, 2, 1, 0, 2, 1};
      }
      else if(dir == 1) {
        map = std::vector<int>{0, 0, 0, 2, 2, 2, 1, 1, 1};
      }
      break;
    case 3:
      if(dir == 0) {
        map = std::vector<int>{0,
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
      }
      else if(dir == 1) {
        map = std::vector<int>{0,
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
      }
      else if(dir == 2) {
        map = std::vector<int>{0,
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
      }
      break;
  }

  return map;
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
