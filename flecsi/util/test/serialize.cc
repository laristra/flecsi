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
#include "flecsi/util/serialize.hh"
#include "flecsi/run/context.hh"
#include "flecsi/util/ftest.hh"

#include <limits>

using namespace flecsi::util;

//----------------------------------------------------------------------------//
// Sanity test.
//----------------------------------------------------------------------------//

int
sanity(int, char **) {
  FTEST {
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
      ASSERT_EQ(m.at(0), 1u);
      ASSERT_EQ(m.at(1), 0u);

      const auto um = serial_get<std::unordered_map<size_t, size_t>>(p);
      ASSERT_EQ(um.at(2), 1u);
      ASSERT_EQ(um.at(3), 2u);

      const auto s = serial_get<std::set<size_t>>(p);
      ASSERT_NE(s.find(0), s.end());
      ASSERT_NE(s.find(1), s.end());
      ASSERT_NE(s.find(2), s.end());
      ASSERT_NE(s.find(3), s.end());
      ASSERT_NE(s.find(4), s.end());
    } // scope
  };
} // sanity

ftest_register_driver(sanity);

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

int
user_type(int, char **) {
  FTEST {
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

      ASSERT_EQ(serial_get<type_t>(p).id(), 0u);
      ASSERT_EQ(serial_get<type_t>(p).id(), 1u);
      ASSERT_EQ(serial_get<type_t>(p).id(), 2u);
      ASSERT_EQ(serial_get<type_t>(p).id(), 3u);
      ASSERT_EQ(serial_get<type_t>(p).id(), 4u);
    } // scope
  };
} // user_type

ftest_register_driver(user_type);

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
struct flecsi::util::serial_convert<simple_context_t> {
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

int
simple_context(int, char **) {
  FTEST {
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

      check_entry(m0, 0, 10u);
      check_entry(m0, 1, 11u);
      check_entry(m0, 2, 12u);
      check_entry(m0, 3, 13u);
      check_entry(m0, 4, 14u);

      {
        auto mita = element_map.find(1);
        ASSERT_NE(mita, element_map.end());
      }

      auto m1 = element_map[1];

      check_entry(m1, 0, 20u);
      check_entry(m1, 1, 21u);
      check_entry(m1, 2, 22u);
      check_entry(m1, 3, 23u);
      check_entry(m1, 4, 24u);

#undef check_entry

    } // scope
  };
} // simple_context

ftest_register_driver(simple_context);

//----------------------------------------------------------------------------//
// FleCSI context test.
//----------------------------------------------------------------------------//

int
flecsi_context(int, char **) {
  FTEST {};
} // simple_context

ftest_register_driver(flecsi_context);
