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

struct legion_hdf5_logical_region_t
{
  legion_hdf5_logical_region_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name, std::map<FieldID, std::string> &field_string_map);

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
  bool generate_hdf5_file(int file_idx);
  
  std::string file_name;
  int num_files;
  std::vector<legion_hdf5_logical_region_t> logical_region_vector;
};

struct legion_checkpoint_internal_data_t {
  legion_checkpoint_internal_data_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name);
    
  LogicalRegion logical_region;
  LogicalPartition logical_partition;
  std::string logical_region_name;
  std::map<FieldID, std::string> field_string_map;
};

struct legion_io_policy_t {
  using hdf5_t = legion_hdf5_t;
  using checkpoint_internal_data_t = legion_checkpoint_internal_data_t;
  using launch_space_t = IndexSpace;
  
  legion_io_policy_t() {}
  
  ~legion_io_policy_t();
  
  void add_regions(legion_hdf5_t &hdf5_file, std::vector<legion_checkpoint_internal_data_t> &cp_test_data_vector);
  
  void add_default_index_topology(hdf5_t &hdf5_file, std::vector<std::string> &field_string_vector);
  
  void generate_hdf5_files(legion_hdf5_t &hdf5_file);
  
  void checkpoint_data(legion_hdf5_t &hdf5_file, IndexSpace launch_space, std::vector<legion_checkpoint_internal_data_t> &cp_test_data_vector, bool attach_flag);
  
  void checkpoint_index_topology_field(hdf5_t &hdf5_file, auto fh);
  
  void recover_data(legion_hdf5_t &hdf5_file, IndexSpace launch_space, std::vector<legion_checkpoint_internal_data_t> &cp_test_data_vector, bool attach_flag);
  
private:
  IndexSpace default_index_topology_file_is;
  IndexPartition default_index_topology_file_ip;
  LogicalPartition default_index_topology_file_lp;
  
}; // struct legion_io_policy_t

} // namespace io
} // namespace flecsi
