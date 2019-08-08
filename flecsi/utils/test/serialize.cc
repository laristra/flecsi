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
#include "flecsi/runtime/context.hh"
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/serialize.hh>

#include <limits>

using namespace flecsi::utils;

//----------------------------------------------------------------------------//
// Sanity test.
//----------------------------------------------------------------------------//

int
sanity(int argc, char ** argv) {

  FTEST();

  char * data{nullptr};
  size_t size{0};

  {
    std::vector<double> v{0.0, 1.0, 2.0, 3.0, 4.0};
    std::map<size_t, size_t> m{{0, 1}, {1, 0}};
    std::unordered_map<size_t, size_t> um{{2, 1}, {3, 2}};
    std::set<size_t> s{0, 1, 2, 3, 4};

    binary_serializer_t serializer;

    serializer << v;
    serializer << m;
    serializer << um;
    serializer << s;

    serializer.flush();

    size = serializer.bytes();
    data = new char[size];
    memcpy(data, serializer.data(), size);
  } // scope

  {
    std::vector<double> v;
    std::map<size_t, size_t> m;
    std::unordered_map<size_t, size_t> um;
    std::set<size_t> s;

    binary_deserializer_t deserializer(data, size);

    deserializer >> v;

    ASSERT_EQ(v[0], 0.0);
    ASSERT_EQ(v[1], 1.0);
    ASSERT_EQ(v[2], 2.0);
    ASSERT_EQ(v[3], 3.0);
    ASSERT_EQ(v[4], 4.0);

    deserializer >> m;

    ASSERT_EQ(m[0], 1);
    ASSERT_EQ(m[1], 0);

    deserializer >> um;

    ASSERT_EQ(um[2], 1);
    ASSERT_EQ(um[3], 2);

    deserializer >> s;

    ASSERT_NE(s.find(0), s.end());
    ASSERT_NE(s.find(1), s.end());
    ASSERT_NE(s.find(2), s.end());
    ASSERT_NE(s.find(3), s.end());
    ASSERT_NE(s.find(4), s.end());
  } // scope

  delete[] data;

  return 0;
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
  friend class boost::serialization::access;

  template<typename ARCHIVE_TYPE>
  void serialize(ARCHIVE_TYPE & ar, const unsigned int version) {
    ar & id_;
  } // serialize

  size_t id_;

}; // struct type_t

int
user_type(int argc, char ** argv) {

  FTEST();

  char * data{nullptr};
  size_t size{0};

  {
    type_t t0(0);
    type_t t1(1);
    type_t t2(2);
    type_t t3(3);
    type_t t4(4);

    binary_serializer_t serializer;

    serializer << t0;
    serializer << t1;
    serializer << t2;
    serializer << t3;
    serializer << t4;

    serializer.flush();

    size = serializer.bytes();
    data = new char[size];
    memcpy(data, serializer.data(), size);
  } // scope

  {
    type_t t;

    binary_deserializer_t deserializer(data, size);

    deserializer >> t;
    ASSERT_EQ(t.id(), 0);

    deserializer >> t;
    ASSERT_EQ(t.id(), 1);

    deserializer >> t;
    ASSERT_EQ(t.id(), 2);

    deserializer >> t;
    ASSERT_EQ(t.id(), 3);

    deserializer >> t;
    ASSERT_EQ(t.id(), 4);
  } // scope

  return 0;
} // user_type

ftest_register_driver(user_type);

//----------------------------------------------------------------------------//
// Simple context test.
//----------------------------------------------------------------------------//

struct simple_context_t {

  struct element_info_t {
    size_t id;

    template<typename ARCHIVE_TYPE>
    void serialize(ARCHIVE_TYPE & ar, const unsigned int version) {
      ar & id;
    } // serialize

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
  friend class boost::serialization::access;

  template<typename ARCHIVE_TYPE>
  void serialize(ARCHIVE_TYPE & ar, const unsigned int version) {
    ar & element_map_;
  } // serialize

  std::unordered_map<size_t, map_element_t> element_map_;

}; // struct simple_context_t

int
simple_context(int argc, char ** argv) {

  FTEST();

  char * data{nullptr};
  size_t size{0};

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

    binary_serializer_t serializer;

    serializer << context;

    serializer.flush();

    size = serializer.bytes();
    data = new char[size];
    memcpy(data, serializer.data(), size);
  } // scope

  context.clear();

  {
    binary_deserializer_t deserializer(data, size);

    deserializer >> context;

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

  delete[] data;

  return 0;
} // simple_context

ftest_register_driver(simple_context);

//----------------------------------------------------------------------------//
// FleCSI context test.
//----------------------------------------------------------------------------//

int
flecsi_context(int argc, char ** argv) {
  return 0;
} // simple_context

ftest_register_driver(flecsi_context);
