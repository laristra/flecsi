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
#define __FLECSI_PRIVATE__
#define __FLECSI_PRIVATE__
#include "flecsi/util/demangle.hh"
#include "flecsi/util/unit.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>
#include <flecsi/util/array_ref.hh>

using namespace flecsi;
using namespace flecsi::data;
using namespace flecsi::topo;
using namespace flecsi::util;

const std::size_t nents = 1 << 10;

using hkey_t = std::size_t;
struct htype_t {
  std::size_t a, b, c;
  double d, e, f;
  std::string g, h, i;
};

using pair_t = std::pair<hkey_t, htype_t>;

using hmap_t = hashtable<hkey_t, htype_t>;

int
assign(span<pair_t> & span_ht) {
  hmap_t hmap(span_ht);
  int error = 0;
  for(std::size_t i = 0; i < nents; ++i) {
    auto it = hmap.insert(i + 1, i, i, i, 0., 0., 0., "a", "b", "c");
    if(it == hmap.end()) {
      ++error;
    }
  }
  return error;
} // assign

int
check(span<pair_t> & span_ht) {
  hmap_t hmap(span_ht);
  int error = 0;
  for(std::size_t i = 0; i < nents; ++i) {
    auto it = hmap.find(i + 1);
    if(it == hmap.end()) {
      ++error;
    }
    else if(it->second.a != i && it->second.i != "c") {
      ++error;
    }
  }
  size_t i = 0;
  // Loop over the table and check elements
  for(auto & a : hmap) {
    if(i % 20 && a.first != i) {
      ++error;
    }
  }
  return error;
} // check

int
empty(span<pair_t> & span_ht) {
  hmap_t hmap(span_ht);
  hmap.clear();
  // Assert the table is empty
  int error;
  for(auto & a : hmap) {
    ++error;
  }
  return error;
} // print

int
hashtable_driver() {
  UNIT {
    const std::size_t ht_size = 1 << 20;

    std::vector<pair_t> idx_s;
    span<pair_t> span_ht;

    idx_s.resize(ht_size);
    span_ht = span(&(idx_s.front()), &(idx_s.back()));

    EXPECT_EQ(assign(span_ht), 0);
    EXPECT_EQ(check(span_ht), 0);

    // Empty the table
    EXPECT_EQ(empty(span_ht), 0);

    // Re-insert values
    EXPECT_EQ(assign(span_ht), 0);
    EXPECT_EQ(check(span_ht), 0);
  }; // UNIT
} // ntree_driver

flecsi::unit::driver<hashtable_driver> driver;
