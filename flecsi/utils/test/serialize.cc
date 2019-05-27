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

#include <flecsi/utils/ftest.h>
#include <flecsi/utils/serialize.h>

using namespace flecsi::utils;

int
serialize(int argc, char ** argv) {

  FTEST();

  char * data{nullptr};
  size_t size{0};

  {
  std::vector<double> v{ 0.0, 1.0, 2.0, 3.0, 4.0 };
  std::map<size_t, size_t> m{ { 0, 1 }, { 1, 0 } };
  std::unordered_map<size_t, size_t> um{ { 2, 1 }, { 3, 2 } };
  std::set<size_t> s{ 0, 1, 2, 3, 4 };

  binary_serializer_t serializer;

  serializer << v;
  serializer << m;
  serializer << um;
  serializer << s;

  serializer.flush();

  size = serializer.size();
  data = new char[size];
  memcpy(data, serializer.data(), size);
  }

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
  }

  delete[] data;

  return 0;
}

ftest_register_test(serialize);
