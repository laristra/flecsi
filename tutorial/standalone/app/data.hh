/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/types.hh"

namespace standalone {

const single<std::size_t>::definition<flecsi::topo::global> refinement_max;

struct mpi_data {
  double values[10];
};

const single<mpi_data *>::definition<flecsi::topo::index> mpi_state;

} // namespace standalone
