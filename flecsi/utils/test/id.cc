/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/id.h"

// includes: C++
#include <string>

// includes: other
#include <cinchtest.h>
#include "boost/core/demangle.hpp"



// =============================================================================
// Helper constructs
// =============================================================================

// prtype: print boost-demangled type
template<class T>
inline void prtype(void)
{
   CINCH_CAPTURE() << boost::core::demangle(typeid(T).name()) << std::endl;
}

// print
// Print a flecsi::utils::id_
template<std::size_t P, std::size_t E, std::size_t F, std::size_t G>
void print(const flecsi::utils::id_<P,E,F,G> &value)
{
   CINCH_CAPTURE() << "\n dimension == " << value.dimension();
   CINCH_CAPTURE() << "\n domain    == " << value.domain   ();
   CINCH_CAPTURE() << "\n partition == " << value.partition();
   CINCH_CAPTURE() << "\n entity    == " << value.entity   ();
   CINCH_CAPTURE() << "\n flags     == " << value.flags    ();
   CINCH_CAPTURE() << "\n global    == " << value.global   ();
   CINCH_CAPTURE() << std::endl;
}

// binary
// Write value as binary.
// Assumes value is of an unsigned integral type.
template<class T>
std::string binary(const T u)
{
   std::string s;
   for (std::size_t i = sizeof(T)*CHAR_BIT;  i-- ; )
      s += u >> i & T(1) ? '1' : '0';
   return s;
}

// numbers of bits for our tests
#define PBITS 20  /* for partition */
#define EBITS 40  /* for entity    */
#define FBITS 4   /* for flags     */
#define GBITS 60  /* for global    */



// =============================================================================
// Exercise all of id.h's constructs
// =============================================================================

TEST(id, all) {

   // type: id == id_<PBITS,EBITS,FBITS,GBITS>
   using id = flecsi::utils::id_<PBITS,EBITS,FBITS,GBITS>;


   // ------------------------
   // misc
   // ------------------------

   // local_id_t
   prtype<flecsi::utils::local_id_t>();
   CINCH_CAPTURE()
      << "sizeof(flecsi::utils::local_id_t) == "
      <<  sizeof(flecsi::utils::local_id_t) << std::endl;

   // id::FLAGS_UNMASK
   CINCH_CAPTURE() << "FLAGS_UNMASK == " << id::FLAGS_UNMASK << std::endl;
   CINCH_CAPTURE() << "FLAGS_UNMASK == " << binary(id::FLAGS_UNMASK) << '\n';
   CINCH_CAPTURE() << std::endl;

   /**/
   /**/
   /**/
   CINCH_CAPTURE() << binary(1u) << std::endl;
   CINCH_CAPTURE() << binary(2u) << std::endl;
   CINCH_CAPTURE() << binary(3u) << std::endl;
   CINCH_CAPTURE() << binary(4u) << std::endl;
   CINCH_CAPTURE() << binary(5u) << std::endl;
   CINCH_CAPTURE() << binary(6u) << std::endl;
   CINCH_CAPTURE() << binary(7u) << std::endl;
   CINCH_CAPTURE() << binary(8u) << std::endl;
   CINCH_CAPTURE() << std::endl;
   /**/
   /**/
   /**/


   // ------------------------
   // constructors
   // ------------------------

   {
      id a;
      const id b(id{});
      const id c = a;
      const id d(123);

      print(a);
      print(b);
      print(c);
      print(d);
   }


   // ------------------------
   // make
   // ------------------------

   {
      // make<DIMENSION,DOMAIN>
      // Arguments: (local_id [,partition_id [,flags [,global]]])
      const id a = id::make<1,2>(3);
      const id b = id::make<1,2>(3,4);
      const id c = id::make<1,2>(3,4,5);
      const id d = id::make<1,2>(3,4,5,6);
      print(a);
      print(b);
      print(c);
      print(d);

      // make<DOMAIN>
      // Arguments: (dimension, local_id [,partition_id [,flags [,global]]])
      const id e = id::make<2>(1,3);
      const id f = id::make<2>(1,3,4);
      const id g = id::make<2>(1,3,4,5);
      const id h = id::make<2>(1,3,4,5,6);
      print(e);
      print(f);
      print(g);
      print(h);
   }


   // ------------------------
   // local_id
   // global_id
   // ------------------------

   {
      const id a = id::make<1,2>(3,4,5,6);
      print(a);
      CINCH_CAPTURE() << std::endl;

      CINCH_CAPTURE() << "entity    : " << binary(a.entity   ()) << std::endl;
      CINCH_CAPTURE() << "partition : " << binary(a.partition()) << std::endl;
      CINCH_CAPTURE() << "domain    : " << binary(a.domain   ()) << std::endl;
      CINCH_CAPTURE() << "dimension : " << binary(a.dimension()) << std::endl;
      CINCH_CAPTURE() << "local_id(): " << binary(a.local_id ()) << std::endl;
      CINCH_CAPTURE() << std::endl;
      /*
        11000000000000000001001001
        11000000000000000001001001
      */

      CINCH_CAPTURE() << "local_id()  == " << a.local_id () << std::endl;
      CINCH_CAPTURE() << "global_id() == " << a.global_id() << std::endl;
      CINCH_CAPTURE() << std::endl;
   }


   // ------------------------
   // assignment
   // ------------------------

   {
      id a = id::make<1,2>(3,4,5,6);
      id b;
      a = b = id{};
      b = a = a;
   }


   // ------------------------
   // set_*
   // ------------------------

   {
      id a = id::make<1,2>(3,4,5,6);
      a.set_global(100);
      a.set_partition(200);
      a.set_flags(300);
   }

/*
   // assignment operators
   id_& operator=(id_ &&) = default;

   id_& operator=(const id_ &id)
   {
     // zzz should probably have: if (this != &id) { ... }
     dimension_ = id.dimension_;
     domain_ = id.domain_;
     partition_ = id.partition_;
     entity_ = id.entity_;
     global_ = id.global_;
     flags_ = id.flags_;
     return *this;
   }

   // zzz how are the first two const???

   void set_global   (size_t global   ) const
   { global_ = global; }

   void set_partition(size_t partition) const
   { partition_ = partition; }

   size_t set_flags  (size_t flags    )
   {
     assert(flags < 1 << FBITS && "flag bits exceeded");
     flags_ = flags;
     // zzz no return value???
     // zzz Also: earlier two set_* have void return
   }

   size_t global           () const { return global_;    }
   size_t dimension        () const { return dimension_; }
   size_t domain           () const { return domain_;    }
   size_t partition        () const { return partition_; }
   size_t entity           () const { return entity_;    }
   size_t index_space_index() const { return entity_;    }  // zzz why?
   size_t flags            () const { return flags_;     }

   // zzz must be for convenience/brevity
   bool operator<(const id_ & id) const{
     return local_id() < id.local_id();
   }

   bool operator==(const id_ & id) const{
     return (local_id() & FLAGS_UNMASK) == (id.local_id() & FLAGS_UNMASK);
   }

   // zzz odd != definition
   bool operator!=(const id_ & id) const{
     return !(local_id() == id.local_id());
   }
*/


   // ------------------------
   // ostream << local_id_t
   // ------------------------

   {
      flecsi::utils::local_id_t a = 1, b = 2;
      CINCH_CAPTURE() << a << '\n' << b << std::endl;
      //flecsi::utils::operator<<(std::cout,a);
      //flecsi::utils::operator<<(std::cout,b);
   }



   /*
   // ------------------------
   // Compare
   // ------------------------

   EXPECT_TRUE(CINCH_EQUAL_BLESSED("id.blessed"));
   */

} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
