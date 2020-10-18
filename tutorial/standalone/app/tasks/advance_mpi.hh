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

void
advance_mpi(single<mpi_data *>::accessor<rw> data) {
  mpi_data * d = data.get();

  std::stringstream ss;
  ss << "mpi data: ";
  for(std::size_t i{0}; i < 10; ++i) {
    d->values[i] += 1;
    ss << d->values[i] << " ";
  } // for
  flog(info) << ss.str() << std::endl;
} // advance_mpi

} // namespace task
} // namespace standalone
