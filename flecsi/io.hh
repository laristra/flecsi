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

#include "io/backend.hh"

namespace flecsi::io {

#ifdef DOXYGEN // these are implemented per-backend
typedef unspecified hdf5_t, hdf5_region_t, launch_space_t;
#endif

// TODO: make these member functions
hdf5_t init_hdf5_file(const char *, int num_files);
bool create_hdf5_file(hdf5_t &, int file_idx);
bool open_hdf5_file(hdf5_t &, int file_idx);
bool close_hdf5_file(hdf5_t &);
bool create_datasets_for_regions(hdf5_t & hdf5_file, int file_idx);

bool write_string_to_hdf5_file(hdf5_t &,
  int file_idx,
  const char * group_name,
  const char * dataset_name,
  const std::string &,
  size_t);
bool read_string_from_hdf5_file(hdf5_t &,
  int file_idx,
  const char * group_name,
  const char * dataset_name,
  std::string &);

void add_regions(hdf5_t &, std::vector<hdf5_region_t> &);
void generate_hdf5_files(hdf5_t &);

void checkpoint_data(hdf5_t &,
  launch_space_t,
  std::vector<hdf5_region_t> &,
  bool attach);
void recover_data(hdf5_t &,
  launch_space_t,
  std::vector<hdf5_region_t> &,
  bool attach);

#ifdef DOXYGEN
struct io_interface_t {
  void add_process_topology(hdf5_t &);
  void checkpoint_process_topology(hdf5_t &);
  void checkpoint_index_topology_field(hdf5_t &, const field_reference_t &);
  void recover_process_topology(hdf5_t &);
  void recover_index_topology_field(hdf5_t &, const field_reference_t &);
};
#endif

} // namespace flecsi::io
