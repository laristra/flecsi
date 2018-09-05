/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#pragma once

#include <bitset>
#include <functional>
#include <iostream>

namespace flecsi {
namespace tutorial {

struct node_t {

  using bitset_t = std::bitset<8>;
  using action_t = std::function<int(int, char **)>;

  node_t(action_t const & action = {}, bitset_t const & bitset = {})
    : action_(action), bitset_(bitset) {}

  bool initialize(node_t const & node) {
    action_ = node.action_;
    bitset_ = node.bitset_;
    return true;
  } // initialize

  action_t const & action() const { return action_; }
  action_t & action() { return action_; }

  bitset_t const & bitset() const { return bitset_; }
  bitset_t & bitset() { return bitset_; }

private:

  action_t action_;
  bitset_t bitset_;

}; // struct node_t

std::ostream &
operator << (std::ostream & stream, node_t const & node) {
  stream << "bitset: " << node.bitset() << std::endl;
  stream << "action: " << &node.action() << std::endl;
  return stream;
} // operator <<

} // namespace tutorial
} // namespace flecsi
