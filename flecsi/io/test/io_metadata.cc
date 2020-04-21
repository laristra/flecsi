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
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>

#include <assert.h>

using namespace flecsi;

int
index_topology(int, char **) {
  FTEST {
    char file_name[256];
    strcpy(file_name, "checkpoint.dat");

    int my_rank = process();
    int num_files = processes();
    io::hdf5_t checkpoint_file = io::init_hdf5_file(file_name, num_files);

    // create hdf5 file and checkpoint
    io::create_hdf5_file(checkpoint_file, my_rank);

    std::string str1("control_ds1");
    io::write_string_to_hdf5_file(
      checkpoint_file, my_rank, "control", "ds1", str1, str1.size());

    io::close_hdf5_file(checkpoint_file);

    // re-open and continute checkpoint
    io::open_hdf5_file(checkpoint_file, my_rank);

    std::string str2("control_ds2");
    io::write_string_to_hdf5_file(
      checkpoint_file, my_rank, "control", "ds2", str2, str2.size());

    std::string str3("topology_ds1");
    io::write_string_to_hdf5_file(
      checkpoint_file, my_rank, "topology", "ds1", str3, str3.size());

    io::close_hdf5_file(checkpoint_file);

    // recover
    io::open_hdf5_file(checkpoint_file, my_rank);

    std::string str1_recover;
    io::read_string_from_hdf5_file(
      checkpoint_file, my_rank, "control", "ds1", str1_recover);

    flog(info) << "str1 reover " << str1_recover << std::endl;
    ASSERT_EQ(str1_recover, "control_ds1");

    std::string str2_recover;
    io::read_string_from_hdf5_file(
      checkpoint_file, my_rank, "control", "ds2", str2_recover);

    flog(info) << "str2 reover " << str2_recover << std::endl;
    ASSERT_EQ(str2_recover, "control_ds2");

    std::string str3_recover;
    io::read_string_from_hdf5_file(
      checkpoint_file, my_rank, "topology", "ds1", str3_recover);

    flog(info) << "str3 reover " << str3_recover << std::endl;
    ASSERT_EQ(str3_recover, "topology_ds1");

    io::close_hdf5_file(checkpoint_file);
  };
} // index

ftest_register_driver(index_topology);
