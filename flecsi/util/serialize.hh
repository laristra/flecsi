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
#include <cstring> // memcpy
#include <map>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility> // declval
#include <vector>

#include "type_traits.hh"
#include <flecsi/flog.hh>

namespace flecsi {
namespace util {

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

// Store t at p, advancing past its serialized form.
// For calculating sizes, P should be std::size_t.
// For actual serialization, P should be std::byte*.
template<class T, class P>
void
serial_put(P & p, const T & t) {
  serial<std::remove_const_t<T>>::put(p, t);
}
template<class T>
std::size_t serial_size(
  const T & t) { // wrapper to provide an initial size of 0
  std::size_t ret = 0;
  serial_put(ret, t);
  return ret;
}
template<class T>
T serial_get(const std::byte *& p) { // reconstruct and advance past an object
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

struct serial_cast {
  const std::byte *& p;
  template<class T>
  operator T() const {
    return serial_get<T>(p);
  }
};

namespace detail {
template<class T>
struct serial_container {
  template<class P>
  static void put(P & p, const T & c) {
    serial_put(p, c.size());
    for(auto & t : c)
      serial_put(p, t);
  }
  static T get(const std::byte *& p) {
    T ret;
    for(auto n = serial_get<typename T::size_type>(p); n--;)
      ret.insert(serial_get<typename T::value_type>(p));
    return ret;
  }
};
} // namespace detail

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
    // Suppress -Wclass-memaccess: the default constructor needn't be trivial!
    std::memcpy(static_cast<void *>(&ret), p, sizeof ret);
    p += sizeof ret;
    return ret;
  }
};
// To allow convenient serial_put(std::tie(...)), it is part of the interface
// that pair and tuple elements are just concatenated.
template<class T, class U>
struct serial<std::pair<T, U>,
  std::enable_if_t<!memcpyable_v<std::pair<T, U>>>> {
  using type = std::pair<T, U>;
  template<class P>
  static void put(P & p, const type & v) {
    serial_put(p, v.first);
    serial_put(p, v.second);
  }
  static type get(const std::byte *& p) {
    return {serial_get<T>(p), serial_get<U>(p)};
  }
};
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
template<class T>
struct serial<std::vector<T>> {
  using type = std::vector<T>;
  template<class P>
  static void put(P & p, const type & v) {
    serial_put(p, v.size());
    for(auto & t : v)
      serial_put(p, t);
  }
  static type get(const std::byte *& p) {
    auto n = serial_get<typename type::size_type>(p);
    type ret;
    ret.reserve(n);
    while(n--)
      ret.push_back(serial_get<T>(p));
    return ret;
  }
};
template<class T>
struct serial<std::set<T>> : detail::serial_container<std::set<T>> {};
template<class K, class V>
struct serial<std::map<K, V>> : detail::serial_container<std::map<K, V>> {};
template<class K, class V>
struct serial<std::unordered_map<K, V>>
  : detail::serial_container<std::unordered_map<K, V>> {};
template<>
struct serial<std::string> {
  template<class P>
  static void put(P & p, const std::string & s) {
    const auto n = s.size();
    serial_put(p, n);
    mempcpy(p, s.data(), n);
  }
  static std::string get(const std::byte *& p) {
    const auto n = serial_get<std::string::size_type>(p);
    const auto d = p;
    p += n;
    return {reinterpret_cast<const char *>(d), n};
  }
};

// Adapters for other protocols:

template<class T>
struct serial_value { // serializes nothing; returns T()
  using type = T;
  template<class P>
  static void put(P &, const T &) {}
  static T get(const std::byte *&) {
    return T();
  }
};

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

} // namespace util
} // namespace flecsi
