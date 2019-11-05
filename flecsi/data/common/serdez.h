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

#include <cstring>
#include <istream>
#include <ostream>
#include <stdint.h>

#include <flecsi/data/common/row_vector.h>

namespace flecsi {
namespace data {

template<typename T>
class serdez_u
{
  // not implemented for general case
};

template<typename T>
class serdez_u<row_vector_u<T>>
{
public:
  typedef uint32_t int_t;
  typedef row_vector_u<T> vector_t;
  // note:  FIELD_TYPE and MAX_SERIALIZED_SIZE are required by legion
  typedef vector_t FIELD_TYPE;
  // TODO:  figure out how to make this user-settable
  static const size_t MAX_ENTRIES = 32;
  static const size_t MAX_SERIALIZED_SIZE =
    sizeof(int_t) + MAX_ENTRIES * sizeof(T);

  static size_t serialized_size(const vector_t & val) {
    return sizeof(int_t) + (val.count * sizeof(T));
  }

  // note:  serialize and deserialize could also be implemented using
  //        the Realm::Serialization classes
  static size_t serialize(const vector_t & val, void * buffer) {
    char * buf = (char *)buffer;
    int_t count = val.size();
    std::memcpy(buf, &count, sizeof(int_t));
    buf += sizeof(int_t);
    int_t s = count * sizeof(T);
    if(s)
      std::memcpy(buf, val.data(), s);
    return s + sizeof(int_t);
  }

  static size_t serialize(const vector_t & val, std::ostream & os) {
    int_t count = val.size();
    os.write((char *)&count, sizeof(int_t));
    int_t s = count * sizeof(T);
    if(s)
      os.write((char *)val.data(), s);
    return s + sizeof(int_t);
  }

  static size_t deserialize(vector_t & val, const void * buffer) {
    const char * buf = (char *)buffer;
    int_t count;
    std::memcpy(&count, buf, sizeof(int_t));
    buf += sizeof(int_t);
    val.resize(count);
    int_t s = count * sizeof(T);
    if(s)
      std::memcpy(val.data(), buf, s);
    return s + sizeof(int_t);
  }

  static size_t deserialize(vector_t & val, std::istream & is) {
    int_t count;
    is.read((char *)&count, sizeof(int_t));
    val.resize(count);
    int_t s = count * sizeof(T);
    if(s)
      is.read((char *)val.data(), s);
    return s + sizeof(int_t);
  }

  // not part of the legion serdez interface, but useful for flecsi
  static size_t deep_copy(const vector_t & valin, vector_t & valout) {
    valout = valin;
    return serialized_size(valout);
  }

  static void destroy(vector_t & val) {
    val.clear();
  }
}; // class serdez_u<row_vector_u<T>>

struct serdez_untyped_t {

public:
  serdez_untyped_t(void) {}

  virtual ~serdez_untyped_t(void) {}

  virtual size_t serialize(const void * field_ptr, std::ostream & os) const = 0;

  virtual size_t deserialize(void * field_ptr, std::istream & is) const = 0;

  virtual size_t deep_copy(const void * ptr_in, void * ptr_out) const = 0;

}; // class serdez_untyped_t

template<typename SERDEZ>
struct serdez_wrapper_u : public serdez_untyped_t {

public:
  serdez_wrapper_u(void) {}

  virtual ~serdez_wrapper_u(void) {}

  virtual size_t serialize(const void * field_ptr, std::ostream & os) const {
    using TYPE = typename SERDEZ::FIELD_TYPE;
    auto item_ptr = static_cast<const TYPE *>(field_ptr);
    return SERDEZ::serialize(*item_ptr, os);
  }

  virtual size_t deserialize(void * field_ptr, std::istream & is) const {
    using TYPE = typename SERDEZ::FIELD_TYPE;
    auto item_ptr = static_cast<TYPE *>(field_ptr);
    return SERDEZ::deserialize(*item_ptr, is);
  }

  virtual size_t deep_copy(const void * ptr_in, void * ptr_out) const {
    using TYPE = typename SERDEZ::FIELD_TYPE;
    auto in = static_cast<const TYPE *>(ptr_in);
    auto out = static_cast<TYPE *>(ptr_out);
    return SERDEZ::deep_copy(*in, *out);
  }
}; // class serdez_wrapper_u

} // namespace data
} // namespace flecsi
