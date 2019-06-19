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

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

using namespace Legion;

namespace flecsi {
namespace io {

struct legion_hdf5_region_t
{
  legion_hdf5_region_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name, std::map<FieldID, std::string> &field_string_map);
  legion_hdf5_region_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name);

  LogicalRegion logical_region;
  LogicalPartition logical_partition;
  std::string logical_region_name;
  std::map<FieldID, std::string> field_string_map;
  size_t dim_size[3];
};
  
struct legion_hdf5_t {
  legion_hdf5_t(const char* file_name, int num_files);
  legion_hdf5_t(std::string file_name, int num_files);
  void add_logical_region(LogicalRegion lr, LogicalPartition lp, std::string lr_name, std::map<FieldID, std::string> field_string_map);
  void add_hdf5_region(legion_hdf5_region_t hdf5_region);
  bool generate_hdf5_file(int file_idx);
  
  std::string file_name;
  int num_files;
  std::vector<legion_hdf5_region_t> hdf5_region_vector;
};

struct legion_io_policy_t {
  using hdf5_t = legion_hdf5_t;
  using hdf5_region_t = legion_hdf5_region_t;
  using launch_space_t = IndexSpace;
  
  legion_io_policy_t() {}
  
  ~legion_io_policy_t();
  
  void add_regions(legion_hdf5_t &hdf5_file, std::vector<legion_hdf5_region_t> &hdf5_region_vector);
  
  void add_default_index_topology(legion_hdf5_t &hdf5_file);
  
  void generate_hdf5_files(legion_hdf5_t &hdf5_file);
  
  void checkpoint_data(legion_hdf5_t &hdf5_file, IndexSpace launch_space, std::vector<legion_hdf5_region_t> &hdf5_region_vector, bool attach_flag);
  
  void checkpoint_default_index_topology(legion_hdf5_t &hdf5_file);
  
  void recover_data(legion_hdf5_t &hdf5_file, IndexSpace launch_space, std::vector<legion_hdf5_region_t> &hdf5_region_vector, bool attach_flag);
  
  void recover_default_index_topology(legion_hdf5_t &hdf5_file);
  
private:
  IndexSpace default_index_topology_file_is;
  IndexPartition default_index_topology_file_ip;
  LogicalPartition default_index_topology_file_lp;
  
}; // struct legion_io_policy_t

} // namespace io
} // namespace flecsi
