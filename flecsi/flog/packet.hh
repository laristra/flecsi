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
#pragma once

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
#include <array>
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

// Microsecond interval
#ifndef FLOG_PACKET_FLUSH_INTERVAL
#define FLOG_PACKET_FLUSH_INTERVAL 100000
#endif

namespace flog {

/*!
  Packet type for serializing output from distributed-memory tasks.
 */

struct packet_t {
  static constexpr size_t sec_bytes = sizeof(time_t);
  static constexpr size_t usec_bytes = sizeof(suseconds_t);
  static constexpr size_t packet_bytes =
    sec_bytes + usec_bytes + FLOG_MAX_MESSAGE_SIZE;

  packet_t(const char * msg = nullptr) {
    timeval stamp;
    if(gettimeofday(&stamp, NULL)) {
      std::cerr << "FLOG: call to gettimeofday failed!!! " << __FILE__
                << __LINE__ << std::endl;
      std::exit(1);
    } // if

    strncpy(
      bytes_.data(), reinterpret_cast<const char *>(&stamp.tv_sec), sec_bytes);

    strncpy(bytes_.data() + sec_bytes,
      reinterpret_cast<const char *>(&stamp.tv_usec),
      usec_bytes);

    std::ostringstream oss;
    oss << msg;

    strcpy(bytes_.data() + sec_bytes + usec_bytes, oss.str().c_str());
  } // packet_t

  time_t const & seconds() const {
    return *reinterpret_cast<time_t const *>(bytes_.data());
  } // seconds

  suseconds_t const & useconds() const {
    return *reinterpret_cast<suseconds_t const *>(bytes_.data() + sec_bytes);
  } // seconds

  const char * message() {
    return bytes_.data() + sec_bytes + usec_bytes;
  } // message

  const char * data() const {
    return bytes_.data();
  } // data

  static constexpr size_t bytes() {
    return sec_bytes + usec_bytes + FLOG_MAX_MESSAGE_SIZE;
  } // bytes

  bool operator<(packet_t const & b) {
    return this->seconds() == b.seconds() ? this->useconds() < b.useconds()
                                          : this->seconds() < b.seconds();
  } // operator <

private:
  std::array<char, packet_bytes> bytes_;

}; // packet_t

} // namespace flog

#endif // FLECSI_ENABLE_FLOG
