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
init(single<std::size_t>::accessor<wo> refinement_max) {
  flog(info) << "initializing max" << std::endl;

  refinement_max = 10;

  return 0;
} // init_mpi

} // namespace task
} // namespace standalone
