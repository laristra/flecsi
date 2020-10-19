/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include "advance.hh"
#include "analyze.hh"
#include "data.hh"
#include "finalize.hh"
#include "initialize.hh"
#include "specialization/control.hh"

#include <flecsi/execution.hh>
#include <flecsi/flog.hh>

using namespace standalone;

int
main(int argc, char ** argv) {
  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  }

  status = control::check_options();

  if(status != flecsi::run::status::success) {
    flecsi::finalize();
    return status == flecsi::run::status::option ? 0 : status;
  } // if

  flecsi::log::add_output_stream("clog", std::clog, true);

  status = flecsi::start(control::execute);

  flecsi::finalize();

  return status;
} // main
