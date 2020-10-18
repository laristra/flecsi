/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/control.hh"
#include "tasks/init_mpi.hh"

namespace standalone {
namespace action {

int
initialize() {

  control::state().steps() = 20;

  execute<task::init_mpi, mpi>(mpi_state(process_topology));

  return 0;
}
control::action<initialize, cp::initialize> initialize_action;

} // namespace action
} // namespace standalone
