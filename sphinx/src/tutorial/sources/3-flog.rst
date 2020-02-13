.. code-block:: cpp

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
  #include <flecsi/execution.hh>
  #include <flecsi/utils/flog.hh>
  int top_level_action(int, char **) {
    flog(info) << "Hello World" << std::endl
    return 0
  } // top_level_action
  inline bool top_level_action_registered =
    flecsi::register_top_level_action(top_level_action)
  int main(int argc, char ** argv) {
    auto status = flecsi::initialize(argc, argv)
    flecsi::utils::flog::flog_t::instance().config_stream().add_buffer(
      "flog", std::clog, true)
    if(status != flecsi::runtime::status::success) {
      return status == flecsi::runtime::status::help ? 0 : status
    } // if
    status = flecsi::start()
    flecsi::finalize()
    return status
  } // main
