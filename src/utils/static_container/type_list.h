#ifndef  TYPE_LIST_HPP
#define TYPE_LIST_HPP

//this code is based on the static meta container exmaple from
//http://b.atch.se/posts/constexpr-meta-container

#include <cstddef>
#include <utility>

//each static meta container consist of the list of the types 
//(type_list), name of the container, and index sequence
//
//example:
//
//  int main () {
//    using A = atch::type_list<>;                 // empty list
//    using B = A::push<void, int, short>::result; // type_list<void,  int, short>
//    using C = B::         set<0, float>::result; // type_list<float, int, short>
//    using D = C::                 at<1>::result; // int
//    using E = C::                  init::result; // type_list<float, int>
//  }

namespace flecsi {
namespace utils {

namespace container{

   template<std::size_t N>
   using ic = std::integral_constant<std::size_t, N>;

   template<std::size_t... Ns>
   using iseq=std::index_sequence<Ns...>;

   template<std::size_t N>
   using make_iseq=std::make_index_sequence<N>;

   // -------------------------------------------------------------

   template<class... Ts>
   struct type_list {
      using size_type=std::size_t;

      // --------------------------------------------------------

      //getting size of the type_list 
      static constexpr size_type size() {
         return sizeof... (Ts);}

      // ---------------------------------------------------------

      //getting an "Indx" type from type_list
      template<size_type Indx>
      struct at {
         template<size_type N, class U, class... Us>
         struct access_helper
           : access_helper<N+1, Us ...>
         { };

         template<class U, class... Us>
         struct access_helper<Indx, U, Us...>{
           using result = U;
         };

       using result = typename access_helper<0,Ts...>::result;
      }; //end struct at

      // -----------------------------------------------------------

      template <class... Us>
      struct push {
         using result = type_list<Ts..., Us...>;
      };
  
      // ------------------------------------------------------------

      // set element of the type_list with index=Indx to type U
      template<size_type Indx, class U>
      struct set{

           template<size_type N>
           static auto at_helper (ic<N>)    -> typename at<N>::result;
           static auto at_helper (ic<Indx>) -> U;

           template<size_type... Ns>
           static auto set_helper (iseq<Ns...>)
               -> type_list<decltype (at_helper (ic<Ns> {} ))...>;

         using result = decltype (set_helper (make_iseq<size()> {} ));
      };//end struct set 
       
      // --------------------------------------------------------------

      struct init {
         template <size_type... Ns>
         static auto helper (iseq<Ns...>)
            ->type_list<typename at<Ns>::result...>;

         using result = decltype (helper (make_iseq<size () -1> {} ));
      };

   };
           
} //namespace container

} //namespace utils
} //namespace flecsi
#endif
