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
#include <hdf5.h>
#include <flecsi/io/legion/io_policy.h>
#define __FLECSI_PRIVATE__
#include <flecsi/utils/const_string.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/internal_task.h>
#include <flecsi/data/legion/runtime_data_types.h>

#include <sys/types.h>
#include <unistd.h>

namespace flecsi {
namespace io {
  
#define SERIALIZATION_BUFFER_SIZE 4096  
  
struct checkpoint_task_args_s{
 size_t field_map_size;
 char field_map_serial[SERIALIZATION_BUFFER_SIZE];
 char file_name[32];
};

legion_hdf5_logical_region_t::legion_hdf5_logical_region_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name, std::map<FieldID, std::string> &field_string_map)
  :logical_region(lr), logical_partition(lp), logical_region_name(lr_name), field_string_map(field_string_map) {
  
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();
  if (lr.get_dim() == 1) {
    Domain domain = runtime->get_index_space_domain(ctx, lr.get_index_space());
    dim_size[0] = domain.get_volume();
    printf("ID logical region size %ld\n", dim_size[0]);
  } else {
    Domain domain = runtime->get_index_space_domain(ctx, lr.get_index_space());
    dim_size[0] = domain.get_volume();
    printf("2D ID logical region size %ld\n", dim_size[0]);
  }
}

legion_hdf5_t::legion_hdf5_t(const char* file_name, int num_files)
  :legion_hdf5_t(std::string(file_name), num_files) {
}

legion_hdf5_t::legion_hdf5_t(std::string file_name, int num_files)
  :file_name(file_name), num_files(num_files) {
  logical_region_vector.clear();  
}

void legion_hdf5_t::add_logical_region(LogicalRegion lr, LogicalPartition lp, std::string lr_name, std::map<FieldID, std::string> field_string_map) {
  legion_hdf5_logical_region_t h5_lr(lr, lp, lr_name, field_string_map);
  logical_region_vector.push_back(h5_lr);
}

bool legion_hdf5_t::generate_hdf5_file(int file_idx) {
  hid_t file_id;
  
  assert(logical_region_vector.size() > 0);

  std::string fname = file_name + std::to_string(file_idx);
  file_id = H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); 
  if(file_id < 0) {
    printf("H5Fcreate failed: %lld\n", (long long)file_id);
    return false;
  }
  
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();

  for (std::vector<legion_hdf5_logical_region_t>::iterator lr_it = logical_region_vector.begin(); lr_it != logical_region_vector.end(); ++lr_it) {
    hid_t dataspace_id = -1;
    if ((*lr_it).logical_region.get_index_space().get_dim() == 1) {
      LogicalRegion sub_lr = runtime->get_logical_subregion_by_color(ctx, (*lr_it).logical_partition, file_idx);
      Domain domain = runtime->get_index_space_domain(ctx, sub_lr.get_index_space());
      hsize_t dims[1];
      dims[0] = domain.get_volume();
      dataspace_id = H5Screate_simple(1, dims, NULL);
    } else {
      LogicalRegion sub_lr = runtime->get_logical_subregion_by_color(ctx, (*lr_it).logical_partition, file_idx);
      Domain domain = runtime->get_index_space_domain(ctx, sub_lr.get_index_space());
      hsize_t dims[1];
      dims[0] = domain.get_volume();
      dataspace_id = H5Screate_simple(1, dims, NULL);
    }
    if(dataspace_id < 0) {
      printf("H5Screate_simple failed: %lld\n", (long long)dataspace_id);
      H5Fclose(file_id);
      return false;
    }
#if 0
    hid_t group_id = H5Gcreate2(file_id, (*lr_it).logical_region_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (group_id < 0) {
      printf("H5Gcreate2 failed: %lld\n", (long long)group_id);
      H5Sclose(dataspace_id);
      H5Fclose(file_id);
      return false;
    }
#endif
    for (std::map<FieldID, std::string>::iterator it = (*lr_it).field_string_map.begin() ; it != (*lr_it).field_string_map.end(); ++it) {
      const char* dataset_name = (it->second).c_str();
      hid_t dataset = H5Dcreate2(file_id, dataset_name,
    			     H5T_IEEE_F64LE, dataspace_id,
    			     H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      if(dataset < 0) {
        printf("H5Dcreate2 failed: %lld\n", (long long)dataset);
    //    H5Gclose(group_id);
        H5Sclose(dataspace_id);
        H5Fclose(file_id);
        return false;
      }
      H5Dclose(dataset);
    }
 //   H5Gclose(group_id);
    H5Sclose(dataspace_id);
  }
  H5Fflush(file_id, H5F_SCOPE_LOCAL);
  H5Fclose(file_id);
  return true;
}

legion_io_policy_t::~legion_io_policy_t() {
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();
  printf("clean up\n");
  if (default_index_topology_file_is != IndexSpace::NO_SPACE) {
    runtime->destroy_index_space(ctx, default_index_topology_file_is);
  }
}

legion_checkpoint_internal_data_t::legion_checkpoint_internal_data_t(LogicalRegion lr, LogicalPartition lp, std::string lr_name)
  : logical_region(lr), logical_partition(lp), logical_region_name(lr_name)
{
  field_string_map.clear();
}

void legion_io_policy_t::add_regions(legion_hdf5_t & hdf5_file, std::vector<legion_checkpoint_internal_data_t> & cp_test_data_vector)
{
  for(std::vector<legion_checkpoint_internal_data_t>::iterator it = cp_test_data_vector.begin() ; it != cp_test_data_vector.end(); ++it) {
    hdf5_file.add_logical_region((*it).logical_region, (*it).logical_partition, (*it).logical_region_name, (*it).field_string_map);
  }
}

void legion_io_policy_t::add_default_index_topology(hdf5_t &hdf5_file, std::vector<std::string> &field_string_vector) {
  constexpr size_t identifier =
    utils::hash::topology_hash<flecsi_internal_string_hash("internal"),
      flecsi_internal_string_hash("index_topology")>();
  
  auto & flecsi_context = execution::context_t::instance();    
  data::legion::index_runtime_data_t & index_runtime_data =
    flecsi_context.index_topology_instance(identifier);
  
  std::map<FieldID, std::string> field_string_map;
  for (std::vector<std::string>::iterator it = field_string_vector.begin() ; it != field_string_vector.end(); ++it) {
    field_string_map.insert(std::make_pair(1, *it));
  }

  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();
  Rect<1> file_color_bounds(0, hdf5_file.num_files-1);
  default_index_topology_file_is = runtime->create_index_space(ctx, file_color_bounds);
#if 0 
  default_index_topology_file_ip = runtime->create_pending_partition(ctx, index_runtime_data.index_space, default_index_topology_file_is);
  int idx = 0; 
  int num_subregions = index_runtime_data.colors;
  for (int point = 0; point < hdf5_file.num_files; point++) {
    std::vector<IndexSpace> subspaces;
    for (int i = 0; i < num_subregions/hdf5_file.num_files; i++) {
      subspaces.push_back(runtime->get_index_subspace(ctx, index_runtime_data.color_partition.get_index_partition(), idx));
      idx ++;
    }
    runtime->create_index_space_union(ctx, default_index_topology_file_ip, point, subspaces);
  }
#else
  default_index_topology_file_ip = runtime->create_equal_partition(ctx, index_runtime_data.index_space, default_index_topology_file_is);
#endif
  default_index_topology_file_lp = runtime->get_logical_partition(ctx, index_runtime_data.logical_region, default_index_topology_file_ip);  
  hdf5_file.add_logical_region(index_runtime_data.logical_region, default_index_topology_file_lp, "null", field_string_map);
}

void legion_io_policy_t::generate_hdf5_files(legion_hdf5_t &hdf5_file)
{
  for (int i = 0; i < hdf5_file.num_files; i++) {
    hdf5_file.generate_hdf5_file(i);
  }
}

void legion_io_policy_t::checkpoint_data(legion_hdf5_t & hdf5_file, IndexSpace launch_space, std::vector<legion_checkpoint_internal_data_t> & cp_test_data_vector, bool attach_flag)
{
  std::string file_name = hdf5_file.file_name;
  printf("Start checkpoint, %s\n", file_name.c_str());
 
#if 1 
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();
  
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  for(std::vector<legion_checkpoint_internal_data_t>::iterator it = cp_test_data_vector.begin() ; it != cp_test_data_vector.end(); ++it) {
    field_string_map_vector.push_back((*it).field_string_map);
  }
  
  struct checkpoint_task_args_s task_argument;
  strcpy(task_argument.file_name, file_name.c_str());
  
  Realm::Serialization::DynamicBufferSerializer dbs(0);
  dbs << field_string_map_vector;
  task_argument.field_map_size = dbs.bytes_used();
  if (task_argument.field_map_size > SERIALIZATION_BUFFER_SIZE) {
    assert(0);
  }
  memcpy(task_argument.field_map_serial, dbs.detach_buffer(), task_argument.field_map_size);
  
  execution::context_t & context_ = execution::context_t::instance();
  auto task_id = 0;
  if (attach_flag == true) {
    task_id = context_.task_id<flecsi_internal_hash(checkpoint_with_attach_task)>();
  } else {
    task_id = context_.task_id<flecsi_internal_hash(checkpoint_without_attach_task)>();
  }
  
  IndexLauncher checkpoint_launcher(task_id, launch_space, TaskArgument(&task_argument, sizeof(task_argument)), ArgumentMap());
  
  int idx = 0;
  for(std::vector<legion_checkpoint_internal_data_t>::iterator it = cp_test_data_vector.begin() ; it != cp_test_data_vector.end(); ++it) {
    checkpoint_launcher.add_region_requirement(
          RegionRequirement((*it).logical_partition, 0/*projection ID*/, 
                            READ_ONLY, EXCLUSIVE, (*it).logical_region));
    
    std::map<FieldID, std::string> &field_string_map = (*it).field_string_map;
    for (std::map<FieldID, std::string>::iterator it = field_string_map.begin() ; it != field_string_map.end(); ++it) {
      checkpoint_launcher.region_requirements[idx].add_field(it->first);
    }
    idx ++;
  }
  FutureMap fumap = runtime->execute_index_space(ctx, checkpoint_launcher);
  fumap.wait_all_results();
#endif
  
}

void legion_io_policy_t::checkpoint_index_topology_field(hdf5_t &hdf5_file, auto fh)
{
  
}

void legion_io_policy_t::recover_data(legion_hdf5_t & hdf5_file, IndexSpace launch_space, std::vector<legion_checkpoint_internal_data_t> & cp_test_data_vector, bool attach_flag)
{
  std::string file_name = hdf5_file.file_name;
  printf("Start recover, %s\n", file_name.c_str());
 
#if 1 
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();
  
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  for(std::vector<legion_checkpoint_internal_data_t>::iterator it = cp_test_data_vector.begin() ; it != cp_test_data_vector.end(); ++it) {
    field_string_map_vector.push_back((*it).field_string_map);
  }
  
  struct checkpoint_task_args_s task_argument;
  strcpy(task_argument.file_name, file_name.c_str());
  
  Realm::Serialization::DynamicBufferSerializer dbs(0);
  dbs << field_string_map_vector;
  task_argument.field_map_size = dbs.bytes_used();
  if (task_argument.field_map_size > SERIALIZATION_BUFFER_SIZE) {
    assert(0);
  }
  memcpy(task_argument.field_map_serial, dbs.detach_buffer(), task_argument.field_map_size);
  
  execution::context_t & context_ = execution::context_t::instance();
  auto task_id = 0;
  if (attach_flag == true) {
    task_id = context_.task_id<flecsi_internal_hash(recover_with_attach_task)>();
  } else {
    task_id = context_.task_id<flecsi_internal_hash(recover_without_attach_task)>();
  }
  
  IndexLauncher recover_launcher(task_id, launch_space, TaskArgument(&task_argument, sizeof(task_argument)), ArgumentMap());
  int idx = 0;
  for(std::vector<legion_checkpoint_internal_data_t>::iterator it = cp_test_data_vector.begin() ; it != cp_test_data_vector.end(); ++it) {
    recover_launcher.add_region_requirement(
          RegionRequirement((*it).logical_partition, 0/*projection ID*/, 
                            WRITE_DISCARD, EXCLUSIVE, (*it).logical_region));
    
    std::map<FieldID, std::string> &field_string_map = (*it).field_string_map;
    for (std::map<FieldID, std::string>::iterator it = field_string_map.begin() ; it != field_string_map.end(); ++it) {
      recover_launcher.region_requirements[idx].add_field(it->first);
    }
    idx ++;
  }
  
  FutureMap fumap = runtime->execute_index_space(ctx, recover_launcher);
  fumap.wait_all_results();
#endif
  
}

void checkpoint_with_attach_task(const Legion::Task * task,                      
    const std::vector<Legion::PhysicalRegion> & regions,                       
    Legion::Context ctx,                                                       
    Legion::Runtime * runtime)
{  
  struct checkpoint_task_args_s task_arg = *(struct checkpoint_task_args_s *) task->args;
  
  const int point = task->index_point.point_data[0];
  
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(task_arg.field_map_serial, task_arg.field_map_size);
  bool ok  = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }
  
  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char *file_name = const_cast<char*>(fname.c_str());
  
  
  for (unsigned int rid = 0; rid < regions.size(); rid++) {
    PhysicalRegion cp_pr;
    LogicalRegion input_lr = regions[rid].get_logical_region();
    LogicalRegion cp_lr = runtime->create_logical_region(ctx, input_lr.get_index_space(), input_lr.get_field_space());
  
    AttachLauncher hdf5_attach_launcher(EXTERNAL_HDF5_FILE, cp_lr, cp_lr);
    std::map<FieldID,const char*> field_map;
    std::set<FieldID> field_set = task->regions[rid].privilege_fields;  
    std::map<FieldID, std::string>::iterator map_it;
    for (std::set<FieldID>::iterator it = field_set.begin() ; it != field_set.end(); ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if (map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(*it, (map_it->second).c_str()));
      } else {
        assert(0);
      }
    }
    printf("Checkpointing data to HDF5 file attach '%s' region %d, (datasets='%ld'), vector size %ld, pid %ld\n", file_name, rid, field_map.size(), field_string_map_vector.size(), getpid());
    hdf5_attach_launcher.attach_hdf5(file_name, field_map, LEGION_FILE_READ_WRITE);
    cp_pr = runtime->attach_external_resource(ctx, hdf5_attach_launcher);
   // cp_pr.wait_until_valid();

    CopyLauncher copy_launcher1;
    copy_launcher1.add_copy_requirements(
        RegionRequirement(input_lr, READ_ONLY, EXCLUSIVE, input_lr),
        RegionRequirement(cp_lr, WRITE_DISCARD, EXCLUSIVE, cp_lr));
    for(std::set<FieldID>::iterator it = field_set.begin(); it != field_set.end(); ++it) {
      copy_launcher1.add_src_field(0, *it);
      copy_launcher1.add_dst_field(0, *it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher1);
  
  
    Future fu = runtime->detach_external_resource(ctx, cp_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, cp_lr);
  }
} // checkpoint_with_attach_task
  
void checkpoint_without_attach_task(const Legion::Task * task,                      
    const std::vector<Legion::PhysicalRegion> & regions,                       
    Legion::Context ctx,                                                       
    Legion::Runtime * runtime)
{
  struct checkpoint_task_args_s task_arg = *(struct checkpoint_task_args_s *) task->args;
  
  const int point = task->index_point.point_data[0];
  
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(task_arg.field_map_serial, task_arg.field_map_size);
  bool ok  = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }
  
  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char *file_name = const_cast<char*>(fname.c_str());
  
  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT); 
  if(file_id < 0) {
    printf("H5Fopen failed: %lld\n", (long long)file_id);
    assert(0);
  }
  
  for (unsigned int rid = 0; rid < regions.size(); rid++) {
    LogicalRegion input_lr = regions[rid].get_logical_region();

    std::set<FieldID> field_set = task->regions[rid].privilege_fields;  
    std::map<FieldID, std::string>::iterator map_it;
    for (std::set<FieldID>::iterator it = field_set.begin() ; it != field_set.end(); ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if (map_it != field_string_map_vector[rid].end()) {
        const FieldAccessor<READ_ONLY,double,1,coord_t, Realm::AffineAccessor<double,1,coord_t> > acc_fid(regions[rid], *it);
        Rect<1> rect = runtime->get_index_space_domain(ctx, task->regions[rid].region.get_index_space());
        const double *dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id = H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          printf("H5Dopen2 failed: %lld\n", (long long)dataset_id);
          H5Fclose(file_id);
          assert(0);
        }
        H5Dwrite(dataset_id, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data);
        H5Dclose(dataset_id);
      } else {
        assert(0);
      }
    }
    printf("Checkpointing data to HDF5 file no attach '%s' region %d, (datasets='%ld'), vector size %ld, pid %ld\n", file_name, rid, field_set.size(), field_string_map_vector.size(), getpid());
  }
  
  H5Fflush(file_id, H5F_SCOPE_LOCAL);
  H5Fclose(file_id);
} // checkpoint_without_attach_task

void recover_with_attach_task(const Legion::Task * task,                      
    const std::vector<Legion::PhysicalRegion> & regions,                       
    Legion::Context ctx,                                                       
    Legion::Runtime * runtime)
{
  const int point = task->index_point.point_data[0];
  
  struct checkpoint_task_args_s task_arg = *(struct checkpoint_task_args_s *) task->args;
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(task_arg.field_map_serial, task_arg.field_map_size);
  bool ok  = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }
  
  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char *file_name = const_cast<char*>(fname.c_str());
  
  for (unsigned int rid = 0; rid < regions.size(); rid++) {
    PhysicalRegion restart_pr;
    LogicalRegion input_lr2 = regions[rid].get_logical_region();
    LogicalRegion restart_lr = runtime->create_logical_region(ctx, input_lr2.get_index_space(), input_lr2.get_field_space());

    AttachLauncher hdf5_attach_launcher(EXTERNAL_HDF5_FILE, restart_lr, restart_lr);
    std::map<FieldID,const char*> field_map;
    std::set<FieldID> field_set = task->regions[rid].privilege_fields;  
    std::map<FieldID, std::string>::iterator map_it;
    for (std::set<FieldID>::iterator it = field_set.begin() ; it != field_set.end(); ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if (map_it != field_string_map_vector[rid].end()) {
        field_map.insert(std::make_pair(*it, (map_it->second).c_str()));
      } else {
        assert(0);
      }
    }
    printf("Recoverring data to HDF5 file attach '%s' region %d, (datasets='%ld'), vector size %ld, pid %ld\n", file_name, rid, field_map.size(), field_string_map_vector.size(), getpid());
    hdf5_attach_launcher.attach_hdf5(file_name, field_map, LEGION_FILE_READ_WRITE);
    restart_pr = runtime->attach_external_resource(ctx, hdf5_attach_launcher);

    CopyLauncher copy_launcher2;
    copy_launcher2.add_copy_requirements(
        RegionRequirement(restart_lr, READ_ONLY, EXCLUSIVE, restart_lr),
        RegionRequirement(input_lr2, WRITE_DISCARD, EXCLUSIVE, input_lr2));
    for(std::set<FieldID>::iterator it = field_set.begin(); it != field_set.end(); ++it) {
      copy_launcher2.add_src_field(0, *it);
      copy_launcher2.add_dst_field(0, *it);
    }
    runtime->issue_copy_operation(ctx, copy_launcher2);

    Future fu = runtime->detach_external_resource(ctx, restart_pr, true);
    fu.wait();
    runtime->destroy_logical_region(ctx, restart_lr);
  }
} // recover_with_attach_task

void recover_without_attach_task(const Legion::Task * task,                      
    const std::vector<Legion::PhysicalRegion> & regions,                       
    Legion::Context ctx,                                                       
    Legion::Runtime * runtime)
{
  const int point = task->index_point.point_data[0];
  
  struct checkpoint_task_args_s task_arg = *(struct checkpoint_task_args_s *) task->args;
  std::vector<std::map<FieldID, std::string>> field_string_map_vector;
  Realm::Serialization::FixedBufferDeserializer fdb(task_arg.field_map_serial, task_arg.field_map_size);
  bool ok  = fdb >> field_string_map_vector;
  if(!ok) {
    printf("task args deserializer error\n");
  }
  
  std::string fname(task_arg.file_name);
  fname = fname + std::to_string(point);
  char *file_name = const_cast<char*>(fname.c_str());
  
  hid_t file_id;
  file_id = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT); 
  if(file_id < 0) {
    printf("H5Fopen failed: %lld\n", (long long)file_id);
    assert(0);
  }
  
  for (unsigned int rid = 0; rid < regions.size(); rid++) {
    LogicalRegion input_lr2 = regions[rid].get_logical_region();

    std::set<FieldID> field_set = task->regions[rid].privilege_fields;  
    std::map<FieldID, std::string>::iterator map_it;
    for (std::set<FieldID>::iterator it = field_set.begin() ; it != field_set.end(); ++it) {
      map_it = field_string_map_vector[rid].find(*it);
      if (map_it != field_string_map_vector[rid].end()) {
        const FieldAccessor<WRITE_DISCARD,double,1,coord_t, Realm::AffineAccessor<double,1,coord_t> > acc_fid(regions[rid], *it);
        Rect<1> rect = runtime->get_index_space_domain(ctx, task->regions[rid].region.get_index_space());
        double *dset_data = acc_fid.ptr(rect.lo);
        hid_t dataset_id = H5Dopen2(file_id, (map_it->second).c_str(), H5P_DEFAULT);
        if(dataset_id < 0) {
          printf("H5Dopen2 failed: %lld\n", (long long)dataset_id);
          H5Fclose(file_id);
          assert(0);
        }
        H5Dread(dataset_id, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data);
        H5Dclose(dataset_id);
      } else {
        assert(0);
      }
    }
    printf("Recoverring data to HDF5 file no attach '%s' region %d, (datasets='%ld'), vector size %ld, pid %ld\n", file_name, rid, field_set.size(), field_string_map_vector.size(), getpid());
  }
} // recover_without_attach_task

flecsi_internal_register_legion_task(checkpoint_with_attach_task,
  processor_type_t::loc,
  inner);

flecsi_internal_register_legion_task(checkpoint_without_attach_task,
  processor_type_t::loc,
  leaf);

flecsi_internal_register_legion_task(recover_with_attach_task,
  processor_type_t::loc,
  inner);  
  
flecsi_internal_register_legion_task(recover_without_attach_task,
  processor_type_t::loc,
  leaf);


} // namespace io
} // namespace flecsi
