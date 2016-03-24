#ifndef META_COUNTER_HPP
#define META_COUNTER_HPP

#include <cstddef>
#include <iostream>

#define MAX_COUNTER_SIZE 6

namespace flecsi {
namespace utils {

namespace container{
  template <class Tag>
  struct meta_counter {

    using size_type = std::size_t;

    template<size_type N>
    struct ident {
//pragmas to avoid unnecessary warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wunused-function"

        friend constexpr size_type adl_lookup (ident<N>);
        static constexpr size_type value = N;

    };   
#pragma GCC diagnostic pop
    // -----------------------------------------------------

    template<class Ident>
    struct writer {
         //we use a class template to create a definition for adl_flag (flag<N>)
         friend constexpr size_type adl_lookup (Ident){
             return Ident::value;
         }
      
      static constexpr size_type value = Ident::value; 
    };
    
    // -----------------------------------------------------

    //The different overloads of reader will always be invoked 
    //with two arguments, the first being int (0), and the 
    //second a specialization of template<int> struct ident


   //Without explicitly passing template-arguments to the reader, 
   //a specialization will only be viable if adl_flag (ident<N> {}) 
   //is usable where a constant-expression is required (  it can 
   // only be called if the appropriate overload of adl_flag has been define):

    template<size_type N, int = adl_lookup (ident<N> {})>
    static constexpr size_type value_reader (int, ident<N>) {
      return N;
    }

    //the searcher: the default-argument specified for R will be 
    //evaluated. This will effectively walk down our different
    // overloads until it finds a function that will stop the recursion 
    // — yielding the current count.
    template<size_type N>
    static constexpr size_type value_reader (float, ident<N>,
              size_type R = value_reader (0, ident<N-1> ())) {
      return R;
    }
    //base reader:
    // Since the searcher walks down the path by looking at the next
    // overload matching a value less than N, we must have a base-case 
    // so that we do not run into infinite recursion 
    // (in case reader (int, flag<N>) can never be instantiated).
    static constexpr size_type value_reader (float, ident<0>) {
      return 0;
    }

    // -----------------------------------------------------
    
  template<size_type Max = MAX_COUNTER_SIZE>
    static constexpr size_type value (size_type R = value_reader (0, ident<Max> {})) {
      return R;
    }

    template<size_type N = 1, class H = meta_counter>
    static constexpr size_type next (size_type R = writer<ident<N + H::value ()>>::value) {
        return R<MAX_COUNTER_SIZE ? R : (throw std::logic_error("Number of elements in the static container exceed the maximum container size. Please increase MAX_CONTAINER_SIZE variable"));
    }
 
  }; //end struct meta_counter

}// end namespace

} //end utils
} //end flecsi

#endif
