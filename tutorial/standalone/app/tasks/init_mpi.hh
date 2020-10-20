/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include <flecsi/execution.hh>

#include "../data.hh"

namespace standalone {
namespace task {

using namespace flecsi;

int
init_mpi(single<mpi_data *>::accessor<wo> data) {
  flog(info) << "newing data on process " << process() << std::endl;

  data = new mpi_data;

  for(std::size_t i{0}; i < 10; ++i) {
    data->values[i] = i + process() * 10;
  } // for

  return 0;
} // init_mpi

} // namespace task
} // namespace standalone
