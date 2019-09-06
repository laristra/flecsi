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
#pragma once

/*! @file */

#include <algorithm>
#include <stdint.h>

#include <flecsi/data/common/simple_vector.h>

namespace flecsi {
namespace data {

template<typename T>
class serdez_u
{
  // not implemented for general case
};

template<typename T>
class serdez_u<simple_vector_u<T>>
{
public:
  typedef simple_vector_u<T> vector_t;
  // note:  following two defs are required by legion
  typedef vector_t FIELD_TYPE;
  static const size_t MAX_SERIALIZED_SIZE = 4096;

  static size_t serialized_size(const vector_t & val) {
    return sizeof(int) + (val.count * sizeof(T));
  }

  // note:  serialize and deserialize could also be implemented using
  //        the Realm::Serialization classes
  static size_t serialize(const vector_t & val, void * buffer) {
    char * buf = (char *)buffer;
    int count = val.size();
    memcpy(buf, &count, sizeof(int));
    buf += sizeof(int);
    int s = count * sizeof(T);
    if(s && val.data)
      memcpy(buf, val.data, s);
    return s + sizeof(int);
  }

  static size_t deserialize(vector_t & val, const void * buffer) {
    const char * buf = (char *)buffer;
    int count;
    memcpy(&count, buf, sizeof(int));
    buf += sizeof(int);
    // some C++ magic to construct object in uninitialized memory
    // CRF:  this assumes val is always uninitialized - need to verify
    new(&val) vector_t(count);
    int s = count * sizeof(T);
    if(s && val.data)
      memcpy(val.data, buf, s);
    return s + sizeof(int);
  }

  static void destroy(vector_t & val) {
    val.clear();
  }
};

} // namespace data
} // namespace flecsi
