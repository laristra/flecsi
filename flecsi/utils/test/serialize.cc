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

#include <flecsi/utils/serialize.h>

#include <cstddef>
#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cinchtest.h>

using namespace flecsi::utils;

//----------------------------------------------------------------------------//
// Sanity test.
//----------------------------------------------------------------------------//

TEST(serialize, sanity) {

  std::vector<std::byte> data;

  {
    std::vector<double> v{0.0, 1.0, 2.0, 3.0, 4.0};
    std::map<size_t, size_t> m{{0, 1}, {1, 0}};
    std::unordered_map<size_t, size_t> um{{2, 1}, {3, 2}};
    std::set<size_t> s{0, 1, 2, 3, 4};

    data = serial_put(std::tie(v, m, um, s));
  } // scope

  {
    const auto * p = data.data();

    const auto v = serial_get<std::vector<double>>(p);
    ASSERT_EQ(v[0], 0.0);
    ASSERT_EQ(v[1], 1.0);
    ASSERT_EQ(v[2], 2.0);
    ASSERT_EQ(v[3], 3.0);
    ASSERT_EQ(v[4], 4.0);

    const auto m = serial_get<std::map<size_t, size_t>>(p);
    ASSERT_EQ(m.at(0), 1);
    ASSERT_EQ(m.at(1), 0);

    const auto um = serial_get<std::unordered_map<size_t, size_t>>(p);
    ASSERT_EQ(um.at(2), 1);
    ASSERT_EQ(um.at(3), 2);

    const auto s = serial_get<std::set<size_t>>(p);
    ASSERT_NE(s.find(0), s.end());
    ASSERT_NE(s.find(1), s.end());
    ASSERT_NE(s.find(2), s.end());
    ASSERT_NE(s.find(3), s.end());
    ASSERT_NE(s.find(4), s.end());
  } // scope

} // sanity

//----------------------------------------------------------------------------//
// Simple user type.
//----------------------------------------------------------------------------//

struct type_t {
  type_t(size_t id = std::numeric_limits<size_t>::max()) : id_(id) {}

  size_t id() const {
    return id_;
  }

private:
  size_t id_;

}; // struct type_t

TEST(serialize, user_type) {

  std::vector<std::byte> data;

  {
    type_t t0(0);
    type_t t1(1);
    type_t t2(2);
    type_t t3(3);
    type_t t4(4);

    data = serial_put(std::tie(t0, t1, t2, t3, t4));
  } // scope

  {
    const auto * p = data.data();

    ASSERT_EQ(serial_get<type_t>(p).id(), 0);
    ASSERT_EQ(serial_get<type_t>(p).id(), 1);
    ASSERT_EQ(serial_get<type_t>(p).id(), 2);
    ASSERT_EQ(serial_get<type_t>(p).id(), 3);
    ASSERT_EQ(serial_get<type_t>(p).id(), 4);
  } // scope

} // user_type

//----------------------------------------------------------------------------//
// Simple context test.
//----------------------------------------------------------------------------//

struct simple_context_t {

  struct element_info_t {
    size_t id;
  }; // struct element_info_t

  using map_element_t = std::unordered_map<size_t, element_info_t>;

  static simple_context_t & instance() {
    static simple_context_t tc;
    return tc;
  } // instance

  void add_map_element_info(size_t map_identifier,
    size_t element_identifier,
    element_info_t const & info) {
    element_map_[map_identifier][element_identifier] = info;
  } // add_map_element_info

  std::unordered_map<size_t, map_element_t> & element_map() {
    return element_map_;
  } // map_element

  void clear() {
    for(auto e : element_map_) {
      e.second.clear();
    } // for

    element_map_.clear();
  } // clear

private:
  friend serial_convert<simple_context_t>;

  std::unordered_map<size_t, map_element_t> element_map_;

}; // struct simple_context_t

template<>
struct flecsi::utils::serial_convert<simple_context_t> {
  using Rep = decltype(simple_context_t::element_map_);
  static const Rep & put(const simple_context_t & c) {
    return c.element_map_;
  }
  static simple_context_t get(Rep m) {
    simple_context_t ret;
    ret.element_map_.swap(m);
    return ret;
  }
};

TEST(serialize, simple_context) {

  std::vector<std::byte> data;

  simple_context_t & context = simple_context_t::instance();

  {
    simple_context_t::element_info_t info;

    info.id = 10;
    context.add_map_element_info(0, 0, info);

    info.id = 11;
    context.add_map_element_info(0, 1, info);

    info.id = 12;
    context.add_map_element_info(0, 2, info);

    info.id = 13;
    context.add_map_element_info(0, 3, info);

    info.id = 14;
    context.add_map_element_info(0, 4, info);

    info.id = 20;
    context.add_map_element_info(1, 0, info);

    info.id = 21;
    context.add_map_element_info(1, 1, info);

    info.id = 22;
    context.add_map_element_info(1, 2, info);

    info.id = 23;
    context.add_map_element_info(1, 3, info);

    info.id = 24;
    context.add_map_element_info(1, 4, info);

    data = serial_put(context);
  } // scope

  context.clear();

  {
    context = serial_get1<simple_context_t>(data.data());

    auto & element_map = context.element_map();

    {
      auto mita = element_map.find(0);
      ASSERT_NE(mita, element_map.end());
    }

    auto m0 = element_map[0];

#define check_entry(map, key, value)                                           \
  {                                                                            \
    auto ita = map.find(key);                                                  \
    ASSERT_NE(ita, map.end());                                                 \
    ASSERT_EQ(map[key].id, value);                                             \
  }

    check_entry(m0, 0, 10);
    check_entry(m0, 1, 11);
    check_entry(m0, 2, 12);
    check_entry(m0, 3, 13);
    check_entry(m0, 4, 14);

    {
      auto mita = element_map.find(1);
      ASSERT_NE(mita, element_map.end());
    }

    auto m1 = element_map[1];

    check_entry(m1, 0, 20);
    check_entry(m1, 1, 21);
    check_entry(m1, 2, 22);
    check_entry(m1, 3, 23);
    check_entry(m1, 4, 24);

#undef check_entry

  } // scope

} // simple_context
