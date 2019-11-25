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
#include <flecsi/data.hh>
#include <flecsi/execution.hh>
#include <flecsi/io.hh>
#include <flecsi/utils/ftest.hh>

#include <legion.h>
#include <mpi.h>

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

using namespace Legion;

using namespace flecsi;

void
check() {

  FTEST();

} // check

enum FieldIDs {
  FID_X,
  FID_Y,
};

int
io_sanity(int, char **) {

  int num_elements = 1023;
  int num_files = 16;
  char file_name[256];
  strcpy(file_name, "checkpoint.dat");

  // Check for any command line arguments
  {
    const InputArgs & command_args = Runtime::get_input_args();
    for(int i = 1; i < command_args.argc; i++) {
      if(!strcmp(command_args.argv[i], "-s"))
        num_elements = atoi(command_args.argv[++i]);
      if(!strcmp(command_args.argv[i], "-m"))
        num_files = atoi(command_args.argv[++i]);
      if(!strcmp(command_args.argv[i], "-f"))
        strcpy(file_name, command_args.argv[++i]);
    }
  }

  std::cout << "Running for " << num_elements << " elements, pid " << getpid()
            << std::endl;

  Runtime * runtime = Runtime::get_runtime();
  Context ctx = Runtime::get_context();

  Rect<1> elem_rect_1(0, num_elements - 1);
  Rect<1> elem_rect_2(0, num_elements * 2 - 1);
  IndexSpace is_1 = runtime->create_index_space(ctx, elem_rect_1);
  IndexSpace is_2 = runtime->create_index_space(ctx, elem_rect_2);
  FieldSpace input_fs = runtime->create_field_space(ctx);
  {
    FieldAllocator allocator = runtime->create_field_allocator(ctx, input_fs);
    allocator.allocate_field(sizeof(double), FID_X);
    allocator.allocate_field(sizeof(double), FID_Y);
  }

  Rect<1> file_color_bounds(0, num_files - 1);
  IndexSpace file_is = runtime->create_index_space(ctx, file_color_bounds);
  IndexPartition file_ip_1 =
    runtime->create_equal_partition(ctx, is_1, file_is);
  IndexPartition file_ip_2 =
    runtime->create_equal_partition(ctx, is_2, file_is);

  LogicalRegion input_lr_1 =
    runtime->create_logical_region(ctx, is_1, input_fs);
  LogicalRegion output_lr_1 =
    runtime->create_logical_region(ctx, is_1, input_fs);
  LogicalRegion input_lr_2 =
    runtime->create_logical_region(ctx, is_2, input_fs);
  LogicalRegion output_lr_2 =
    runtime->create_logical_region(ctx, is_2, input_fs);

  LogicalPartition file_checkpoint_lp_input_1 =
    runtime->get_logical_partition(ctx, input_lr_1, file_ip_1);
  LogicalPartition file_checkpoint_lp_input_2 =
    runtime->get_logical_partition(ctx, input_lr_2, file_ip_2);

  LogicalPartition file_recover_lp_output_1 =
    runtime->get_logical_partition(ctx, output_lr_1, file_ip_1);
  LogicalPartition file_recover_lp_output_2 =
    runtime->get_logical_partition(ctx, output_lr_2, file_ip_2);

  io::hdf5_region_t cp_test_data_1(
    input_lr_1, file_checkpoint_lp_input_1, "input_lr_1");
  cp_test_data_1.field_string_map[FID_X] = "A_FID_X";
  cp_test_data_1.field_string_map[FID_Y] = "A_FID_Y";

  io::hdf5_region_t cp_test_data_2(
    input_lr_2, file_checkpoint_lp_input_2, "input_lr_2");
  cp_test_data_2.field_string_map[FID_X] = "B_FID_X";
  cp_test_data_2.field_string_map[FID_Y] = "B_FID_Y";

  std::vector<io::hdf5_region_t> cp_test_data_vector;
  cp_test_data_vector.push_back(cp_test_data_1);
  cp_test_data_vector.push_back(cp_test_data_2);

  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  io::io_interface_t cp_io;
  io::hdf5_t checkpoint_file = cp_io.init_hdf5_file(file_name, num_files);
  cp_io.add_regions(checkpoint_file, cp_test_data_vector);
  if(my_rank == 0) {
    cp_io.generate_hdf5_files(checkpoint_file);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  int ct = 0;

  {
    RegionRequirement req(input_lr_1, READ_WRITE, EXCLUSIVE, input_lr_1);
    req.add_field(FID_X);
    req.add_field(FID_Y);

    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE, double, 1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE, double, 1> acc_y(input_region, FID_Y);
    for(PointInRectIterator<1> pir(elem_rect_1); pir(); pir++) {
      acc_x[*pir] = 0.29 + ct;
      acc_y[*pir] = 0.29 + ct;
      ct++;
    }
    runtime->unmap_region(ctx, input_region);
  }

  {
    ct = 0;
    RegionRequirement req(input_lr_2, READ_WRITE, EXCLUSIVE, input_lr_2);
    req.add_field(FID_X);
    req.add_field(FID_Y);

    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE, double, 1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE, double, 1> acc_y(input_region, FID_Y);
    for(PointInRectIterator<1> pir(elem_rect_2); pir(); pir++) {
      acc_x[*pir] = 0.29 + ct;
      acc_y[*pir] = 0.29 + ct;
      ct++;
    }
    runtime->unmap_region(ctx, input_region);
  }

  cp_io.checkpoint_data(checkpoint_file, file_is, cp_test_data_vector, true);

  io::hdf5_region_t re_test_data_1(
    output_lr_1, file_recover_lp_output_1, "output_lr_1");
  re_test_data_1.field_string_map[FID_X] = "A_FID_X";
  re_test_data_1.field_string_map[FID_Y] = "A_FID_Y";

  io::hdf5_region_t re_test_data_2(
    output_lr_2, file_recover_lp_output_2, "output_lr_2");
  re_test_data_2.field_string_map[FID_X] = "B_FID_X";
  re_test_data_2.field_string_map[FID_Y] = "B_FID_Y";

  std::vector<io::hdf5_region_t> re_test_data_vector;
  re_test_data_vector.push_back(re_test_data_1);
  // re_test_data_vector.push_back(re_test_data_2);

  cp_io.recover_data(checkpoint_file, file_is, re_test_data_vector, false);

  {
    ct = 0;
    RegionRequirement req(output_lr_1, READ_WRITE, EXCLUSIVE, output_lr_1);
    req.add_field(FID_X);
    req.add_field(FID_Y);

    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE, double, 1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE, double, 1> acc_y(input_region, FID_Y);
    for(PointInRectIterator<1> pir(elem_rect_1); pir(); pir++) {
      if(acc_x[*pir] != 0.29 + ct)
        std::abort();
      if(acc_y[*pir] != 0.29 + ct)
        std::abort();
      ct++;
    }
    runtime->unmap_region(ctx, input_region);
  }
#if 0  
  {
    ct = 0;
    RegionRequirement req(output_lr_2, READ_WRITE, EXCLUSIVE, output_lr_2);
    req.add_field(FID_X);
    req.add_field(FID_Y);
    
    InlineLauncher input_launcher(req);
    PhysicalRegion input_region = runtime->map_region(ctx, input_launcher);
    input_region.wait_until_valid();
    const FieldAccessor<READ_WRITE,double,1> acc_x(input_region, FID_X);
    const FieldAccessor<READ_WRITE,double,1> acc_y(input_region, FID_Y);
    for (PointInRectIterator<1> pir(elem_rect_2); pir(); pir++) {
      if(acc_x[*pir] != 0.29 + ct) std::abort();
      if(acc_y[*pir] != 0.29 + ct) std::abort();
      ct ++;
    }
    runtime->unmap_region(ctx, input_region);
  }
#endif

  execute<check>();

  runtime->destroy_logical_region(ctx, input_lr_1);
  runtime->destroy_logical_region(ctx, input_lr_2);
  runtime->destroy_logical_region(ctx, output_lr_1);
  runtime->destroy_logical_region(ctx, output_lr_2);
  runtime->destroy_field_space(ctx, input_fs);
  runtime->destroy_index_space(ctx, file_is);
  runtime->destroy_index_space(ctx, is_1);
  runtime->destroy_index_space(ctx, is_2);

  return 0;
} // io_sanity

ftest_register_driver(io_sanity);
