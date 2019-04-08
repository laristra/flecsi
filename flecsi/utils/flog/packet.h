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

#if defined(FLECSI_ENABLE_FLOG)

#if defined(FLOG_ENABLE_MPI)
#include <mpi.h>
#endif

#if defined(_MSC_VER)
#error "Need implementation for Windows"
#endif

#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#ifndef FLOG_MAX_MESSAGE_SIZE
#define FLOG_MAX_MESSAGE_SIZE 1024
#endif

#ifndef FLOG_MAX_PACKET_BUFFER
#define FLOG_MAX_PACKET_BUFFER 1024
#endif

#ifndef FLOG_PACKET_FLUSH_INTERVAL
#define FLOG_PACKET_FLUSH_INTERVAL 100000
#endif

namespace flecsi {
namespace utils {
namespace flog {

/*!
  Packet type for serializing output from distributed-memory tasks.
 */

struct packet_t {
  static constexpr size_t sec_bytes = sizeof(time_t);
  static constexpr size_t usec_bytes = sizeof(suseconds_t);

  packet_t(const char * msg = nullptr) {
    timeval stamp;
    if(gettimeofday(&stamp, NULL)) {
      std::cerr << "FLOG: call to gettimeofday failed!!! " << __FILE__
                << __LINE__ << std::endl;
      std::exit(1);
    } // if

    strncpy(data_, reinterpret_cast<const char *>(&stamp.tv_sec), sec_bytes);
    strncpy(data_ + sec_bytes,
      reinterpret_cast<const char *>(&stamp.tv_usec),
      usec_bytes);

    std::ostringstream oss;
    oss << msg;

    strcpy(data_ + sec_bytes + usec_bytes, oss.str().c_str());
  } // packet_t

  time_t const & seconds() const {
    return *reinterpret_cast<time_t const *>(data_);
  } // seconds

  suseconds_t const & useconds() const {
    return *reinterpret_cast<suseconds_t const *>(data_ + sec_bytes);
  } // seconds

  const char * message() {
    return data_ + sec_bytes + usec_bytes;
  } // message

  const char * data() const {
    return data_;
  } // data

  size_t bytes() const {
    return sec_bytes + usec_bytes + FLOG_MAX_MESSAGE_SIZE;
  } // bytes

  bool operator<(packet_t const & b) {
    return this->seconds() == b.seconds() ? this->useconds() < b.useconds()
                                          : this->seconds() < b.seconds();
  } // operator <

private:
  char data_[sec_bytes + usec_bytes + FLOG_MAX_MESSAGE_SIZE];

}; // packet_t

// Forward
void flush_packets();

#if defined(FLOG_ENABLE_MPI)
struct mpi_state_t {
  static mpi_state_t & instance() {
    static mpi_state_t s;
    return s;
  } // instance

  void initialize() {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
    MPI_Comm_size(MPI_COMM_WORLD, &size_);

    std::thread flusher(flush_packets);
    instance().flusher_thread().swap(flusher);

    initialized_ = true;
  } // initialize

  void finalize() {
    if(initialized_) {
      end_flusher();
      flusher_thread_.join();
    } // if
  } // finalize

  bool initialized() {
    return initialized_;
  }

  int rank() {
    return rank_;
  }
  int size() {
    return size_;
  }

  std::thread & flusher_thread() {
    return flusher_thread_;
  }
  std::mutex & packets_mutex() {
    return packets_mutex_;
  }
  std::vector<packet_t> & packets() {
    return packets_;
  }

  bool run_flusher() {
    return run_flusher_;
  }
  void end_flusher() {
    run_flusher_ = false;
  }

private:
  int rank_;
  int size_;
  std::thread flusher_thread_;
  std::mutex packets_mutex_;
  std::vector<packet_t> packets_;
  bool run_flusher_ = true;
  bool initialized_ = false;

}; // mpi_state_t
#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
