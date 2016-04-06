/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file static_list.h
 * 
 * \brief A static list that can be checked at compile time.
 *
 ******************************************************************************/
#pragma once

//! user includes
//#include "detail/static_list.h"

namespace flecsi {
namespace utils {



  /// Identity functor
  template <typename X>
  struct id {
    using type = X;
  };

  /// Create a meta functor that returns `X` when invoked with any argument.
  template <typename X>
  struct constant {
    template <typename>
    struct apply {
      using type = X;
    };
  };

  /// End of list marker type
  struct Nil { };

  /// Lazy list of values
  template <typename head, template<typename> class get_rest>
  struct List {
    using first = head;
    using rest = typename get_rest<head>::type;
  };

  /// Get head of list `L`.
  template <typename L>
  using car = typename L::first;

  /// Get rest of list `L`.
  template <typename L>
  using cdr = typename L::rest;

  // Prepend element `X` on to a list `L`.
  template <typename X, typename L>
  using cons = List<X, constant<L>::template apply>;

  /// Create a list from a parameter pack.
  template <typename...>
  struct ListFromParams;

  template <typename X, typename... XS>
  struct ListFromParams<X, XS...> {
    using type = cons<X, typename ListFromParams<XS...>::type>;
  };

  template <>
  struct ListFromParams<> {
    using type = Nil;
  };

  template <typename... elements>
  using list_from_params = typename ListFromParams<elements...>::type;

  /// Append list `L1` onto `L2`.
  template <typename L1, typename L2>
  struct ListConcat {
    template <typename>
    struct ConcatImpl {
      using type = typename ListConcat<cdr<L1>, L2>::type;
    };

    using type = List<car<L1>, ConcatImpl>;
  };

  template <typename L2>
  struct ListConcat<Nil, L2> {
    using type = L2;
  };

  template <typename L1, typename L2>
  using concat = typename ListConcat<L1, L2>::type;

  /// Reverse a list.
  template <typename L>
  struct Reverse {
    using type = concat<
      typename Reverse<cdr<L>>::type,
      cons<car<L>, Nil>>;
  };

  template<>
  struct Reverse<Nil> {
    using type = Nil;
  };

  template <typename L>
  using reverse = typename Reverse<L>::type;

  /// Create a list of at most `count` elements from list `L`.
  template <size_t count, typename L>
  struct ListTake {
    template <typename>
    struct TakeImpl {
      using type = typename ListTake<count - 1, cdr<L>>::type;
    };

    using type = List<car<L>, TakeImpl>;
  };

  template <typename L>
  struct ListTake<0, L> {
    using type = Nil;
  };

  template <size_t count>
  struct ListTake<count, Nil> {
    using type = Nil;
  };

  template <size_t count, typename L>
  using take = typename ListTake<count, L>::type;


  /// Perform a map operation on a list
  template <template<typename> class F, typename L>
  struct ListMap {
    template <typename>
    struct MapImpl {
      using type = typename ListMap<F, cdr<L>>::type;
    };
    
    using type = List<typename F<car<L>>::type, MapImpl>;
  };

  template <template<typename> class F>
  struct ListMap<F, Nil> {
    using type = Nil;
  };

  template <template<typename> class F, typename L>
  using map = typename ListMap<F, L>::type;

  ///
  template <template<typename> class F, typename X>
  struct Iterate {
    template <typename L>
    struct IterateImpl {
      using type = List<typename F<L>::type, Iterate<F, X>::IterateImpl>;
    };
    
    using type = List<X, IterateImpl>;
  };

  template <template<typename> class F, typename X>
  using iterate = typename Iterate<F, X>::type;


  /// Infinite list of values
  template <typename X>
  using gen = iterate<id, X>;


} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

