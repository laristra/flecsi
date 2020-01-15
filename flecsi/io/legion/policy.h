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

#include <sstream>

#include <hdf5.h>
#include <legion.h>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <cinchlog.h>

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/internal_task.h"
#include "flecsi/utils/serialize.h"

clog_register_tag(io);

namespace flecsi {
namespace io {

inline void checkpoint_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);

inline void checkpoint_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);

inline void recover_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);

inline void recover_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);

/*----------------------------------------------------------------------------*
  HDF5 descriptor of one logical region, not called by users.
 *----------------------------------------------------------------------------*/
struct legion_hdf5_region_t {
  legion_hdf5_region_t(Legion::LogicalRegion lr,
    Legion::LogicalPartition lp,
    std::string lr_name,
    std::map<Legion::FieldID, std::string> & field_string_map)
    : logical_region(lr), logical_partition(lp), logical_region_name(lr_name),
      field_string_map(field_string_map) {
    Legion::Runtime * runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();
    if(lr.get_dim() == 1) {
      Legion::Domain domain =
        runtime->get_index_space_domain(ctx, lr.get_index_space());
      dim_size[0] = domain.get_volume();
      {
        clog_tag_guard(io);
        clog(info) << "ID logical region size " << dim_size[0] << std::endl;
      }
    }
    else {
      Legion::Domain domain =
        runtime->get_index_space_domain(ctx, lr.get_index_space());
      dim_size[0] = domain.get_volume();
      {
        clog_tag_guard(io);
        clog(info) << "ID logical region size " << dim_size[0] << std::endl;
      }
    }
  }

  legion_hdf5_region_t(Legion::LogicalRegion lr,
    Legion::LogicalPartition lp,
    std::string lr_name)
    : logical_region(lr), logical_partition(lp), logical_region_name(lr_name) {}

  Legion::LogicalRegion logical_region;
  Legion::LogicalPartition logical_partition;
  std::string logical_region_name;
  std::map<Legion::FieldID, std::string> field_string_map;
  size_t dim_size[3];
};

/*----------------------------------------------------------------------------*
  HDF5 file, not called by users.
 *----------------------------------------------------------------------------*/
struct legion_hdf5_t {

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::legion_hdf5_t.
  //----------------------------------------------------------------------------//
  legion_hdf5_t(std::string file_name, int num_files)
    : hdf5_file_id(-1), file_name(file_name), num_files(num_files) {
    hdf5_region_vector.clear();
    hdf5_group_map.clear();
    {
      clog_tag_guard(io);
      clog(info) << "Init HDF5 file " << file_name << " num_files " << num_files
                 << std::endl;
    }
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::legion_hdf5_t.
  //----------------------------------------------------------------------------//
  legion_hdf5_t(const char * file_name, int num_files)
    : legion_hdf5_t(std::string(file_name), num_files) {}

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::create_hdf5_file.
  //----------------------------------------------------------------------------//
  bool create_hdf5_file(int file_idx) {
    assert(hdf5_file_id == -1);
    std::string fname = file_name + std::to_string(file_idx);
    hdf5_file_id =
      H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if(hdf5_file_id < 0) {
      std::ostringstream os;
      os << "H5Fcreate failed: " << hdf5_file_id;
      clog_error(os.str());
      return false;
    }
    {
      clog_tag_guard(io);
      clog(info) << "Create HDF5 file " << fname << " file_id " << hdf5_file_id
                 << std::endl;
    }
    return true;
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t:open_hdf5_file.
  //----------------------------------------------------------------------------//
  bool open_hdf5_file(int file_idx) {
    assert(hdf5_file_id == -1);
    std::string fname = file_name + std::to_string(file_idx);
    hdf5_file_id = H5Fopen(fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    if(hdf5_file_id < 0) {
      std::ostringstream os;
      os << "H5Fopen failed: " << hdf5_file_id;
      clog_error(os.str());
      return false;
    }
    {
      clog_tag_guard(io);
      clog(info) << "Open HDF5 file " << fname << " file_id " << hdf5_file_id
                 << std::endl;
    }
    return true;
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t:close_hdf5_file.
  //----------------------------------------------------------------------------//
  bool close_hdf5_file() {
    assert(hdf5_file_id >= 0);
    H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);
    H5Fclose(hdf5_file_id);
    {
      clog_tag_guard(io);
      clog(info) << "Close HDF5 file_id " << hdf5_file_id << std::endl;
    }
    hdf5_file_id = -1;
    return true;
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::write_string_to_hdf5_file.
  //----------------------------------------------------------------------------//
  bool write_string_to_hdf5_file(const char * group_name,
    const char * dataset_name,
    const std::string & str,
    size_t size) {

    assert(hdf5_file_id >= 0);

    const char * cstr = str.c_str();
    herr_t status;
    // TODO:FIXME
    // status = H5Eset_auto(NULL, NULL);
    // status = H5Gget_objinfo (hdf5_file_id, group_name, 0, NULL);

    hid_t group_id;
    std::map<std::string, hid_t>::iterator it;
    it = hdf5_group_map.find(std::string(group_name));
    if(it != hdf5_group_map.end()) {
      group_id = H5Gopen2(hdf5_file_id, group_name, H5P_DEFAULT);
    }
    else {
      group_id = H5Gcreate2(
        hdf5_file_id, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      hdf5_group_map[std::string(group_name)] = group_id;
    }
    if(group_id < 0) {
      std::ostringstream os;
      os << "H5Gcreate2 failed: " << group_id;
      clog_error(os.str());
      H5Fclose(hdf5_file_id);
      return false;
    }

    hid_t filetype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(filetype, H5T_VARIABLE);
    hid_t memtype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(memtype, H5T_VARIABLE);

    hsize_t dims[1];
    dims[0] = 1;
    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

    const char * data[1];
    data[0] = cstr;
    hid_t dset = H5Dcreate2(group_id, dataset_name, filetype, dataspace_id,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite(dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);
    status = H5Dclose(dset);
    status = H5Sclose(dataspace_id);
    status = H5Tclose(filetype);
    status = H5Tclose(memtype);
    status = H5Gclose(group_id);
    return true;
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::read_string_from_hdf5_file.
  //----------------------------------------------------------------------------//
  bool read_string_from_hdf5_file(const char * group_name,
    const char * dataset_name,
    std::string & str) {

    assert(hdf5_file_id >= 0);

    herr_t status;
    // TODO:FIXME
    // status = H5Eset_auto(NULL, NULL);
    // status = H5Gget_objinfo (hdf5_file_id, group_name, 0, NULL);

    hid_t group_id;
    group_id = H5Gopen2(hdf5_file_id, group_name, H5P_DEFAULT);

    if(group_id < 0) {
      std::ostringstream os;
      os << "H5Gcreate2 failed: " << group_id;
      clog_error(os.str());
      H5Fclose(hdf5_file_id);
      return false;
    }

    hid_t dset = H5Dopen2(group_id, dataset_name, H5P_DEFAULT);

    hid_t filetype = H5Dget_type(dset);
    hid_t memtype = H5Tcopy(H5T_C_S1);
    status = H5Tset_size(memtype, H5T_VARIABLE);

    char * data[1];
    status = H5Dread(dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    str = str + std::string(data[0]);
    H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);

    hid_t space = H5Dget_space(dset);
    status = H5Dvlen_reclaim(memtype, space, H5P_DEFAULT, data);
    status = H5Dclose(dset);
    status = H5Tclose(filetype);
    status = H5Tclose(memtype);
    status = H5Gclose(group_id);
    return true;
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::add_logical_region.
  //----------------------------------------------------------------------------//
  void add_logical_region(Legion::LogicalRegion lr,
    Legion::LogicalPartition lp,
    std::string lr_name,
    std::map<Legion::FieldID, std::string> field_string_map) {
    legion_hdf5_region_t h5_lr(lr, lp, lr_name, field_string_map);
    hdf5_region_vector.push_back(h5_lr);
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::add_hdf5_region.
  //----------------------------------------------------------------------------//
  void add_hdf5_region(const legion_hdf5_region_t & hdf5_region) {
    hdf5_region_vector.push_back(hdf5_region);
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_hdf5_t::create_datasets_for_regions.
  //----------------------------------------------------------------------------//
  bool create_datasets_for_regions(int file_idx) {
    assert(hdf5_region_vector.size() > 0);
    assert(hdf5_file_id >= 0);

    Legion::Runtime * runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();

    {
      clog_tag_guard(io);
      clog(info) << "Create HDF5 datasets file_id " << hdf5_file_id
                 << " regions size " << hdf5_region_vector.size() << std::endl;
    }

    for(legion_hdf5_region_t & lr_it : hdf5_region_vector) {

      hid_t dataspace_id = -1;
      if(lr_it.logical_region.get_index_space().get_dim() == 1) {
        Legion::LogicalRegion sub_lr = runtime->get_logical_subregion_by_color(
          ctx, lr_it.logical_partition, file_idx);
        Legion::Domain domain =
          runtime->get_index_space_domain(ctx, sub_lr.get_index_space());
        hsize_t dims[1];
        dims[0] = domain.get_volume();
        dataspace_id = H5Screate_simple(1, dims, NULL);
      }
      else {
        Legion::LogicalRegion sub_lr = runtime->get_logical_subregion_by_color(
          ctx, lr_it.logical_partition, file_idx);
        Legion::Domain domain =
          runtime->get_index_space_domain(ctx, sub_lr.get_index_space());
        hsize_t dims[1];
        dims[0] = domain.get_volume();
        dataspace_id = H5Screate_simple(1, dims, NULL);
      }
      if(dataspace_id < 0) {
        std::ostringstream os;
        os << "H5Screate_simple failed: " << dataspace_id;
        clog_error(os.str());
        H5Fclose(hdf5_file_id);
        return false;
      }
#if 0
      hid_t group_id = H5Gcreate2(file_id, (*lr_it).logical_region_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      if (group_id < 0) {
        printf("H5Gcreate2 failed: %lld\n", (long long)group_id);
        H5Sclose(dataspace_id);
        H5Fclose(hdf5_file_id);
        return false;
      }
#endif
      for(std::pair<const Legion::FieldID, std::string> & it :
        lr_it.field_string_map) {
        const char * dataset_name = (it.second).c_str();
        hid_t dataset = H5Dcreate2(hdf5_file_id, dataset_name, H5T_IEEE_F64LE,
          dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if(dataset < 0) {
          std::ostringstream os;
          os << "H5Dcreate2 failed: " << dataset;
          clog_error(os.str());
          //    H5Gclose(group_id);
          H5Sclose(dataspace_id);
          H5Fclose(hdf5_file_id);
          return false;
        }
        H5Dclose(dataset);
      }
      //   H5Gclose(group_id);
      H5Sclose(dataspace_id);
    }
    H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);
    return true;
  }

  hid_t hdf5_file_id;
  std::string file_name;
  int num_files;
  std::vector<legion_hdf5_region_t> hdf5_region_vector;
  std::map<std::string, hid_t> hdf5_group_map;
};

/*----------------------------------------------------------------------------*
  Legion HDF5 checkpoint interface.
 *----------------------------------------------------------------------------*/
struct legion_policy_t {
  using hdf5_t = legion_hdf5_t;
  using hdf5_region_t = legion_hdf5_region_t;
  using launch_space_t = Legion::IndexSpace;

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::~legion_io_policy_t.
  //----------------------------------------------------------------------------//
  ~legion_policy_t() {
    Legion::Runtime * runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();
    for(std::pair<const size_t, Legion::IndexSpace> & it : file_is_map) {
      if(it.second != Legion::IndexSpace::NO_SPACE) {
        // printf("clean up default_index_topology_file_is hash %ld\n",
        // it->first);
        runtime->destroy_index_space(ctx, it.second);
      }
    }
    file_is_map.clear();
    file_ip_map.clear();
    file_lp_map.clear();
  }

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::init_hdf5_file.
  //----------------------------------------------------------------------------//
  legion_hdf5_t init_hdf5_file(const char * file_name, int num_files) {
    return legion_hdf5_t(file_name, num_files);
  } // init_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::create_hdf5_file.
  //----------------------------------------------------------------------------//
  bool create_hdf5_file(legion_hdf5_t & hdf5_file, int file_idx) {
    return hdf5_file.create_hdf5_file(file_idx);
  } // create_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::open_hdf5_file.
  //----------------------------------------------------------------------------//
  bool open_hdf5_file(legion_hdf5_t & hdf5_file, int file_idx) {
    return hdf5_file.open_hdf5_file(file_idx);
  } // open_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::close_hdf5_file.
  //----------------------------------------------------------------------------//
  bool close_hdf5_file(legion_hdf5_t & hdf5_file) {
    return hdf5_file.close_hdf5_file();
  } // close_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::create_datasets_for_regions.
  //----------------------------------------------------------------------------//
  bool create_datasets_for_regions(legion_hdf5_t & hdf5_file, int file_idx) {
    return hdf5_file.create_datasets_for_regions(file_idx);
  } // create_datasets_for_regions

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::write_string_to_hdf5_file.
  //----------------------------------------------------------------------------//
  bool write_string_to_hdf5_file(legion_hdf5_t & hdf5_file,
    int rank_id,
    const char * group_name,
    const char * dataset_name,
    const std::string & str,
    size_t size) {
    return hdf5_file.write_string_to_hdf5_file(
      group_name, dataset_name, str, size);
  } // write_string_to_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::read_string_from_hdf5_file.
  //----------------------------------------------------------------------------//
  bool read_string_from_hdf5_file(legion_hdf5_t & hdf5_file,
    int file_idx,
    const char * group_name,
    const char * dataset_name,
    std::string & str) {
    return hdf5_file.read_string_from_hdf5_file(group_name, dataset_name, str);
  } // read_string_from_hdf5_file

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::add_regions.
  //----------------------------------------------------------------------------//
  void add_regions(legion_hdf5_t & hdf5_file,
    std::vector<legion_hdf5_region_t> & hdf5_region_vector) {
    for(auto & r : hdf5_region_vector)
      hdf5_file.add_hdf5_region(r);
  } // add_regions

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::generate_hdf5_files.
  //----------------------------------------------------------------------------//
  void generate_hdf5_files(legion_hdf5_t & hdf5_file) {
    for(int i = 0; i < hdf5_file.num_files; i++) {
      hdf5_file.create_hdf5_file(i);
      hdf5_file.create_datasets_for_regions(i);
      hdf5_file.close_hdf5_file();
    }
  } // generate_hdf5_files

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::checkpoint_data.
  //----------------------------------------------------------------------------//
  void checkpoint_data(legion_hdf5_t & hdf5_file,
    Legion::IndexSpace launch_space,
    std::vector<legion_hdf5_region_t> & hdf5_region_vector,
    bool attach_flag) {
    std::string file_name = hdf5_file.file_name;

    Legion::Runtime * runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();

    std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;
    for(legion_hdf5_region_t & it : hdf5_region_vector) {
      field_string_map_vector.push_back(it.field_string_map);
    }

    std::vector<std::byte> task_args;
    task_args = utils::serial_put(std::tie(field_string_map_vector, file_name));

    execution::context_t & context_ = execution::context_t::instance();

    auto task_id =
      attach_flag
        ? context_
            .task_id<flecsi_internal_task_key(checkpoint_with_attach_task)>()
        : context_.task_id<flecsi_internal_task_key(
            checkpoint_without_attach_task)>();

    Legion::IndexLauncher checkpoint_launcher(task_id, launch_space,
      Legion::TaskArgument((void *)(task_args.data()), task_args.size()),
      Legion::ArgumentMap());

    int idx = 0;
    for(legion_hdf5_region_t & it : hdf5_region_vector) {
      checkpoint_launcher.add_region_requirement(
        Legion::RegionRequirement(it.logical_partition, 0 /*projection ID*/,
          READ_ONLY, EXCLUSIVE, it.logical_region));

      std::map<Legion::FieldID, std::string> & field_string_map =
        it.field_string_map;
      for(std::pair<const Legion::FieldID, std::string> & it :
        field_string_map) {
        checkpoint_launcher.region_requirements[idx].add_field(it.first);
      }
      idx++;
    }

    {
      clog_tag_guard(io);
      clog(info) << "Start checkpoint file " << file_name << " regions size "
                 << hdf5_region_vector.size() << std::endl;
    }

    Legion::FutureMap fumap =
      runtime->execute_index_space(ctx, checkpoint_launcher);
    fumap.wait_all_results();
  } // checkpoint_data

  //----------------------------------------------------------------------------//
  // Implementation of legion_io_policy_t::recover_data.
  //----------------------------------------------------------------------------//
  void recover_data(legion_hdf5_t & hdf5_file,
    Legion::IndexSpace launch_space,
    std::vector<legion_hdf5_region_t> & hdf5_region_vector,
    bool attach_flag) {
    std::string file_name = hdf5_file.file_name;

    Legion::Runtime * runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();

    std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;
    for(legion_hdf5_region_t & it : hdf5_region_vector) {
      field_string_map_vector.push_back(it.field_string_map);
    }

    std::vector<std::byte> task_args;
    task_args = utils::serial_put(std::tie(field_string_map_vector, file_name));

    execution::context_t & context_ = execution::context_t::instance();

    auto task_id =
      attach_flag
        ? context_.task_id<flecsi_internal_task_key(recover_with_attach_task)>()
        : context_
            .task_id<flecsi_internal_task_key(recover_without_attach_task)>();

    Legion::IndexLauncher recover_launcher(task_id, launch_space,
      Legion::TaskArgument((void *)(task_args.data()), task_args.size()),
      Legion::ArgumentMap());
    int idx = 0;
    for(legion_hdf5_region_t & it : hdf5_region_vector) {
      recover_launcher.add_region_requirement(
        Legion::RegionRequirement(it.logical_partition, 0 /*projection ID*/,
          WRITE_DISCARD, EXCLUSIVE, it.logical_region));

      std::map<Legion::FieldID, std::string> & field_string_map =
        it.field_string_map;
      for(std::pair<const Legion::FieldID, std::string> & it :
        field_string_map) {
        recover_launcher.region_requirements[idx].add_field(it.first);
      }
      idx++;
    }

    {
      clog_tag_guard(io);
      clog(info) << "Start recover file " << file_name << " regions size "
                 << hdf5_region_vector.size() << std::endl;
    }

    Legion::FutureMap fumap =
      runtime->execute_index_space(ctx, recover_launcher);
    fumap.wait_all_results();
  } // recover_data

private:
  std::map<size_t, Legion::IndexSpace> file_is_map;
  std::map<size_t, Legion::IndexPartition> file_ip_map;
  std::map<size_t, Legion::LogicalPartition> file_lp_map;

}; // struct legion_policy_t

inline void
checkpoint_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {

  const int point = task->index_point.point_data[0];

  const std::byte * task_args = (const std::byte *)task->args;

  std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;

  field_string_map_vector =
    utils::serial_get<std::vector<std::map<Legion::FieldID, std::string>>>(
      task_args);

  std::string fname = utils::serial_get<std::string>(task_args);

  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    Legion::PhysicalRegion attach_dst_pr;
    Legion::LogicalRegion input_lr = regions[rid].get_logical_region();
    Legion::LogicalRegion attach_dst_lr = runtime->create_logical_region(
      ctx, input_lr.get_index_space(), input_lr.get_field_space());

    Legion::AttachLauncher hdf5_attach_launcher(
      EXTERNAL_HDF5_FILE, attach_dst_lr, attach_dst_lr);
    std::map<Legion::FieldID, const char *> field_map;
    std::set<Legion::FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<Legion::FieldID, std::string>::iterator map_it;
    for(Legion::FieldID it : field_set) {
      map_it = field_string_map_vector[rid].find(it);
      if(map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(it, (map_it->second).c_str()));
      }
      else {
        assert(0);
      }
    }

    {
      clog_tag_guard(io);
      clog(info) << "Checkpoint data to HDF5 file attach " << file_name
                 << " region_id " << rid
                 << " (dataset(fid) size= " << field_map.size() << ")"
                 << " field_string_map_vector(regions) size "
                 << field_string_map_vector.size() << std::endl;
    }

    hdf5_attach_launcher.attach_hdf5(
      file_name, field_map, LEGION_FILE_READ_WRITE);
    attach_dst_pr =
      runtime->attach_external_resource(ctx, hdf5_attach_launcher);
    // cp_pr.wait_until_valid();

    Legion::CopyLauncher copy_launcher1;
    copy_launcher1.add_copy_requirements(
      Legion::RegionRequirement(input_lr, READ_ONLY, EXCLUSIVE, input_lr),
      Legion::RegionRequirement(
        attach_dst_lr, WRITE_DISCARD, EXCLUSIVE, attach_dst_lr));
    for(Legion::FieldID it : field_set) {
      copy_launcher1.add_src_field(0, it);
      copy_launcher1.add_dst_field(0, it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher1);

    Legion::Future fu =
      runtime->detach_external_resource(ctx, attach_dst_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, attach_dst_lr);
  }
} // checkpoint_with_attach_task

inline void
checkpoint_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {

  const int point = task->index_point.point_data[0];

  const std::byte * task_args = (const std::byte *)task->args;

  std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;

  field_string_map_vector =
    utils::serial_get<std::vector<std::map<Legion::FieldID, std::string>>>(
      task_args);

  std::string fname = utils::serial_get<std::string>(task_args);

  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file_id < 0) {
    std::ostringstream os;
    os << "H5Fopen failed: " << file_id;
    clog_error(os.str());
    assert(0);
  }

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    std::set<Legion::FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<Legion::FieldID, std::string>::iterator map_it;
    for(Legion::FieldID it : field_set) {
      map_it = field_string_map_vector[rid].find(it);
      if(map_it != field_string_map_vector[rid].end()) {
        const Legion::FieldAccessor<READ_ONLY, double, 1, Legion::coord_t,
          Realm::AffineAccessor<double, 1, Legion::coord_t>>
          acc_fid(regions[rid], it);
        Legion::Rect<1> rect = runtime->get_index_space_domain(
          ctx, task->regions[rid].region.get_index_space());
        const double * dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id =
          H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          std::ostringstream os;
          os << "H5Dopen2 failed: " << dataset_id;
          clog_error(os.str());
          H5Fclose(file_id);
          assert(0);
        }
        H5Dwrite(
          dataset_id, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data);
        H5Dclose(dataset_id);
      }
      else {
        assert(0);
      }
    }

    {
      clog_tag_guard(io);
      clog(info) << "Checkpoint data to HDF5 file no attach " << file_name
                 << " region_id " << rid
                 << " (dataset(fid) size= " << field_set.size() << ")"
                 << " field_string_map_vector(regions) size "
                 << field_string_map_vector.size() << std::endl;
    }
  }

  H5Fflush(file_id, H5F_SCOPE_LOCAL);
  H5Fclose(file_id);
} // checkpoint_without_attach_task

inline void
recover_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  const int point = task->index_point.point_data[0];

  const std::byte * task_args = (const std::byte *)task->args;

  std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;

  field_string_map_vector =
    utils::serial_get<std::vector<std::map<Legion::FieldID, std::string>>>(
      task_args);

  std::string fname = utils::serial_get<std::string>(task_args);

  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    Legion::PhysicalRegion attach_src_pr;
    Legion::LogicalRegion output_lr = regions[rid].get_logical_region();
    Legion::LogicalRegion attach_src_lr = runtime->create_logical_region(
      ctx, output_lr.get_index_space(), output_lr.get_field_space());

    Legion::AttachLauncher hdf5_attach_launcher(
      EXTERNAL_HDF5_FILE, attach_src_lr, attach_src_lr);
    std::map<Legion::FieldID, const char *> field_map;
    std::set<Legion::FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<Legion::FieldID, std::string>::iterator map_it;
    for(Legion::FieldID it : field_set) {
      map_it = field_string_map_vector[rid].find(it);
      if(map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(it, (map_it->second).c_str()));
      }
      else {
        assert(0);
      }
    }

    {
      clog_tag_guard(io);
      clog(info) << "Recover data to HDF5 file attach " << file_name
                 << " region_id " << rid
                 << " (dataset(fid) size= " << field_map.size() << ")"
                 << " field_string_map_vector(regions) size "
                 << field_string_map_vector.size() << std::endl;
    }

    hdf5_attach_launcher.attach_hdf5(
      file_name, field_map, LEGION_FILE_READ_WRITE);
    attach_src_pr =
      runtime->attach_external_resource(ctx, hdf5_attach_launcher);

    Legion::CopyLauncher copy_launcher2;
    copy_launcher2.add_copy_requirements(
      Legion::RegionRequirement(
        attach_src_lr, READ_ONLY, EXCLUSIVE, attach_src_lr),
      Legion::RegionRequirement(
        output_lr, WRITE_DISCARD, EXCLUSIVE, output_lr));
    for(Legion::FieldID it : field_set) {
      copy_launcher2.add_src_field(0, it);
      copy_launcher2.add_dst_field(0, it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher2);

    Legion::Future fu =
      runtime->detach_external_resource(ctx, attach_src_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, attach_src_lr);
  }
} // recover_with_attach_task

inline void
recover_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  const int point = task->index_point.point_data[0];

  const std::byte * task_args = (const std::byte *)task->args;

  std::vector<std::map<Legion::FieldID, std::string>> field_string_map_vector;

  field_string_map_vector =
    utils::serial_get<std::vector<std::map<Legion::FieldID, std::string>>>(
      task_args);

  std::string fname = utils::serial_get<std::string>(task_args);

  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file_id < 0) {
    std::ostringstream os;
    os << "H5Fopen failed: " << file_id;
    clog_error(os.str());
    assert(0);
  }

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    std::set<Legion::FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<Legion::FieldID, std::string>::iterator map_it;
    for(Legion::FieldID it : field_set) {
      map_it = field_string_map_vector[rid].find(it);
      if(map_it != field_string_map_vector[rid].end()) {
        const Legion::FieldAccessor<WRITE_DISCARD, double, 1, Legion::coord_t,
          Realm::AffineAccessor<double, 1, Legion::coord_t>>
          acc_fid(regions[rid], it);
        Legion::Rect<1> rect = runtime->get_index_space_domain(
          ctx, task->regions[rid].region.get_index_space());
        double * dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id =
          H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          std::ostringstream os;
          os << "H5Dopen2 failed: " << dataset_id;
          clog_error(os.str());
          H5Fclose(file_id);
          assert(0);
        }
        H5Dread(
          dataset_id, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data);
        H5Dclose(dataset_id);
      }
      else {
        assert(0);
      }
    }

    {
      clog_tag_guard(io);
      clog(info) << "Checkpoint data to HDF5 file no attach " << file_name
                 << " region_id " << rid
                 << " (dataset(fid) size= " << field_set.size() << ")"
                 << " field_string_map_vector(regions) size "
                 << field_string_map_vector.size() << std::endl;
    }
  }
} // recover_without_attach_task

} // namespace io
} // namespace flecsi
