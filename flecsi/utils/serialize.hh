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
#pragma once

#include <cstddef>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <utility> // declval
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

#include "flog.hh"
#include "type_traits.hh"

namespace flecsi {
namespace utils {

/*!
  The binary_serializer_t type provides dynamic serialization of arbitrary
  types through an insertion-style operator.

  Attribution:
  https://stackoverflow.com/questions/3015582/direct-boost-serialization-to-char-array/5604782
 */

struct binary_serializer_t {

  binary_serializer_t()
    : inserter_(serial_data_), stream_(inserter_), oa_(stream_) {}

  /*!
    Insertion operator to write a value to the underlying string device.
   */

  template<typename T>
  binary_serializer_t & operator<<(T const & value) {
    oa_ << value;
    return *this;
  } // operator<<

  /*!
    Synchronize with the underlying storage device.
   */

  void flush() {
    stream_.flush();
  }

  /*!
    Reset the string device.
   */

  void clear() {
    serial_data_.clear();
  }

  /*!
    Return the number of bytes stored in the string device.
   */

  size_t bytes() {
    return serial_data_.size();
  } // size

  /*!
    Return a pointer to the string device data.
   */

  char const * data() const {
    return serial_data_.data();
  } // data

private:
  std::string serial_data_;
  boost::iostreams::back_insert_device<std::string> inserter_;
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>>
    stream_;
  boost::archive::binary_oarchive oa_;

}; // struct binary_serializer_t

/*!
  The binary_deserializer_t type provides dynamic de-serialization of arbitrary
  types through an extraction-style operator.

  Attribution:
  https://stackoverflow.com/questions/3015582/direct-boost-serialization-to-char-array/5604782
 */

struct binary_deserializer_t {

  /*!
    Construct a deserializer from the given data pointer and byte size. Note
    that the buffer must have been created with an instance of
    binary_serializer_t.
   */

  binary_deserializer_t(char const * data, size_t bytes)
    : device_(data, bytes), stream_(device_), ia_(stream_) {}

  /*!
    Extraction operator to write a value to the underlying string device.
   */

  template<typename T>
  binary_deserializer_t & operator>>(T & value) {
    ia_ >> value;
    return *this;
  } // operator>>

private:
  boost::iostreams::basic_array_source<char> device_;
  boost::iostreams::stream<boost::iostreams::basic_array_source<char>> stream_;
  boost::archive::binary_iarchive ia_;

}; // struct binary_deserializer_t

// Similar to that in GNU libc.  NB: memcpy has no alignment requirements.
inline void
mempcpy(std::byte *& d, const void * s, std::size_t n) {
  std::memcpy(d, s, n);
  d += n;
}
// For size precalculation:
inline void
mempcpy(std::size_t & x, const void *, std::size_t n) {
  x += n;
}

template<class, class = void>
struct serial;

template<class T, class P>
void
serial_put(P & p, const T & t) {
  serial<std::remove_const_t<T>>::put(p, t);
}
template<class T>
std::size_t
serial_size(const T & t) {
  std::size_t ret = 0;
  serial_put(ret, t);
  return ret;
}
template<class T>
T
serial_get(const std::byte *& p) {
  return serial<std::remove_const_t<T>>::get(p);
}

template<class T>
auto serial_put(const T & t) { // for a single object
  std::vector<std::byte> ret(serial_size(t));
  auto *const p0 = ret.data(), *p = p0;
  serial_put(p, t);
  flog_assert(p == p0 + ret.size(), "Wrong serialization size");
  return ret;
}
template<class T>
T serial_get1(const std::byte * p) { // for a single object
  return serial_get<T>(p);
}

template<class T>
constexpr bool memcpyable_v =
  std::is_default_constructible_v<T> && std::is_trivially_move_assignable_v<T>;

template<class T>
struct serial<T, std::enable_if_t<memcpyable_v<T>>> {
  static_assert(!std::is_pointer_v<T>, "Cannot serialize pointers");
  template<class P>
  static void put(P & p, const T & t) {
    mempcpy(p, &t, sizeof t);
  }
  static T get(const std::byte *& p) {
    T ret;
    std::memcpy(&ret, p, sizeof ret);
    p += sizeof ret;
    return ret;
  }
};
// To allow convenient serial_put(std::tie(...)), it is part of the interface
// that tuple elements are just concatenated.
template<class... TT>
struct serial<std::tuple<TT...>,
  std::enable_if_t<!memcpyable_v<std::tuple<TT...>>>> {
  using type = std::tuple<TT...>;
  template<class P>
  static void put(P & p, const type & t) {
    std::apply([&p](const TT &... xx) { (serial_put(p, xx), ...); }, t);
  }
  static type get(const std::byte *& p) {
    return type{serial_get<TT>(p)...};
  }
};

// Adapters for other protocols:

// This works even without Legion:
template<class T>
struct serial<T,
  voided<decltype(&T::legion_buffer_size),
    std::enable_if_t<!memcpyable_v<T>>>> {
  template<class P>
  static void put(P & p, const T & t) {
    if constexpr(std::is_pointer_v<P>)
      p += t.legion_serialize(p);
    else
      p += t.legion_buffer_size();
  }
  static T get(const std::byte *& p) {
    T ret;
    p += ret.legion_deserialize(p);
    return ret;
  }
};

// Should define put and get and optionally size:
template<class>
struct serial_convert;
template<class T, class = void>
struct serial_convert_traits : serial_convert<T> {
  static std::size_t size(const T & t) {
    return serial_size(serial_convert<T>::put(t));
  }
};
template<class T>
struct serial_convert_traits<T, decltype(void(serial_convert<T>::size))>
  : serial_convert<T> {};
template<class T>
struct serial<T, decltype(void(serial_convert<T>::put))> {
  using Convert = serial_convert_traits<T>;
  template<class P>
  static void put(P & p, const T & t) {
    if constexpr(std::is_pointer_v<P>)
      serial_put(p, Convert::put(t));
    else
      p += Convert::size(t);
  }
  static T get(const std::byte *& p) {
    return Convert::get(
      serial_get<std::decay_t<decltype(Convert::put(std::declval<T>()))>>(p));
  }
};

} // namespace utils
} // namespace flecsi
