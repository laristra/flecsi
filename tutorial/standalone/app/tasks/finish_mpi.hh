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
finish_mpi(single<mpi_data *>::accessor<wo> data) {

  flog(info) << "deleting data on process " << process() << std::endl;
  delete data.get();

  return 0;
} // finish_mpi

} // namespace task
} // namespace standalone
