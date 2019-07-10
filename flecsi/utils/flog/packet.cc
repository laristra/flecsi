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
#include "packet.hh"
#include "state.hh"
#include "types.hh"

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace utils {
namespace flog {

#if defined(FLOG_ENABLE_MPI)
void
flush_packets() {
  while(flog_t::instance().run_flusher()) {
    usleep(FLOG_PACKET_FLUSH_INTERVAL);

    {
      std::lock_guard<std::mutex> guard(flog_t::instance().packets_mutex());

      if(flog_t::instance().packets().size()) {
        std::sort(flog_t::instance().packets().begin(),
          flog_t::instance().packets().end());

        for(auto & p : flog_t::instance().packets()) {
          flog_t::instance().stream() << p.message();
        } // for

        flog_t::instance().packets().clear();
      } // if
    } // scope

  } // while
} // flush_packets
#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
