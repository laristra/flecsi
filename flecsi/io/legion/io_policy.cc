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
#include <fstream>
#define __FLECSI_PRIVATE__
#include "flecsi/runtime/backend.hh"
#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/field_info.hh>
#include <flecsi/data/data.hh>
#include <flecsi/data/legion/runtime_data_types.hh>
#include <flecsi/io/legion/io_policy.hh>
#include <flecsi/utils/const_string.hh>

#include <sys/types.h>
#include <unistd.h>

namespace flecsi {
namespace io {

void
checkpoint_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  struct checkpoint_task_args_s task_arg =
    *(struct checkpoint_task_args_s *)task->args;

  const int point = task->index_point.point_data[0];

  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(
    task_arg.field_map_serial, task_arg.field_map_size);
  bool ok = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }

  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    PhysicalRegion cp_pr;
    LogicalRegion input_lr = regions[rid].get_logical_region();
    LogicalRegion cp_lr = runtime->create_logical_region(
      ctx, input_lr.get_index_space(), input_lr.get_field_space());

    AttachLauncher hdf5_attach_launcher(EXTERNAL_HDF5_FILE, cp_lr, cp_lr);
    std::map<FieldID, const char *> field_map;
    std::set<FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<FieldID, std::string>::iterator map_it;
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if(map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(*it, (map_it->second).c_str()));
      }
      else {
        assert(0);
      }
    }
    printf("Checkpointing data to HDF5 file attach '%s' region %d, "
           "(datasets='%ld'), vector size %ld, pid %ld\n",
      file_name,
      rid,
      field_map.size(),
      field_string_map_vector.size(),
      getpid());
    hdf5_attach_launcher.attach_hdf5(
      file_name, field_map, LEGION_FILE_READ_WRITE);
    cp_pr = runtime->attach_external_resource(ctx, hdf5_attach_launcher);
    // cp_pr.wait_until_valid();

    CopyLauncher copy_launcher1;
    copy_launcher1.add_copy_requirements(
      RegionRequirement(input_lr, READ_ONLY, EXCLUSIVE, input_lr),
      RegionRequirement(cp_lr, WRITE_DISCARD, EXCLUSIVE, cp_lr));
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      copy_launcher1.add_src_field(0, *it);
      copy_launcher1.add_dst_field(0, *it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher1);

    Future fu = runtime->detach_external_resource(ctx, cp_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, cp_lr);
  }
} // checkpoint_with_attach_task

void
checkpoint_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  struct checkpoint_task_args_s task_arg =
    *(struct checkpoint_task_args_s *)task->args;

  const int point = task->index_point.point_data[0];

  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(
    task_arg.field_map_serial, task_arg.field_map_size);
  bool ok = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }

  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file_id < 0) {
    flog(error) << "H5Fopen failed: " << file_id << std::endl;
    assert(0);
  }

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    LogicalRegion input_lr = regions[rid].get_logical_region();

    std::set<FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<FieldID, std::string>::iterator map_it;
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if(map_it != field_string_map_vector[rid].end()) {
        const FieldAccessor<READ_ONLY,
          double,
          1,
          coord_t,
          Realm::AffineAccessor<double, 1, coord_t>>
          acc_fid(regions[rid], *it);
        Rect<1> rect = runtime->get_index_space_domain(
          ctx, task->regions[rid].region.get_index_space());
        const double * dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id =
          H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          flog(error) << "H5Dopen2 failed: " << dataset_id << std::endl;
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
    printf("Checkpointing data to HDF5 file no attach '%s' region %d, "
           "(datasets='%ld'), vector size %ld, pid %ld\n",
      file_name,
      rid,
      field_set.size(),
      field_string_map_vector.size(),
      getpid());
  }

  H5Fflush(file_id, H5F_SCOPE_LOCAL);
  H5Fclose(file_id);
} // checkpoint_without_attach_task

void
recover_with_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  const int point = task->index_point.point_data[0];

  struct checkpoint_task_args_s task_arg =
    *(struct checkpoint_task_args_s *)task->args;
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(
    task_arg.field_map_serial, task_arg.field_map_size);
  bool ok = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }

  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    PhysicalRegion restart_pr;
    LogicalRegion input_lr2 = regions[rid].get_logical_region();
    LogicalRegion restart_lr = runtime->create_logical_region(
      ctx, input_lr2.get_index_space(), input_lr2.get_field_space());

    AttachLauncher hdf5_attach_launcher(
      EXTERNAL_HDF5_FILE, restart_lr, restart_lr);
    std::map<FieldID, const char *> field_map;
    std::set<FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<FieldID, std::string>::iterator map_it;
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if(map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(*it, (map_it->second).c_str()));
      }
      else {
        assert(0);
      }
    }
    printf("Recoverring data to HDF5 file attach '%s' region %d, "
           "(datasets='%ld'), vector size %ld, pid %ld\n",
      file_name,
      rid,
      field_map.size(),
      field_string_map_vector.size(),
      getpid());
    hdf5_attach_launcher.attach_hdf5(
      file_name, field_map, LEGION_FILE_READ_WRITE);
    restart_pr = runtime->attach_external_resource(ctx, hdf5_attach_launcher);

    CopyLauncher copy_launcher2;
    copy_launcher2.add_copy_requirements(
      RegionRequirement(restart_lr, READ_ONLY, EXCLUSIVE, restart_lr),
      RegionRequirement(input_lr2, WRITE_DISCARD, EXCLUSIVE, input_lr2));
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      copy_launcher2.add_src_field(0, *it);
      copy_launcher2.add_dst_field(0, *it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher2);

    Future fu = runtime->detach_external_resource(ctx, restart_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, restart_lr);
  }
} // recover_with_attach_task

void
recover_without_attach_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime) {
  const int point = task->index_point.point_data[0];

  struct checkpoint_task_args_s task_arg =
    *(struct checkpoint_task_args_s *)task->args;
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(
    task_arg.field_map_serial, task_arg.field_map_size);
  bool ok = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }

  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char * file_name = const_cast<char *>(fname.c_str());

  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);
  if(file_id < 0) {
    flog(error) << "H5Fopen failed: " << file_id << std::endl;
    assert(0);
  }

  for(unsigned int rid = 0; rid < regions.size(); rid++) {
    LogicalRegion input_lr2 = regions[rid].get_logical_region();

    std::set<FieldID> field_set = task->regions[rid].privilege_fields;
    std::map<FieldID, std::string>::iterator map_it;
    for(std::set<FieldID>::iterator it = field_set.begin();
        it != field_set.end();
        ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if(map_it != field_string_map_vector[rid].end()) {
        const FieldAccessor<WRITE_DISCARD,
          double,
          1,
          coord_t,
          Realm::AffineAccessor<double, 1, coord_t>>
          acc_fid(regions[rid], *it);
        Rect<1> rect = runtime->get_index_space_domain(
          ctx, task->regions[rid].region.get_index_space());
        double * dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id =
          H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          flog(error) << "H5Dopen2 failed: " << dataset_id << std::endl;
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
    printf("Recoverring data to HDF5 file no attach '%s' region %d, "
           "(datasets='%ld'), vector size %ld, pid %ld\n",
      file_name,
      rid,
      field_set.size(),
      field_string_map_vector.size(),
      getpid());
  }
} // recover_without_attach_task

} // namespace io
} // namespace flecsi
