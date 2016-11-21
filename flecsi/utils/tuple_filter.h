/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

/*----------------------------------------------------------------------------*
 * Attribution: Implementation based on response to stackoverflow question
 * http://stackoverflow.com/questions/38688365/
 * how-can-i-create-a-tuple-instance-from-another-tuple-
 * instance-that-statically-se
 *----------------------------------------------------------------------------*/

#ifndef flecsi_tuple_filter_h
#define flecsi_tuple_filter_h

#include<tuple>

/*!
 * \file tuple_filter.h
 * \authors bergen
 * \date Initial file creation: Jul 31, 2016
 */

namespace flecsi {

/*!
  Terminator.
 */
template<typename T>
struct as_sequence_;

/*!
  Create an index sequence from the elements of a variadic list.
 */
template<typename ... Ts>
struct as_sequence_<std::tuple<Ts ...>> {
  using type = std::index_sequence<Ts::value ...>;
}; // struct as_sequence_

/*!
  Terminator.
 */
template<template<typename> typename P, typename T, typename S>
struct make_filtered_sequence_;

/*!
  Terminator.
 */
template<template<typename> typename P, typename T, typename S>
struct make_filtered_sequence_index_;

/*!
  Apply a preficate to filter the types of a tuple.
 */
template<template<typename> typename P, typename T, std::size_t ... Is>
struct make_filtered_sequence_<P, T, std::index_sequence<Is ...>> {
  using type = typename as_sequence_<decltype(std::tuple_cat(
    std::conditional_t<
      P<std::tuple_element_t<Is, T>>::value,
      std::tuple<std::integral_constant<std::size_t, Is>>,
      std::tuple<>>{} ...))>::type;
}; // struct make_filtered_sequence

/*!
  Apply a predicate to filter the indices of a tuple.
 */
template<template<typename> typename P, typename T, std::size_t ... Is>
struct make_filtered_sequence_index_<P, T, std::index_sequence<Is ...>> {
  using type = typename as_sequence_<decltype(std::tuple_cat(
    std::conditional_t<
      P<std::integral_constant<std::size_t, Is>>::value,
      std::tuple<std::integral_constant<std::size_t, Is>>,
      std::tuple<>>{} ...))>::type;
}; // struct make_filtered_sequence

/*!
  Use a filter sequence to get elements from an input tuple.
 */
template<typename T, std::size_t ... Is>
auto filter_impl_(const T & t, std::index_sequence<Is ...>)
-> std::tuple<std::tuple_element_t<Is, T> ...> {
  return { std::get<Is>(t) ... };
} // filter_impl_

/*!
  Apply predicate to filter tuple.
 */
template<template<typename> typename P, typename T>
auto tuple_filter_(const T & t) {
  using filtered_sequence = typename make_filtered_sequence_<P, T,
    std::make_index_sequence<std::tuple_size<T>::value>>::type;

  return filter_impl_(t, filtered_sequence());
} // tuple_filter_

/*!
  Apply predicate to filter indices of tuple.
 */
template<template<typename> typename P, typename T>
auto tuple_filter_index_(const T & t) {
  using filtered_sequence = typename make_filtered_sequence_index_<P, T,
    std::make_index_sequence<std::tuple_size<T>::value>>::type;

  return filter_impl_(t, filtered_sequence());
} // tuple_filter_index_

} // namespace flecsi

#endif // flecsi_tuple_filter_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
