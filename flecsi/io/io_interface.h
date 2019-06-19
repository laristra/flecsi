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
  using checkpoint_internal_data_t = typename IO_POLICY::checkpoint_internal_data_t;
  using launch_space_t = typename IO_POLICY::launch_space_t;
  
  void add_regions(hdf5_t &hdf5_file, std::vector<checkpoint_internal_data_t> &cp_test_data_vector)
  {
    return IO_POLICY::add_regions(hdf5_file, cp_test_data_vector);
  }
  
  void add_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::add_default_index_topology(hdf5_file);
  }
  
  void generate_hdf5_files(hdf5_t &hdf5_file)
  {
    return IO_POLICY::generate_hdf5_files(hdf5_file);
  }
  
  void checkpoint_data(hdf5_t &hdf5_file, launch_space_t launch_space, std::vector<checkpoint_internal_data_t> &cp_test_data_vector, bool attach_flag)
  {
    return IO_POLICY::checkpoint_data(hdf5_file, launch_space, cp_test_data_vector, attach_flag);
  }
  
  void checkpoint_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::checkpoint_default_index_topology(hdf5_file);
  }
  
  void recover_data(hdf5_t &hdf5_file, launch_space_t launch_space, std::vector<checkpoint_internal_data_t> &cp_test_data_vector, bool attach_flag)
  {
    return IO_POLICY::recover_data(hdf5_file, launch_space, cp_test_data_vector, attach_flag);
  }
  
  void recover_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::recover_default_index_topology(hdf5_file);
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
using checkpoint_internal_data_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::checkpoint_internal_data_t;
using launch_space_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::launch_space_t;

} // namespace io
} // namespace flecsi
