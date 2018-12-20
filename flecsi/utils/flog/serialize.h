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
#pragma once

/*! @file */

#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_MPI)
  #include <mpi.h>
#endif

#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace flecsi {
namespace utils {
namespace flog {

// Forward
inline void flush_packets();

#if defined(FLECSI_ENABLE_MPI)
struct mpi_state_t
{
  static mpi_state_t & instance() {
    static mpi_state_t s;
    return s;
  } // instance

  void init() {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
    MPI_Comm_size(MPI_COMM_WORLD, &size_);

    std::thread flusher(flush_packets);
    instance().flusher_thread().swap(flusher);

    initialized_ = true;
  } // init

  bool initialized() { return initialized_; }

  int rank() { return rank_; }
  int size() { return size_; }

  std::thread & flusher_thread() { return flusher_thread_; }
  std::mutex & packets_mutex() { return packets_mutex_; }
  std::vector<packet_t> & packets() { return packets_; }

  bool run_flusher() { return run_flusher_; }
  void end_flusher() { run_flusher_ = false; }

private:

  ~mpi_state_t()
  {
    if(initialized_) {
      end_flusher();
      flusher_thread_.join();
    } // if
  }

  int rank_;
  int size_;
  std::thread flusher_thread_;
  std::mutex packets_mutex_;
  std::vector<packet_t> packets_;
  bool run_flusher_ = true;
  bool initialized_ = false;

}; // mpi_state_t

void flush_packets() {
  while(mpi_state_t::instance().run_flusher()) {
    usleep(FLOG_PACKET_FLUSH_INTERVAL);

    {
    std::lock_guard<std::mutex> guard(mpi_state_t::instance().packets_mutex());

    if(mpi_state_t::instance().packets().size()) {
      std::sort(mpi_state_t::instance().packets().begin(),
        mpi_state_t::instance().packets().end());

      for(auto & p: mpi_state_t::instance().packets()) {
        clog_t::instance().stream() << p.message();
      } // for

      mpi_state_t::instance().packets().clear();
    } // if
    } // scope

  } // while
} // flush_packets
#endif // FLECSI_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi
