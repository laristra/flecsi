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
#include <iostream>

namespace flecsi {

/*!
  Enumeration of task processor types. Not all of these may be supported
  by all runtimes. Unsupported processor information will be ignored.

  @ingroup execution
 */

enum processor_type_t : size_t { loc, toc, mpi }; // enum processor_type_t

/*!
  Convenience method to print processor_type_t instances.
 */

inline std::ostream &
operator<<(std::ostream & stream, const processor_type_t & variant) {
  switch(variant) {
    case processor_type_t::loc:
      stream << "loc";
      break;
    case processor_type_t::toc:
      stream << "toc";
      break;
    case processor_type_t::mpi:
      stream << "mpi";
      break;
  } // switch

  return stream;
} // operator <<

} // namespace flecsi
