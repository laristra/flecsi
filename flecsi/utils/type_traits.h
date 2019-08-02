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

#include <iterator>
#include <type_traits>

namespace flecsi {
namespace utils {

////////////////////////////////////////////////////////////////////////////////
//! \brief Test to see if all variadic template arguments Ts are of base type.
////////////////////////////////////////////////////////////////////////////////
namespace detail {
template<typename... Conds>
struct and_ : std::true_type {};

//! \brief Combine conditions.
//! \remark Main template expansion function.
template<typename Cond, typename... Conds>
struct and_<Cond, Conds...>
  : std::conditional<Cond::value, and_<Conds...>, std::false_type>::type {};
} // namespace detail

template<typename Base, typename... Ts>
using are_base_of_t =
  detail::and_<std::is_base_of<std::decay_t<Base>, std::decay_t<Ts>>...>;

////////////////////////////////////////////////////////////////////////////////
// A type trait utility to detect if a type is a strict STL container.
////////////////////////////////////////////////////////////////////////////////

namespace detail {

//! \brief A helper type to check if a particular type is a container.
template<typename... Ts>
struct is_container {};

} // namespace detail

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
  std::conditional_t<false,
    detail::is_container<typename T::value_type,
      typename T::size_type,
      typename T::allocator_type,
      typename T::iterator,
      typename T::const_iterator,
      decltype(std::declval<T>().size()),
      decltype(std::declval<T>().begin()),
      decltype(std::declval<T>().end()),
      decltype(std::declval<T>().cbegin()),
      decltype(std::declval<T>().cend()),
      decltype(std::declval<T>().data())>,
    void>> : public std::true_type {};

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
  std::conditional_t<false,
    is_container<decltype(std::declval<T>().size()),
      decltype(std::declval<T>().data())>,
    void>> : public std::true_type {};

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
  std::conditional_t<false,
    is_container<decltype(std::declval<T>().begin()),
      decltype(std::declval<T>().end())>,
    void>> : public std::true_type {};

//! \brief Equal to true if T is a container.
//! \remark This version uses to a reduced set of requirements for a container.
template<typename T>
constexpr bool is_iterative_container_v = is_iterative_container<T>::value;

////////////////////////////////////////////////////////////////////////////////
//! \brief Check if a particular type T is an iterator.
////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct is_iterator {
  //! \brief This function will get always get instantiated.
  static char test(...);
  //! \brief Use SFINAE to create this function if T is an iterator.
  template<typename U,
    typename = typename std::iterator_traits<U>::difference_type,
    typename = typename std::iterator_traits<U>::pointer,
    typename = typename std::iterator_traits<U>::reference,
    typename = typename std::iterator_traits<U>::value_type,
    typename = typename std::iterator_traits<U>::iterator_category>
  static long test(U &&);
  //! \breif True if T is an iterator and test(U) function exists.
  constexpr static bool value =
    std::is_same<decltype(test(std::declval<T>())), long>::value;
};

//! \brief Equal to true if T is an iterator.
template<typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;
} // namespace utils
} // namespace flecsi
