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

/*! @file */

#include <type_traits>

namespace flecsi {
namespace utils {

////////////////////////////////////////////////////////////////////////////////
// A type trait utility to detect if a type is a strict STL container.
////////////////////////////////////////////////////////////////////////////////

namespace detail {

template<typename... Ts>
struct hold {};

// Adapted from https://stackoverflow.com/questions/25845536/
template<template<class...> class B, class D>
struct base_specialization {
  template<class... AA>
  static B<AA...> test(B<AA...> *);
  static void test(void *);

  using type = decltype(test(static_cast<D *>(nullptr)));
};

template<class T>
struct nonvoid {
  using type = T;
};
template<>
struct nonvoid<void> {};

} // namespace detail

// Workaround for Clang's eager reduction of void_t (see also CWG1980)
template<class... TT>
using voided = std::conditional_t<false, detail::hold<TT...>, void>;

template<template<class...> class B, class D>
struct base_specialization
  : detail::nonvoid<typename detail::base_specialization<B, D>::type> {};
template<template<class...> class B, class D>
using base_specialization_t = typename base_specialization<B, D>::type;

//! \brief Check if a particular type T is a container.
//! \remark If T is not, this version is instantiated.
//! \remark This version adheres to the strict requirements of an STL container.
template<typename T, typename _ = void>
struct is_container : std::false_type {};

//! \brief Check if a particular type T is a container.
//! \remark If T is, this version is instantiated.
//! \remark This version adheres to the strict requirements of an STL container.
template<typename T>
struct is_container<T,
  voided<typename T::value_type,
    typename T::size_type,
    typename T::allocator_type,
    typename T::iterator,
    typename T::const_iterator,
    decltype(std::declval<T>().size()),
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    decltype(std::declval<T>().cbegin()),
    decltype(std::declval<T>().cend()),
    decltype(std::declval<T>().data())>> : public std::true_type {};

//! \brief Equal to true if T is a container.
//! \remark This version adheres to the strict requirements of an STL container.
template<typename T>
constexpr bool is_container_v = is_container<T>::value;

////////////////////////////////////////////////////////////////////////////////
// A type trait utility to detect if a type is a minimal container.
//
// Testing is more relaxed than is_container, it only needs to have a
// size and data memeber function..
////////////////////////////////////////////////////////////////////////////////

/// \brief A Helper to identify if this is a container
//! \remark If T is, this version is instantiated.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T, typename _ = void>
struct is_minimal_container : std::false_type {};

/// \brief A Helper to identify if this is a container
//! \remark If T is, this version is instantiated.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T>
struct is_minimal_container<T,
  voided<decltype(std::declval<T>().size()),
    decltype(std::declval<T>().data())>> : public std::true_type {};

//! \brief Equal to true if T is a container.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T>
constexpr bool is_minimal_container_v = is_minimal_container<T>::value;

////////////////////////////////////////////////////////////////////////////////
// A type trait utility to detect if a type is a minimal container.
//
// Testing is more relaxed than is_container, it only needs to have a
// begin and end function.
////////////////////////////////////////////////////////////////////////////////

/// \brief A Helper to identify if this is a container
//! \remark If T is, this version is instantiated.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T, typename _ = void>
struct is_iterative_container : std::false_type {};

/// \brief A Helper to identify if this is a container
//! \remark If T is, this version is instantiated.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T>
struct is_iterative_container<T,
  voided<decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())>> : public std::true_type {};

//! \brief Equal to true if T is a container.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T>
constexpr bool is_iterative_container_v = is_iterative_container<T>::value;

} // namespace utils
} // namespace flecsi
