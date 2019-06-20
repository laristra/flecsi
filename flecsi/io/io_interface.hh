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
  using hdf5_region_t = typename IO_POLICY::hdf5_region_t;
  using launch_space_t = typename IO_POLICY::launch_space_t;
  using field_reference_t = typename IO_POLICY::field_reference_t;
  
  void add_regions(hdf5_t &hdf5_file, std::vector<hdf5_region_t> &hdf5_region_vector)
  {
    return IO_POLICY::add_regions(hdf5_file, hdf5_region_vector);
  }
  
  void add_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::add_default_index_topology(hdf5_file);
  }
  
  void generate_hdf5_files(hdf5_t &hdf5_file)
  {
    return IO_POLICY::generate_hdf5_files(hdf5_file);
  }
  
  void checkpoint_data(hdf5_t &hdf5_file, launch_space_t launch_space, std::vector<hdf5_region_t> &hdf5_region_vector, bool attach_flag)
  {
    return IO_POLICY::checkpoint_data(hdf5_file, launch_space, hdf5_region_vector, attach_flag);
  }
  
  void checkpoint_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::checkpoint_default_index_topology(hdf5_file);
  }

  void checkpoint_index_topology_field(hdf5_t &hdf5_file, field_reference_t &fh)
  {
    return IO_POLICY::checkpoint_index_topology_field(hdf5_file, fh);
  }
  
  void recover_data(hdf5_t &hdf5_file, launch_space_t launch_space, std::vector<hdf5_region_t> &hdf5_region_vector, bool attach_flag)
  {
    return IO_POLICY::recover_data(hdf5_file, launch_space, hdf5_region_vector, attach_flag);
  }
  
  void recover_default_index_topology(hdf5_t &hdf5_file)
  {
    return IO_POLICY::recover_default_index_topology(hdf5_file);
  }

  void recover_index_topology_field(hdf5_t &hdf5_file, field_reference_t &fh)
  {
    return IO_POLICY::recover_index_topology_field(hdf5_file, fh);
  }
}; // struct io_interface_u

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_IO_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/io_policy.hh>

namespace flecsi {
namespace io {

using io_interface_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>;
using hdf5_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::hdf5_t;
using hdf5_region_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::hdf5_region_t;
using launch_space_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::launch_space_t;
using field_reference_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>::field_reference_t;

} // namespace io
} // namespace flecsi
