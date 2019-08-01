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

flecsi_add_index_field("test", "value", double, 3);
inline auto fh1 = flecsi_index_field_instance("test", "value", 0);
inline auto fh2 = flecsi_index_field_instance("test", "value", 1);
inline auto fh3 = flecsi_index_field_instance("test", "value", 2);

template<size_t PRIVILEGES>
using accessor =
  flecsi::data::index_accessor_u<double, privilege_pack_u<PRIVILEGES>::value>;

namespace index_test {

void
assign(accessor<rw> ia) {
  ia = flecsi_color();
} // assign

flecsi_register_task(assign, index_test, loc, index);

void
reset_zero(accessor<rw> ia) {
  ia = -1;
} // assign

flecsi_register_task(reset_zero, index_test, loc, index);

int
check(accessor<ro> ia) {

  FTEST();

  ASSERT_EQ(ia, flecsi_color());

  return FTEST_RESULT();
} // print

flecsi_register_task(check, index_test, loc, index);

} // namespace index_test

int
index_topology(int argc, char ** argv) {

  char file_name[256];
  strcpy(file_name, "checkpoint.dat");

  flecsi_execute_task(assign, index_test, index, fh1);
  flecsi_execute_task(assign, index_test, index, fh2);
  flecsi_execute_task(assign, index_test, index, fh3);

  auto & flecsi_context = execution::context_t::instance();
  int my_rank = flecsi_context.process();
  int num_files = 4;
  io::io_interface_t cp_io;
  io::hdf5_t checkpoint_file = cp_io.init_hdf5_file(file_name, num_files);

  cp_io.add_default_index_topology(checkpoint_file);
#if 1
  if(my_rank == 0) {
    cp_io.generate_hdf5_files(checkpoint_file);
  }
#else
  int num_ranks = flecsi_context.processes();
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

  flecsi_execute_task(reset_zero, index_test, index, fh1);
  flecsi_execute_task(reset_zero, index_test, index, fh2);
  flecsi_execute_task(reset_zero, index_test, index, fh3);

  // flecsi_execute_task(check, index_test, index, fh1);
  // flecsi_execute_task(check, index_test, index, fh2);

  cp_io.recover_default_index_topology(checkpoint_file);
  // cp_io.recover_index_topology_field(checkpoint_file, fh1);
  // cp_io.recover_index_topology_field(checkpoint_file, fh2);

  flecsi_execute_task(check, index_test, index, fh1);
  flecsi_execute_task(check, index_test, index, fh2);
  flecsi_execute_task(check, index_test, index, fh3);
#endif
  return 0;
} // index

ftest_register_driver(index_topology);
