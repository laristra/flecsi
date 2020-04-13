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
#include "flecsi/util/demangle.hh"
#include "flecsi/util/ftest.hh"
#include <flecsi/data.hh>
#include <flecsi/execution.hh>
#include <flecsi/io.hh>

#include <assert.h>
#include <mpi.h>

using namespace flecsi;
using namespace flecsi::data;

using index_field_t = index_field_member<double>;
const index_field_t test_value_1;
const index_field_t test_value_2;
const index_field_t test_value_3;

const auto fh1 = test_value_1(process_topology);
const auto fh2 = test_value_2(process_topology);
const auto fh3 = test_value_3(process_topology);

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
  FTEST { ASSERT_EQ(ia, color()); };
} // print

int
index_driver(int, char **) {
  FTEST {
    char file_name[256];
    strcpy(file_name, "checkpoint.dat");

    execute<assign>(fh1);
    execute<assign>(fh2);
    execute<assign>(fh3);

    auto & flecsi_context = run::context::instance();
    int my_rank = flecsi_context.process();
    int num_files = 4;
    io::io_interface_t cp_io;
    io::hdf5_t checkpoint_file = io::init_hdf5_file(file_name, num_files);

    cp_io.add_process_topology(checkpoint_file);
#if 1
    if(my_rank == 0) {
      io::generate_hdf5_files(checkpoint_file);
    }
#else
    int num_ranks = flecsi_context.processes();
    assert(num_ranks % num_files == 0);
    int num_ranks_per_file = num_ranks / num_files;
    if(my_rank % num_ranks_per_file == 0) {
      io::create_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file);
      io::create_datasets_for_regions(
        checkpoint_file, my_rank / num_ranks_per_file);
      io::close_hdf5_file(checkpoint_file);
    }
#endif
    MPI_Barrier(MPI_COMM_WORLD);

#if 1
    // cp_io.checkpoint_process_topology(checkpoint_file);
    cp_io.checkpoint_index_topology_field(checkpoint_file, fh1);
    cp_io.checkpoint_index_topology_field(checkpoint_file, fh2);
    cp_io.checkpoint_index_topology_field(checkpoint_file, fh3);

    execute<reset_zero>(fh1);
    execute<reset_zero>(fh2);
    execute<reset_zero>(fh3);

    // EXPECT_EQ(test<check>(fh1),0);
    // EXPECT_EQ(test<check>(fh2),0);

#if 1
    int num_ranks = flecsi_context.processes();
    assert(num_ranks % num_files == 0);
    int num_ranks_per_file = num_ranks / num_files;
    if(my_rank % num_ranks_per_file == 0) {
      io::open_hdf5_file(checkpoint_file, my_rank / num_ranks_per_file);
      std::string str2("test string 2");
      io::write_string_to_hdf5_file(checkpoint_file,
        my_rank / num_ranks_per_file,
        "control",
        "ds2",
        str2,
        str2.size());

      std::string str3;
      io::read_string_from_hdf5_file(
        checkpoint_file, my_rank / num_ranks_per_file, "control", "ds2", str3);
      // printf("str 3 %s\n", str3.c_str());
      io::close_hdf5_file(checkpoint_file);
    }
#endif

    cp_io.recover_process_topology(checkpoint_file);
    // cp_io.recover_process_topology_field(checkpoint_file, fh1);
    // cp_io.recover_process_topology_field(checkpoint_file, fh2);

    EXPECT_EQ(test<check>(fh1), 0);
    EXPECT_EQ(test<check>(fh2), 0);
    EXPECT_EQ(test<check>(fh3), 0);
#endif
  };
} // index

ftest_register_driver(index_driver);
