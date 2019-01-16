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

// system headers
#include <iostream>
#include <tuple>

namespace flecsi {
namespace utils {

namespace detail {

////////////////////////////////////////////////////////////////////////////////
/// \brief A helper to identify if all types Ts are integral.
//! \remark If they are not, this version is instantiated.
////////////////////////////////////////////////////////////////////////////////
template<typename... Ts>
class are_integral : public std::integral_constant<bool, true>
{};

/// \brief A helper to identify if all types Ts are integral.
//! \remark If they are, this version is instantiated.
template<typename T, typename... Ts>
class are_integral<T, Ts...>
  : public std::integral_constant<bool,
      (std::is_integral<T>::value && are_integral<Ts...>::value)>
{};

//! Equal to true if Ts are all integral types.
template<typename... Ts>
constexpr bool are_integral_v = are_integral<Ts...>::value;

////////////////////////////////////////////////////////////////////////////////
/// \brief Comparison function for two tuples
/// Comparison is done in a lexical fashion.
/// \param[in] a,b  The two tuples to compare
/// \return true if a < b
/// \tparam I the tuple index to check
/// \tparam TupleA,TupleB  The types of the two tuples (dont have to match)
////////////////////////////////////////////////////////////////////////////////

// This function terminates the chain
template<std::size_t I = 0,
  typename TupleA,
  typename TupleB,
  std::enable_if_t<std::tuple_size<TupleA>::value ==
                     std::tuple_size<TupleB>::value &&
                   I == std::tuple_size<TupleA>::value - 1> * = nullptr>
bool
less_than(const TupleA & a, const TupleB & b) {
  return (std::get<I>(a) < std::get<I>(b));
}

// this function is the main one
template<std::size_t I = 0,
  typename TupleA,
  typename TupleB,
  typename = std::enable_if_t<std::tuple_size<TupleA>::value ==
                                std::tuple_size<TupleB>::value &&
                              (I < std::tuple_size<TupleA>::value - 1) &&
                              std::tuple_size<TupleA>::value >= 2>>
bool
less_than(const TupleA & a, const TupleB & b) {
  constexpr auto N = std::tuple_size<TupleA>::value;
  if(std::get<I>(a) == std::get<I>(b))
    return less_than<I + 1>(a, b);
  else
    return (std::get<I>(a) < std::get<I>(b));
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Comparison function for two tuples
/// \param[in] a,b  The two tuples to compare
/// \return true if a = b
/// \tparam I the tuple index to check
/// \tparam TupleA,TupleB  The types of the two tuples (dont have to match)
////////////////////////////////////////////////////////////////////////////////

// this one terminates the chain
template<std::size_t I = 0,
  typename TupleA,
  typename TupleB,
  std::enable_if_t<std::tuple_size<TupleA>::value ==
                     std::tuple_size<TupleB>::value &&
                   I == std::tuple_size<TupleA>::value - 1> * = nullptr>
bool
equal_to(const TupleA & a, const TupleB & b) {
  return (std::get<I>(a) == std::get<I>(b));
}

// this is the main routine
template<std::size_t I = 0,
  typename TupleA,
  typename TupleB,
  typename = std::enable_if_t<std::tuple_size<TupleA>::value ==
                                std::tuple_size<TupleB>::value &&
                              (I < std::tuple_size<TupleA>::value - 1) &&
                              std::tuple_size<TupleA>::value >= 2>>
bool
equal_to(const TupleA & a, const TupleB & b) {
  constexpr auto N = std::tuple_size<TupleA>::value;
  auto test = (std::get<I>(a) < std::get<I>(b));
  if(std::get<I>(a) != std::get<I>(b))
    return false;
  else
    return equal_to<I + 1>(a, b);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Print function for a tuple
/// \param[in] a  The tuple to print.
/// \return A reference to the output stream
/// \tparam I the tuple index to print
/// \tparam TupleA  The type of the tuples
////////////////////////////////////////////////////////////////////////////////

// this routine terminates the chain
template<std::size_t I = 0,
  typename TupleA,
  std::enable_if_t<I == std::tuple_size<TupleA>::value - 1> * = nullptr>
std::ostream &
print(std::ostream & output, const TupleA & a, const char *) {
  output << std::get<I>(a);
  return output;
}

// the main calling routine
template<std::size_t I = 0,
  typename TupleA,
  typename = std::enable_if_t<(I < std::tuple_size<TupleA>::value - 1) &&
                              std::tuple_size<TupleA>::value >= 2>>
std::ostream &
print(std::ostream & output, const TupleA & a, const char * sep) {
  output << std::get<I>(a) << sep;
  return print<I + 1>(output, a, sep);
}

} // namespace detail

////////////////////////////////////////////////////////////////////////////////
//! This struct provides a simple lexical comparison
//! \tparam T  The type to perform the lexical comparison
//! \remark This version is the empty one
////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct lexical_comparison {};

//! Specialized version of lexical_comparison accepting tuples
//! \tparam Args  The tuple argument types
template<typename... Args>
struct lexical_comparison<std::tuple<Args...>> {
  using value_type = typename std::tuple<Args...>;
  bool operator()(const value_type & a, const value_type & b) const {
    return detail::less_than(a, b);
  }
};

////////////////////////////////////////////////////////////////////////////////
/// \brief A simple id type that can be constructed from multiple indexes.
///
/// Working towards simplifying/generalizing the id_t type that is used
/// throughout flecsi.  This one is currently only used for the intermediate
/// binding mappings.
///
/// \tparam Args the different integral types that form the overall id.
////////////////////////////////////////////////////////////////////////////////

template<typename T, typename Compare>
class simple_id_t
{};

template<template<typename> class Compare, typename... Args>
class simple_id_t<std::tuple<Args...>, Compare<std::tuple<Args...>>>
{

  //===========================================================================
  // Private data members
  //===========================================================================

  //! The underlying data type
  using value_type = std::tuple<Args...>;

  //! the tuple to store the data
  value_type data_;

  //! the length of the tuple
  static constexpr auto length_ = sizeof...(Args);

public:
  //===========================================================================
  // Public constructors
  //===========================================================================

  //! force the default constructor
  simple_id_t() = default;

  //! force the default copy constructor
  simple_id_t(const simple_id_t &) = default;

  //! constructor using tuple
  //! \param [in] data  The tuple to copy
  simple_id_t(const value_type & data) : data_{data} {}

  //! In place constructor
  //! \param [in] ts  The different elements of the tuple
  //! \tparam Ts  The different typle types (they can differ from original
  //!             data type)
  template<typename... Ts,
    typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Args)>>
  simple_id_t(Ts &&... ts) : data_{std::make_tuple(std::forward<Ts>(ts)...)} {}

  //! Return the size of the tuple
  static constexpr auto size() noexcept {
    return length_;
  }

  //! The output operator
  //! \param[in,out] output  the output stream
  //! \param[in] id  the id to print
  //! \return a reference to the output stream
  friend std::ostream & operator<<(std::ostream & output,
    const simple_id_t & id) {
    detail::print(output, id.data_, ", ");
    return output;
  }

  //! Check for equality
  //! \param [in] id  the id to test agains this object
  //! \return true if this=id
  bool operator==(const simple_id_t & id) const {
    return detail::equal_to(data_, id.data_);
  }

  //! Check for equality
  //! \param [in] id  the id to test agains this object
  //! \return true if this=id
  bool operator<(const simple_id_t & id) const {
    return Compare<value_type>{}(data_, id.data_);
  }

}; // simple_id

} // namespace utils
} // namespace flecsi
