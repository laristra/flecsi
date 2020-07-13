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

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/flog.hh"
#include "flecsi/topo/unstructured/definition.hh"

namespace flecsi {
namespace topo {
namespace unstructured_impl {

using coloring = topo::unstructured_base::coloring;

template<size_t Dimension, typename Real = double>
coloring color(definition<Dimension, Real> const * definition,
  size_t colors) {

  flog(info) << "colors: " << colors << std::endl;
  flog(info) << "dimension: " << definition->dimension() << std::endl;

  return {};
} // color_mesh

} // namespace flecsi
} // namespace topo
} // namespace unstructured_impl
