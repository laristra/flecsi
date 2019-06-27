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
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/ftest.hh>
#include <flecsi/io/io_interface.hh>

#include <assert.h>

using namespace flecsi;

int
index_topology(int argc, char ** argv) {
  
  char file_name[256];
  strcpy(file_name, "checkpoint.dat");
  
  auto & flecsi_context = execution::context_t::instance();

  int my_rank = flecsi_context.process();
  int num_files = flecsi_context.processes();
  io::io_interface_t cp_io;
  io::hdf5_t checkpoint_file = cp_io.init_hdf5_file(file_name, num_files);
  
  cp_io.open_hdf5_file(checkpoint_file, my_rank);
  
  std::string str1("test string 1");
  cp_io.write_string_to_hdf5_file(checkpoint_file, my_rank, "control", "ds1", str1.c_str(), str1.size());
  
  cp_io.close_hdf5_file(checkpoint_file, my_rank);
  
  cp_io.open_hdf5_file(checkpoint_file, my_rank);
#if 1
  std::string str2("test string 2");
  cp_io.write_string_to_hdf5_file(checkpoint_file, my_rank, "control", "ds2", str2.c_str(), str2.size());
  
  cp_io.write_string_to_hdf5_file(checkpoint_file, my_rank, "topology", "ds1", str2.c_str(), str2.size());

  std::string str3("");
  cp_io.read_string_from_hdf5_file(checkpoint_file, my_rank, "control", "ds1", str3);

  printf("str 3 %s\n", str3.c_str());

  std::string str4("");
  cp_io.read_string_from_hdf5_file(checkpoint_file, my_rank, "topology", "ds1", str4);

  printf("str 4 %s\n", str4.c_str());
#endif  
  cp_io.close_hdf5_file(checkpoint_file, my_rank);

  
  return 0;
} // index

ftest_register_driver(index_topology);
