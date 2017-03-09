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

enum data_attributes_t : size_t {
  flagged
}; // enum data_attributes_t

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

  size_t indices(size_t index_space_id) const override {

    switch(index_space_id) {
      case cells:
        return 100;
        break;
      case vertices:
        return 50;
        break;
      default:
        // FIXME: lookup user-defined index space
        clog_fatal("unknown index space");
        return 0;
    } // switch
  }

}; // struct mesh_t

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

TEST(storage, dense) {

  using namespace flecsi::data;

  mesh_t m;

  // Register 3 versions
  flecsi_register_data(m, hydro, pressure, double, dense, 2, cells);
  flecsi_register_data(m, hydro, density, double, dense, 1, cells);

  // Initialize
  {
  auto p0 = flecsi_get_accessor(m, hydro, pressure, double, dense, 0);
  auto p1 = flecsi_get_accessor(m, hydro, pressure, double, dense, 1);
  auto d = flecsi_get_accessor(m, hydro, density, double, dense, 0);

  p0.attributes().set(flagged);

  std::cout << "index space: " << p0.index_space() << std::endl;

  for(size_t i(0); i<100; ++i) {
    p0[i] = i;
    p1[i] = 1000 + i;
    d[i] = -double(i);
  } // for
  } // scope

  // Test
  {
  auto p0 = flecsi_get_accessor(m, hydro, pressure, double, dense, 0);
  auto p1 = flecsi_get_accessor(m, hydro, pressure, double, dense, 1);
  auto d = flecsi_get_accessor(m, hydro, density, double, dense, 0);

  ASSERT_TRUE(p0.attributes().test(flagged));
  ASSERT_FALSE(p1.attributes().test(flagged));
  ASSERT_FALSE(d.attributes().test(flagged));

  for(size_t i(0); i<100; ++i) {
    ASSERT_EQ(p0[i], i);
    ASSERT_EQ(p1[i], 1000+p0[i]);
    ASSERT_EQ(d[i], -p0[i]);
  } // for
  } // scope
} // TEST

//----------------------------------------------------------------------------//
// Scalar storage type.
//----------------------------------------------------------------------------//

TEST(storage, global) {

  using namespace flecsi::data;

  mesh_t m;

  struct my_data_t {
    double t;
    size_t n;
  }; // struct my_data_t

  flecsi_register_data(m, hydro, simulation_data, my_data_t, global, 2);

  // initialize simulation data
  {
  auto s0 = flecsi_get_accessor(m, hydro, simulation_data, my_data_t, global, 0);
  auto s1 = flecsi_get_accessor(m, hydro, simulation_data, my_data_t, global, 1);

  s0.attributes().set(flagged);

  s0->t = 0.5;
  s0->n = 100;
  s1->t = 1.5;
  s1->n = 200;
  } // scope

  {
  auto s0 = flecsi_get_accessor(m, hydro, simulation_data, my_data_t, global, 0);
  auto s1 = flecsi_get_accessor(m, hydro, simulation_data, my_data_t, global, 1);

  ASSERT_TRUE(s0.attributes().test(flagged));
  ASSERT_FALSE(s1.attributes().test(flagged));

  ASSERT_EQ(s0->t, 0.5);
  ASSERT_EQ(s0->n, 100);
  ASSERT_EQ(s1->t, 1.0 + s0->t);
  ASSERT_EQ(s1->n, 100 + s0->n);
  } // scope
} // TEST

//----------------------------------------------------------------------------//
// Sparse storage type.
//----------------------------------------------------------------------------//

TEST(storage, sparse1) {
  using namespace flecsi::data;

// TODO: sparse data changes in progress
  mesh_t m;

  size_t num_indices = 100;
  size_t num_materials = 50;

  flecsi_register_data(m, hydro, a, double, sparse, 1, cells, num_materials);
  auto am = flecsi_get_mutator(m, hydro, a, double, sparse, 0, 10);

  for(size_t i = 0; i < num_indices; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      am(i, j) = i * 100 + j;
    }
  }

  am.commit();

  auto a = flecsi_get_accessor(m, hydro, a, double, sparse, 0);

  for(size_t i = 0; i < num_indices ; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      ASSERT_EQ(a(i, j), i * 100 + j);
    }
  }

  /*
  // iterate through all indices with non-null entries
  for(auto i : a.indices()){
    std::cout << "index: " << i << std::endl;
  }

  // iterate through all materials/entries used by index 0
  for(auto e : a.entries(0)){
    std::cout << "entry for index 0: " << e << std::endl;
  }

  // iterate through all materials/entries used
  for(auto e : a.entries()){
    std::cout << "entry: " << e << std::endl;
  }
  */

} // TEST

TEST(storage, sparse2) {
  using namespace flecsi::data;

// TODO: sparse data changes in progress
  mesh_t m;

  size_t num_indices = 100;
  size_t num_materials = 50;

  flecsi_register_data(m, hydro, a, double, sparse, 1, cells, num_materials);

  std::vector<std::pair<size_t, size_t>> v;

  for(size_t i = 0; i < num_indices; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      v.push_back({i, j});
    }
  }

  std::random_shuffle(v.begin(), v.end());

  auto am = flecsi_get_mutator(m, hydro, a, double, sparse, 0, 30);

  for(auto p : v){
    am(p.first, p.second) = p.first * 1000 + p.second;
  }

  am.commit();

  auto a = flecsi_get_accessor(m, hydro, a, double, sparse, 0);

  for(size_t i = 0; i < num_indices ; i += 2){
    for(size_t j = 0; j < num_materials; j += 2){
      ASSERT_EQ(a(i, j), i * 1000 + j);
    }
  }

} // TEST

/*
TEST(storage, sparse_delete) {
  using namespace flecsi::data;

// TODO: sparse data changes in progress
  mesh_t m;

  size_t num_indices = 5;
  size_t num_materials = 3;

  flecsi_register_data(m, hydro, a, double, sparse, 1, num_indices, num_materials);

  auto am = flecsi_get_mutator(m, hydro, a, double, sparse, 0, 30);


  am(2, 1) = 7;
  am(2, 2) = 3;
  am(1, 1) = 1;

  am.erase(1, 1);
  am.erase(2, 1);

  am.commit();

  auto a = get_accessor(m, hydro, a, double, sparse, 0);
  ASSERT_EQ(a(2, 2), 3);
} // TEST
*/

//! \brief This tests the various ways to access data via attributes.
//! \remark Tests the dense accessor.
TEST(storage, dense_attributes) {

  using namespace flecsi::data;

  mesh_t m;

  // Register 3 versions
  flecsi_register_data(m, hydro, pressure, double, dense, 2, cells);
  flecsi_register_data(m, hydro, density, double, dense, 1, cells);
  flecsi_register_data(m, hydro, speed, double, dense, 1, vertices);
  flecsi_register_data(m, radiation, temperature, double, dense, 1, cells);

  // Initialize
  {
    auto p = flecsi_get_accessor(m, hydro, pressure, double, dense, 0);
    auto d = flecsi_get_accessor(m, hydro, density, double, dense, 0);
    auto t = flecsi_get_accessor(m, radiation, temperature, double, dense, 0);
    p.attributes().set(flagged);
    t.attributes().set(flagged);
  }

  // Test
  {
    auto p0 = flecsi_get_accessor(m, hydro, pressure, double, dense, 0);
    auto p1 = flecsi_get_accessor(m, hydro, pressure, double, dense, 1);
    auto d = flecsi_get_accessor(m, hydro, density, double, dense, 0);
    auto t = flecsi_get_accessor(m, radiation, temperature, double, dense, 0);

    ASSERT_TRUE(p0.attributes().test(flagged));
    ASSERT_FALSE(p1.attributes().test(flagged));
    ASSERT_FALSE(d.attributes().test(flagged));
    ASSERT_TRUE(t.attributes().test(flagged));

    // test is_at(cells)
    auto cell_vars = flecsi_get_accessors(
      m, hydro, double, dense, 0, flecsi_is_at(cells), /* sorted */ true
    );

    ASSERT_EQ( cell_vars.size(), 2 );
    ASSERT_EQ( cell_vars[0].label(), "density" );
    ASSERT_EQ( cell_vars[1].label(), "pressure" );

    auto all_cell_vars = flecsi_get_accessors_all(
      m, double, dense, 0, flecsi_is_at(cells), /* sorted */ true 
    );

    ASSERT_EQ( all_cell_vars.size(), 3 );
    ASSERT_EQ( all_cell_vars[0].label(), "density" );
    ASSERT_EQ( all_cell_vars[1].label(), "pressure" );
    ASSERT_EQ( all_cell_vars[2].label(), "temperature" );


    // test has_attribute_at(flagge,cells)
    auto flagged_vars = flecsi_get_accessors(
      m, hydro, double, dense, 0, flecsi_has_attribute_at(flagged, cells), 
      /* sorted */ true
    );

    ASSERT_EQ( flagged_vars.size(), 1 );
    ASSERT_EQ( flagged_vars[0].label(), "pressure" );

    auto all_flagged_vars = flecsi_get_accessors_all(
      m, double, dense, 0, flecsi_has_attribute_at(flagged, cells), /* sorted */ true 
    );

    ASSERT_EQ( all_flagged_vars.size(), 2 );
    ASSERT_EQ( all_flagged_vars[0].label(), "pressure" );
    ASSERT_EQ( all_flagged_vars[1].label(), "temperature" );

    // test get by type=double
    auto typed_vars = flecsi_get_accessors(
      m, hydro, double, dense, 0, /* sorted */ true
    );

    ASSERT_EQ( typed_vars.size(), 3 );
    ASSERT_EQ( typed_vars[0].label(), "density" );
    ASSERT_EQ( typed_vars[1].label(), "pressure" );
    ASSERT_EQ( typed_vars[2].label(), "speed" );

    auto all_typed_vars = flecsi_get_accessors_all(
      m, double, dense, 0, /* sorted */ true 
    );

    ASSERT_EQ( all_typed_vars.size(), 4 );
    ASSERT_EQ( all_typed_vars[0].label(), "density" );
    ASSERT_EQ( all_typed_vars[1].label(), "pressure" );
    ASSERT_EQ( all_typed_vars[2].label(), "speed" );
    ASSERT_EQ( all_typed_vars[3].label(), "temperature" );


  } // scope
} // TEST

//! \brief This tests the various ways to access data via attributes.
//! \remark Tests the global accessor.
TEST(storage, global_attributes) {

  using namespace flecsi::data;

  mesh_t m;

  // Register 3 versions
  flecsi_register_data(m, hydro, pressure, double, global, 2);
  flecsi_register_data(m, hydro, density, double, global, 1);
  flecsi_register_data(m, hydro, speed, double, global, 1);
  flecsi_register_data(m, radiation, temperature, double, global, 1);

  // Initialize
  {
    auto p = flecsi_get_accessor(m, hydro, pressure, double, global, 0);
    auto d = flecsi_get_accessor(m, hydro, density, double, global, 0);
    auto t = flecsi_get_accessor(m, radiation, temperature, double, global, 0);
    p.attributes().set(flagged);
    t.attributes().set(flagged);
  }

  // Test
  {
    auto p0 = flecsi_get_accessor(m, hydro, pressure, double, global, 0);
    auto p1 = flecsi_get_accessor(m, hydro, pressure, double, global, 1);
    auto d = flecsi_get_accessor(m, hydro, density, double, global, 0);
    auto t = flecsi_get_accessor(m, radiation, temperature, double, global, 0);

    ASSERT_TRUE(p0.attributes().test(flagged));
    ASSERT_FALSE(p1.attributes().test(flagged));
    ASSERT_FALSE(d.attributes().test(flagged));
    ASSERT_TRUE(t.attributes().test(flagged));


    // test has_attribute(flagge)
    auto flagged_vars = flecsi_get_accessors(
      m, hydro, double, global, 0, flecsi_has_attribute(flagged), 
      /* sorted */ true
    );

    ASSERT_EQ( flagged_vars.size(), 1 );
    ASSERT_EQ( flagged_vars[0].label(), "pressure" );

    auto all_flagged_vars = flecsi_get_accessors_all(
      m, double, global, 0, flecsi_has_attribute(flagged), /* sorted */ true 
    );

    ASSERT_EQ( all_flagged_vars.size(), 2 );
    ASSERT_EQ( all_flagged_vars[0].label(), "pressure" );
    ASSERT_EQ( all_flagged_vars[1].label(), "temperature" );

    // test get by type=double
    auto typed_vars = flecsi_get_accessors(
      m, hydro, double, global, 0, /* sorted */ true
    );

    ASSERT_EQ( typed_vars.size(), 3 );
    ASSERT_EQ( typed_vars[0].label(), "density" );
    ASSERT_EQ( typed_vars[1].label(), "pressure" );
    ASSERT_EQ( typed_vars[2].label(), "speed" );

    auto all_typed_vars = flecsi_get_accessors_all(
      m, double, global, 0, /* sorted */ true 
    );

    ASSERT_EQ( all_typed_vars.size(), 4 );
    ASSERT_EQ( all_typed_vars[0].label(), "density" );
    ASSERT_EQ( all_typed_vars[1].label(), "pressure" );
    ASSERT_EQ( all_typed_vars[2].label(), "speed" );
    ASSERT_EQ( all_typed_vars[3].label(), "temperature" );


  } // scope
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
