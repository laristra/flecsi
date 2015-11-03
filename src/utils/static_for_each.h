/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_static_for_each_h
#define flexi_static_for_each_h

#include <type_traits>
#include <tuple>

/*!
 * \file static_for_each.h
 * \authors bergen
 * \date Initial file creation: Oct 22, 2015
 */

namespace flexi {

// convenience define
template<typename TupleType>
using __flexi_integral_constant_t = std::integral_constant<size_t,
   std::tuple_size<typename std::remove_reference<TupleType>::type>::value>;

// Empty sequence call to terminate
template<typename TupleType, typename FunctionType>
void static_for_each(TupleType &&, FunctionType,
   __flexi_integral_constant_t<TupleType>) {}

// Expand each index
template<
   size_t I,
   typename TupleType,
   typename FunctionType,
   typename = typename std::enable_if<I!=std::tuple_size<
      typename std::remove_reference<TupleType>::type>::value>::type
>
void static_for_each(TupleType && t, FunctionType f,
   std::integral_constant<size_t, I>) {
      f(std::get<I>(t));
      static_for_each(std::forward<TupleType>(t), f,
         std::integral_constant<size_t, I + 1>());
} // static_for_each

// Main interface
template<typename TupleType, typename FunctionType>
void static_for_each(TupleType && t, FunctionType f)
{
    static_for_each(std::forward<TupleType>(t), f,
      std::integral_constant<size_t, 0>());
} // static_for_each

} // namespace flexi

#endif // flexi_static_for_each_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
