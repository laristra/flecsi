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

#include <flecsi-config.h>

#include "flecsi/log/packet.hh"
#include "flecsi/log/state.hh"
#include "flecsi/log/types.hh"

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace log {

#if defined(FLOG_ENABLE_MPI)
void
flush_packets() {
  while(state::instance().run_flusher()) {
    usleep(FLOG_PACKET_FLUSH_INTERVAL);
    std::lock_guard<std::mutex> guard(state::instance().packets_mutex());

    if(state::instance().serialized()) {
      if(state::instance().packets().size()) {
        std::sort(state::instance().packets().begin(),
          state::instance().packets().end());

        for(auto & p : state::instance().packets()) {
          state::instance().stream() << p.message();
        } // for

        state::instance().packets().clear();
      } // if
    } // if
  } // while
} // flush_packets
#endif // FLOG_ENABLE_MPI

} // namespace log
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
