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

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.h>
#include <flecsi/io/io_interface.h>
#include <flecsi/utils/ftest.h>

#include <mpi.h>
#include <assert.h>
#include <legion.h>

#include <sys/types.h>
#include <unistd.h>

using namespace Legion;

using namespace flecsi;

namespace io_test {
  
void
check() {

  FTEST();

} // check

flecsi_register_task(check, io_test, loc, index);

} // namespace io_test

enum FieldIDs {
  FID_X,
  FID_Y, 
};

int
io_sanity(int argc, char ** argv) {
  
  int num_elements = 63; 
  int num_subregions = 4;
  int num_files = 4;
  char file_name[256];
  strcpy(file_name, "checkpoint.dat");
  
  // Check for any command line arguments
  {
    const InputArgs &command_args = Runtime::get_input_args();
    for (int i = 1; i < command_args.argc; i++)
    {
      if (!strcmp(command_args.argv[i],"-n"))
        num_subregions = atoi(command_args.argv[++i]);
      if (!strcmp(command_args.argv[i],"-s"))
        num_elements = atoi(command_args.argv[++i]);
      if (!strcmp(command_args.argv[i],"-m"))
        num_files = atoi(command_args.argv[++i]);
      if (!strcmp(command_args.argv[i],"-f"))
	      strcpy(file_name, command_args.argv[++i]);
    }
  }
  
  printf("Running for %d elements, pid %ld\n", num_elements, getpid());
  
  Runtime *runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();

  Rect<1> elem_rect(0,num_elements-1);
  IndexSpace is = runtime->create_index_space(ctx, elem_rect);
  FieldSpace input_fs = runtime->create_field_space(ctx);
  {
    FieldAllocator allocator = 
      runtime->create_field_allocator(ctx, input_fs);
    allocator.allocate_field(sizeof(double),FID_X);
    allocator.allocate_field(sizeof(double),FID_Y);
  }
  
  Rect<1> file_color_bounds(0,num_files-1);
  IndexSpace file_is = runtime->create_index_space(ctx, file_color_bounds);
  IndexPartition file_ip = runtime->create_equal_partition(ctx, is, file_is);
  
  LogicalRegion input_lr_1 = runtime->create_logical_region(ctx, is, input_fs);
  LogicalRegion output_lr_1 = runtime->create_logical_region(ctx, is, input_fs);
  
  LogicalPartition file_checkpoint_lp_input_1 = runtime->get_logical_partition(ctx, input_lr_1, file_ip);
  LogicalPartition file_recover_lp_output_1 = runtime->get_logical_partition(ctx, output_lr_1, file_ip);
  
  std::map<FieldID, std::string> field_string_map_1;
  field_string_map_1[FID_X] = "FID_X";
  field_string_map_1[FID_Y] = "FID_Y";
  
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  io::hdf5_t checkpoint_file(file_name, num_files);
  if (my_rank == 0) { 
    checkpoint_file.add_logical_region(input_lr_1, file_checkpoint_lp_input_1, "input_lr_1", field_string_map_1);
    for (int i = 0; i < num_files; i++) {
      checkpoint_file.generate_hdf5_file(i);
    }
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  {
    RegionRequirement req(input_lr_1, READ_WRITE, EXCLUSIVE, input_lr_1);
    req.add_field(FID_X);
    req.add_field(FID_Y);
    
    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE,double,1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE,double,1> acc_y(input_region, FID_Y);
    for (PointInRectIterator<1> pir(elem_rect); pir(); pir++) {
      acc_x[*pir] = 0.29;
      acc_y[*pir] = 0.29;
    }
    runtime->unmap_region(ctx, input_region);
  }
  
  io::cp_test_data_t cp_test_data;
  cp_test_data.logical_region_vector.push_back(input_lr_1);
  cp_test_data.logical_partition_vector.push_back(file_checkpoint_lp_input_1);
  
  cp_test_data.field_string_map[FID_X] = "FID_X";
  cp_test_data.field_string_map[FID_Y] = "FID_Y";
  cp_test_data.field_id_vector.push_back(FID_X);
  cp_test_data.field_id_vector.push_back(FID_Y);
  
  cp_test_data.launch_space = file_is;
  
  io::io_interface_t cp_io;
  cp_io.checkpoint_data(checkpoint_file, cp_test_data, true);
  
  
  io::cp_test_data_t re_test_data;
  re_test_data.logical_region_vector.push_back(output_lr_1);
  re_test_data.logical_partition_vector.push_back(file_recover_lp_output_1);
  
  re_test_data.field_string_map[FID_X] = "FID_X";
  re_test_data.field_string_map[FID_Y] = "FID_Y";
  re_test_data.field_id_vector.push_back(FID_X);
  re_test_data.field_id_vector.push_back(FID_Y);
  
  re_test_data.launch_space = file_is;
  cp_io.recover_data(checkpoint_file, re_test_data, true);
  
  {
    RegionRequirement req(output_lr_1, READ_WRITE, EXCLUSIVE, output_lr_1);
    req.add_field(FID_X);
    req.add_field(FID_Y);
    
    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE,double,1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE,double,1> acc_y(input_region, FID_Y);
    for (PointInRectIterator<1> pir(elem_rect); pir(); pir++) {
      double x = acc_x[*pir];
      double y = acc_y[*pir];
      assert(x == 0.29);
      assert(y == 0.29);
    }
    runtime->unmap_region(ctx, input_region);
  }


  flecsi_execute_task(check, io_test, index);
  
  runtime->destroy_logical_region(ctx, input_lr_1);
  runtime->destroy_logical_region(ctx, output_lr_1);
  runtime->destroy_field_space(ctx, input_fs);
  runtime->destroy_index_space(ctx, file_is);
  runtime->destroy_index_space(ctx, is);

  return 0;
} // io_sanity

ftest_register_driver(io_sanity);
