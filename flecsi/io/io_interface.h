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

#include <string>

/*!  @file */

template<typename IO_POLICY>
struct io_interface : public IO_POLICY {

  void checkpoint_all_fields(const std::string & filename) {
    IO_POLICY::checkpoint_all_fields(filename);
  }

  void recover_fields(const std::string & filename) {
    IO_POLICY::recover_fields(filename);
  }

}; // struct io_interface

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_IO_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi/io/backend.h"

namespace flecsi {
namespace io {

using io_interface_t = io_interface<FLECSI_RUNTIME_IO_POLICY>;

} // namespace io
} // namespace flecsi
