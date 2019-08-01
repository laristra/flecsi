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
using namespace flecsi::data;
using namespace flecsi::topology;

using index_field_t = index_field_member_u<double>;
const index_field_t field(3);
const auto fh1 = field(flecsi_index_topology, 0);
const auto fh2 = field(flecsi_index_topology, 1);
const auto fh3 = field(flecsi_index_topology, 2);

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
index_driver(int argc, char ** argv) {

  char file_name[256];
  strcpy(file_name, "checkpoint.dat");

  execute<assign>(fh1);
  execute<assign>(fh2);
  execute<assign>(fh3);

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
    cp_io.checkpoint_default_index_topology(checkpoint_file);
  //cp_io.checkpoint_default_index_topology_field(checkpoint_file, fh1);
  //cp_io.checkpoint_default_index_topology_field(checkpoint_file, fh2);
  //cp_io.checkpoint_default_index_topology_field(checkpoint_file, fh3);

  execute<reset_zero>(fh1);
  execute<reset_zero>(fh2);
  execute<reset_zero>(fh3);

  // execute<check>(fh1);
  // execute<check>(fh2);
  
#if 1
  int num_ranks = flecsi_context.processes();
  assert(num_ranks % num_files == 0);
  int num_ranks_per_file = num_ranks / num_files;
  if(my_rank % num_ranks_per_file == 0) {
    cp_io.open_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file);
    std::string str2("test string 2");
    cp_io.write_string_to_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file, "control", "ds2", str2.c_str(), str2.size());

    std::string str3("");
    cp_io.read_string_from_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file, "control", "ds2", str3);
    printf("str 3 %s\n", str3.c_str());
    cp_io.close_hdf5_file(checkpoint_file);
  }
#endif

  cp_io.recover_default_index_topology(checkpoint_file);
  // cp_io.recover_default_index_topology_field(checkpoint_file, fh1);
  // cp_io.recover_default_index_topology_field(checkpoint_file, fh2);

  execute<check>(fh1);
  execute<check>(fh2);
  execute<check>(fh3);
#endif
  

  return 0;
} // index

ftest_register_driver(index_driver);
