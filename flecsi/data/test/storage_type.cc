/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <vector>
#include <algorithm>

#include "flecsi/data/data.h"

/*
#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << X << std::endl
*/

using namespace flecsi;

enum mesh_index_spaces_t : size_t {
  vertices,
  edges,
  faces,
  cells
}; // enum mesh_index_spaces_t

enum class privileges : size_t {
  read,
  read_write,
  write_discard
}; // enum data_access_type_t

struct mesh_t : public data::data_client_t {

  size_t indices(size_t index_space_id) override {

    switch(index_space_id) {
      case cells:
        return 100;
        break;
      default:
        // FIXME: lookup user-defined index space
        assert(false && "unknown index space");
    } // switch
  }

}; // struct mesh_t

/*----------------------------------------------------------------------------*
 * Dense storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, dense) {
  using namespace flecsi::data;

  mesh_t m;

  // Register 3 versions
  register_data(m, "pressure", 3, double, dense, cells);

  // Initialize
  {
  auto p0 = get_accessor(m, "pressure", 0, double, dense);
  auto p1 = get_accessor(m, "pressure", 1, double, dense);
  auto p2 = get_accessor(m, "pressure", 2, double, dense);

  for(size_t i(0); i<100; ++i) {
    p0[i] = i;
    p1[i] = 1000 + i;
    p2[i] = -double(i);
  } // for
  } // scope

  // Test
  {
  auto p0 = get_accessor(m, "pressure", 0, double, dense);
  auto p1 = get_accessor(m, "pressure", 1, double, dense);
  auto p2 = get_accessor(m, "pressure", 2, double, dense);

  for(size_t i(0); i<100; ++i) {
    ASSERT_EQ(p0[i], i);
    ASSERT_EQ(p1[i], 1000+p0[i]);
    ASSERT_EQ(p2[i], -p0[i]);
  } // for
  } // scope
} // TEST

/*----------------------------------------------------------------------------*
 * Scalar storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, global) {
  using namespace flecsi::data;

  mesh_t m;

  struct my_data_t {
    double t;
    size_t n;
  }; // struct my_data_t

  register_data(m, "simulation data", 2, my_data_t, global);

  // initialize simulation data
  {
  auto s0 = get_accessor(m, "simulation data", 0, my_data_t, global);
  auto s1 = get_accessor(m, "simulation data", 1, my_data_t, global);
  s0->t = 0.5;
  s0->n = 100;
  s1->t = 1.5;
  s1->n = 200;
  } // scope

  {
  auto s0 = get_accessor(m, "simulation data", 0, my_data_t, global);
  auto s1 = get_accessor(m, "simulation data", 1, my_data_t, global);

  ASSERT_EQ(s0->t, 0.5);
  ASSERT_EQ(s0->n, 100);
  ASSERT_EQ(s1->t, 1.0 + s0->t);
  ASSERT_EQ(s1->n, 100 + s0->n);
  } // scope
} // TEST

/*----------------------------------------------------------------------------*
 * Sparse storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, sparse1) {
  using namespace flecsi::data;

// TODO: sparse data changes in progress
  mesh_t m;

  size_t num_indices = 100;
  size_t num_materials = 50;

  register_data(m, "a", 1, double, sparse, num_indices, num_materials);
  auto am = get_mutator(m, "a", 0, double, sparse, 10);

  for(size_t i = 0; i < num_indices; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      am(i, j) = i * 100 + j;
    }
  }

  am.commit();

  auto a = get_accessor(m, "a", 0, double, sparse);

  for(size_t i = 0; i < num_indices ; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      ASSERT_EQ(a(i, j), i * 100 + j);
    }
  }

} // TEST

TEST(storage, sparse2) {
  using namespace flecsi::data;

// TODO: sparse data changes in progress
  mesh_t m;

  size_t num_indices = 1000;
  size_t num_materials = 50;

  register_data(m, "a", 1, double, sparse, num_indices, num_materials);

  std::vector<std::pair<size_t, size_t>> v;

  for(size_t i = 0; i < num_indices; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      v.push_back({i, j});
    }
  }

  std::random_shuffle(v.begin(), v.end());

  auto am = get_mutator(m, "a", 0, double, sparse, 30);

  for(auto p : v){
    am(p.first, p.second) = p.first * 1000 + p.second;
  }

  am.commit();

  auto a = get_accessor(m, "a", 0, double, sparse);

  for(size_t i = 0; i < num_indices ; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      ASSERT_EQ(a(i, j), i * 1000 + j);
    }
  }

} // TEST

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 *  CINCH_ASSERT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 *  CINCH_EXPECT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
