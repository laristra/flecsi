/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>
#include <type_traits> // std::is_same

// user includes
#include "flecsi/utils/static_container/meta_list.h"
#include "flecsi/utils/static_container/static_for.h"

// using declarations
//using flecsi::utils;

using global_ID = flecsi::utils::container::meta_counter<class GlobalCount>;

using Legion_task_list = flecsi::utils::container::meta_list<class LegionList>;

template<std::size_t A>
class Task_ID{
 public:
 std::size_t get_id(){return A;};
};


//=============================================================================
//! \brief Test the tuple_zip function.
//=============================================================================
TEST(meta_container, simple) {

  
  using A = flecsi::utils::container::type_list<>;                 // empty list
  using B = A::push<void, int, short>::result; // type_list<void,  int, short>
  using C = B::         set<0, float>::result; // type_list<float, int, short>
  using D = C::                 at<1>::result; // int
  using E = C::                  init::result; // type_list<float, int>

  //checking meta_counter

  using C1 = flecsi::utils::container::meta_counter <class Counter1>;
  using C2 = flecsi::utils::container::meta_counter <class Counter2>;

  C1::next (); //1
  C1::next (); //2
  C1::next (); //3

  C2::next (); //1
  C2::next (); //2

  static_assert (C1::value () == 3, "C1::value () == 3");
  static_assert (C2::value () == 2, "C2::value () == 2");

 //checking meta_list
  using LX = flecsi::utils::container::meta_list<class Container>;

  LX::push<void, void, void, void> ();
  LX::set<0, class Hello> ();
  LX::set<2, class World> ();
  LX::pop ();

  LX::value<> x; // type_list<class Hello, void, class World>

  static_assert (
    std::is_same<
      flecsi::utils::container::type_list<class Hello, void, class World>, LX::value<>
    >::value, "try again"
  );

  //using meta_list in terms of Legion tasks

 //push a new element of the container
  Legion_task_list::push<class Task_ID<global_ID::next()> > ();
  Legion_task_list::push<class Task_ID<global_ID::next()> > ();
  Legion_task_list::push<class Task_ID<global_ID::next() > > ();
  Legion_task_list::push<class Task_ID<global_ID::next() > > ();
  //remove last element of the container
  Legion_task_list::pop();

  //getting size of the container
  std::cout << "Size of the container = " << Legion_task_list::value<>::size()<<std::endl;

  //explicitly printing 1rst and 2nd elements of the container by using "get" function
  using task_0 = Legion_task_list::get <0>::result;
  task_0 class_id0;
  std::cout << "Firs task ID from the Legion task's list = "<<class_id0.get_id()<<std::endl;

  using task_1 = Legion_task_list::get <1>::result;
  task_1 class_id1;
  std::cout << "Second task ID from the Legion task's list = "<<class_id1.get_id()<<std::endl;

   //static_for for the container
   //in this example we just print Indexes, but we can call "register task" instead.
  flecsi::utils::container::static_for<Legion_task_list::value<>::size(), Legion_task_list::print_all<>>();

}


/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

