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

/*!  @file */

template<typename IO_POLICY>
struct io_interface_u : public IO_POLICY{
  using hdf5_t = typename IO_POLICY::hdf5_t;
  using cp_test_data_t = typename IO_POLICY::cp_test_data_t;
  
  void add_regions(hdf5_t &hdf5_file, std::vector<cp_test_data_t> &cp_test_data_vector)
  {
    return IO_POLICY::add_regions(hdf5_file, cp_test_data_vector);
  }
  
  void generate_hdf5_files(hdf5_t &hdf5_file)
  {
    return IO_POLICY::generate_hdf5_files(hdf5_file);
  }
  
  void checkpoint_data(hdf5_t &hdf5_file, std::vector<cp_test_data_t> &cp_test_data_vector, bool attach_flag)
  {
    return IO_POLICY::checkpoint_data(hdf5_file, cp_test_data_vector, attach_flag);
  }
  
  void recover_data(hdf5_t &hdf5_file, std::vector<cp_test_data_t> &cp_test_data_vector, bool attach_flag)
  {
    return IO_POLICY::recover_data(hdf5_file, cp_test_data_vector, attach_flag);
  }
}; // struct io_interface_u

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_IO_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/io_policy.h>

namespace flecsi {
namespace io {

using io_interface_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>;
using hdf5_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::hdf5_t;
using cp_test_data_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::cp_test_data_t;

} // namespace io
} // namespace flecsi
