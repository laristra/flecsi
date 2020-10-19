/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/control.hh"
#include "tasks/finish_mpi.hh"

namespace standalone {
namespace action {

int
finalize() {

  execute<task::finish_mpi, mpi>(mpi_state(process_topology));

  return 0;
}
control::action<finalize, cp::finalize> finalize_action;

} // namespace action
} // namespace standalone
