#ifndef META_LIST_HPP
#define META_LIST_HPP

#include <cstddef>

#include "type_list.h"
#include "meta_counter.h"

namespace flecsi {
namespace utils {

namespace container{ 
 namespace {

 template<class Tag>
  struct meta_list {
    //Every instantiation of meta_list has its own unique counter
    using   counter = container::meta_counter<meta_list<Tag>>;
    using size_type = typename counter::size_type;

    //the below dtruct allow us to assosiate a particular state (value) 
    //with a particuler ident (identity) by declaring afunction with 
    //deduce return-type (adl_lookup)
    template<size_type N, class = void>
    struct ident {
//following pragmas are used to avoid unnecessary warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wunused-function"
      friend auto adl_lookup (ident<N>);
#pragma GCC diagnostic pop
      static constexpr size_type value = N;
    };

    //cleate an empty type_list for ident<0> (starter state)
    template<class Dummy>
    struct ident<0, Dummy> {
      friend auto adl_lookup (ident<0>) {
        return container::type_list<> {};
      }
    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    //we declare struct "writer" in purpose to conditionally provide a definition for the
    // "adl_lookup" assosiated with particular state
    template<class Ident, class Value>
    struct writer {
      friend auto adl_lookup (ident<Ident::value>) {
        return Value {};
      }

      static constexpr size_type value = Ident::value;
    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    //helper function
    //needed to increment the associated "counter", construct an "ident<N>" (using the 
    // return value), and instantiate writer<Ident, State>
    template<
      class State,
      class     H = meta_list,
      class Ident = typename H::template ident<H::counter::next ()>
    >
    static constexpr size_type push_state (size_type R = writer<Ident, State>::value) {
      return R;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // value_ident, will allow us to get the current ident. 
    // It simply reads the current value of list's counter and
    //  alias the corresponding ident<N>.  
    template<
      class     H = meta_list,
      size_type N = H::counter::value ()>
    using value_ident = typename H::template ident<N>;

    //value uses the previously declared alias, value_ident, and
    // look up the deduced return-type of the associated
    //  adl_lookup using decltype;
    //  meaning that it yields the most recent state of the list.
    template<
      class     H = meta_list,
      class Ident = typename H::template value_ident<>
    >
    using value = decltype (adl_lookup (Ident {}));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    template<class... Ts, class H = meta_list>
    static constexpr void push (
      size_type = push_state<
        typename H::template value<>::template push<Ts...>::result
      > ()
    ) {}

    template<class H = meta_list>
    static constexpr void pop (
      size_type = push_state<
        typename H::template value<>::init::result
      > ()
    ) {}

    template<size_type Idx, class T, class H = meta_list>
    static constexpr void set (
      size_type = push_state<
        typename H::template value<>::template set<Idx, T>::result
      > ()
    ) {}

    template<size_type Idx,  class H = meta_list>
    struct get {
       using result = typename H::template value<>::template at<Idx>::result;
     };

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   template<class H = meta_list>
   struct print_all
   {
    template<size_type Idx> static inline void func()
    {
        using result = typename H::template value<>::template at<Idx>::result;
        std::cout << Idx << std::endl;
    }
   };
 
  };
 
 }//end namespace
} //end namespace container

}//namespace utils
}//namespace flecsi
#endif
