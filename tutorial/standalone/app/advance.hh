/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/control.hh"
#include "tasks/advance_mpi.hh"

using namespace flecsi;

namespace standalone {
namespace action {

int
advance() {
  if(control::state().step() % 10 == 0) {
    flog(warn) << __func__ << " step: " << control::state().step() << std::endl;
  } // if
  log::flush();

  execute<task::advance_mpi, mpi>(mpi_state(process_topology));

  return 0;
}
control::action<advance, cp::advance> advance_action;

} // namespace action
} // namespace standalone
