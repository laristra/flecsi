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
#include <flecsi/data/data.hh>
#include <flecsi/execution/execution.hh>
#include <flecsi/io/io_interface.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>

#include <assert.h>
#include <mpi.h>

using namespace flecsi;

using index_field_t = index_field_member_u<double>;
const index_field_t test_value_1;
const index_field_t test_value_2;
const index_field_t test_value_3;

const auto fh1 = test_value_1(flecsi_index_topology);
const auto fh2 = test_value_2(flecsi_index_topology);
const auto fh3 = test_value_3(flecsi_index_topology);

void
assign(index_field_t::accessor<rw> ia) {
  ia = color();
} // assign

void
reset_zero(index_field_t::accessor<rw> ia) {
  ia = -1;
} // assign

int
check(index_field_t::accessor<ro> ia) {

  FTEST();

  ASSERT_EQ(ia, color());

  return FTEST_RESULT();
} // print

int
index_topology(int argc, char ** argv) {

  char file_name[256];
  strcpy(file_name, "checkpoint.dat");

  execute<assign>(fh1);
  execute<assign>(fh2);
  execute<assign>(fh3);

  int my_rank = process();
  int num_files = 4;
  io::io_interface_t cp_io;
  io::hdf5_t checkpoint_file = cp_io.init_hdf5_file(file_name, num_files);

  cp_io.add_default_index_topology(checkpoint_file);
#if 1
  if(my_rank == 0) {
    cp_io.generate_hdf5_files(checkpoint_file);
  }
#else
  int num_ranks = processes();
  assert(num_ranks % num_files == 0);
  int num_ranks_per_file = num_ranks / num_files;
  if(my_rank % num_ranks_per_file == 0) {
    cp_io.create_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file);
    cp_io.create_datasets_for_regions(
      checkpoint_file, my_rank / num_ranks_per_file);
    cp_io.close_hdf5_file(checkpoint_file);
  }
#endif
  MPI_Barrier(MPI_COMM_WORLD);

#if 1
  // cp_io.checkpoint_default_index_topology(checkpoint_file);
  cp_io.checkpoint_index_topology_field(checkpoint_file, fh1);
  cp_io.checkpoint_index_topology_field(checkpoint_file, fh2);
  cp_io.checkpoint_index_topology_field(checkpoint_file, fh3);

  execute<reset_zero>(fh1);
  execute<reset_zero>(fh2);
  execute<reset_zero>(fh3);

  // flecsi_execute_task(check, index_test, index, fh1);
  // flecsi_execute_task(check, index_test, index, fh2);

  cp_io.recover_default_index_topology(checkpoint_file);
  // cp_io.recover_index_topology_field(checkpoint_file, fh1);
  // cp_io.recover_index_topology_field(checkpoint_file, fh2);

  execute<check>(fh1);
  execute<check>(fh2);
  execute<check>(fh3);
#endif
  return 0;
} // index

ftest_register_driver(index_topology);
