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

/*! @file */

#include <bitset>

namespace flecsi {
namespace control {

struct default_node_t {

  using bitset_t = std::bitset<8>;
  using action_t = std::function<void(int, char **)>;

  default_node_t(action_t const & action = {}, bitset_t const & bitset = {})
    : action_(action), bitset_(bitset) {}

  bool initialize(default_node_t const & node) {
    action_ = node.action_;
    bitset_ = node.bitset_;
    return true;
  } // initialize

  action_t const & action() const {
    return action_;
  }
  action_t & action() {
    return action_;
  }

  bitset_t const & bitset() const {
    return bitset_;
  }
  bitset_t & bitset() {
    return bitset_;
  }

private:
  action_t action_ = {};
  bitset_t bitset_ = 0;

}; // struct default_node_t

inline std::ostream &
operator<<(std::ostream & stream, default_node_t const & node) {
  stream << "bitset: " << node.bitset() << std::endl;
  stream << "action: " << &node.action() << std::endl;
  return stream;
} // operator <<

} // namespace control
} // namespace flecsi
