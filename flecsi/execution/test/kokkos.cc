/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */


#define __FLECSI_PRIVATE__
#include <flecsi/data/data.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/execution/kernel_interface.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/target.hh>

//#include <Kokkos_Core.hpp>

flog_register_tag(kokkos);


using namespace flecsi;
using namespace flecsi::execution;
using namespace flecsi::data;
using namespace flecsi::topology;

using index_field_t = index_field_member<double>;
const index_field_t pressure_field;

const auto pressure = pressure_field(flecsi_index_topology);

void
assign(index_field_t::accessor<wo> p) {
  flog(info) << "assign on " << color() << std::endl;
  p = color();
} // assign

int
check(index_field_t::accessor<ro> p) {

  FTEST();

  size_t c=color();
 
  struct iterator{
    FLECSI_INLINE_TARGET 
    size_t operator[](size_t i){ return i;};
    FLECSI_INLINE_TARGET
    size_t size(){return 5;}
    size_t array[5];
  };

  iterator it;

  parallel_for(it, 
    KOKKOS_LAMBDA(auto c) {
      assert(p == c);
  },
  std::string("test"));

//  forall(it, "test2"){
//    assert(p == i);
//  };

  return FTEST_RESULT();
} // print


/*

 */

int
kokkos_sanity(int argc, char ** argv) {

  FTEST();

  // Kokkos::initialize(argc, argv);
  Kokkos::print_configuration(std::cerr);
  // Kokkos::finalize();

  execute<assign>(pressure);
  execute<check>(pressure);

  return FTEST_RESULT();
}

ftest_register_driver(kokkos_sanity);
