/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#include "packet.h"
#include "types.h"

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace utils {
namespace flog {

#if defined(FLOG_ENABLE_MPI)
void
flush_packets() {
  while(mpi_state_t::instance().run_flusher()) {
    usleep(FLOG_PACKET_FLUSH_INTERVAL);

    {
      std::lock_guard<std::mutex> guard(
        mpi_state_t::instance().packets_mutex());

      if(mpi_state_t::instance().packets().size()) {
        std::sort(mpi_state_t::instance().packets().begin(),
          mpi_state_t::instance().packets().end());

        for(auto & p : mpi_state_t::instance().packets()) {
          flog_t::instance().stream() << p.message();
        } // for

        mpi_state_t::instance().packets().clear();
      } // if
    } // scope

  } // while
} // flush_packets
#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
