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

#include <flecsi/utils/ftest.hh>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.hh>

using namespace flecsi::execution;

flog_register_tag(global_object);

struct base_t {
  virtual int operator()() = 0;
};

struct derived_a_t : public base_t {
  derived_a_t(int value) : value_(value) {}

  ~derived_a_t() {
    std::cout << "desctructor derived_a_t" << std::endl;
  }

  int operator()() override {
    return value_;
  } // print

private:
  int value_;

}; // struct derived_a_t

struct derived_b_t : public base_t {
  derived_b_t(int value) : value_(value) {}

  ~derived_b_t() {
    std::cout << "desctructor derived_b_t" << std::endl;
  }

  int operator()() override {
    return 5 * value_;
  } // print

private:
  int value_;

}; // struct derived_b_t

/*

 */

int
global_object(int argc, char ** argv) {

  FTEST();

  flecsi_add_global_object(0, "objects", derived_a_t, 10);
  flecsi_add_global_object(1, "objects", derived_b_t, 20);

  auto go_a = flecsi_get_global_object(0, "objects", base_t);
  auto go_b = flecsi_get_global_object(1, "objects", base_t);

  ASSERT_EQ((*go_a)(), 10);
  ASSERT_EQ((*go_b)(), 100);

  return FTEST_RESULT();
}

ftest_register_driver(global_object);
