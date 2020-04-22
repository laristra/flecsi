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
#pragma once

#include "cycle_control.hh"
#include "package_a.hh"
#include "package_b.hh"

#include "flecsi/flog.hh"

namespace package_c {

inline int
advance() {
  flog(info) << "C advance" << std::endl;
  return 0;
} // advance

inline control::action<advance, cp::advance> advance_action;

inline const auto dep_bc = package_b::advance_action.add(advance_action);
inline const auto dep_ca = advance_action.add(package_a::advance_action);

inline int
analyze() {
  flog(info) << "C analyze" << std::endl;
  return 0;
} // analyze

inline control::action<analyze, cp::analyze> analyze_action;
inline const auto dep_a = analyze_action.add(package_a::analyze_action);

} // namespace package_c
