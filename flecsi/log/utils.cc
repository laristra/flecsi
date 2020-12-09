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

#include "flecsi/log/utils.hh"
#include "flecsi/log/packet.hh"
#include "flecsi/log/state.hh"
#include "flecsi/util/serialize.hh"

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace log {

#if defined(FLOG_ENABLE_MPI)

std::size_t
log_size() {
  return state::instance().packets().size();
} // log_size

void
send_to_one() {

  if(state::instance().initialized()) {
    std::lock_guard<std::mutex> guard(state::instance().packets_mutex());

    int * sizes = state::instance().process() == 0
                    ? new int[state::instance().processes()]
                    : nullptr;

    std::vector<std::byte> data = util::serial_put(state::instance().packets()),
                           buffer;

    const int bytes = data.size();

    MPI_Gather(&bytes, 1, MPI_INT, sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int * offsets = nullptr;
    int sum{0};

    if(state::instance().process() == 0) {
      offsets = new int[state::instance().processes()];

      for(size_t p{0}; p < state::instance().processes(); ++p) {
        offsets[p] = sum;
        sum += sizes[p];
      } // for

      buffer.resize(sum);
    } // if

    MPI_Gatherv(data.data(),
      bytes,
      MPI_CHAR,
      buffer.data(),
      sizes,
      offsets,
      MPI_CHAR,
      0,
      MPI_COMM_WORLD);

    state::instance().packets().clear();

    if(state::instance().process() == 0) {

      for(size_t p{0}; p < state::instance().processes(); ++p) {

        if(!state::instance().one_process() ||
           p == state::instance().output_process()) {
          auto remote_packets = util::serial_get1<std::vector<packet_t>>(
            buffer.data() + offsets[p]);

          std::vector<packet_t> & packets = state::instance().packets();

          packets.reserve(packets.size() + remote_packets.size());
          packets.insert(
            packets.end(), remote_packets.begin(), remote_packets.end());
        } // if
      } // for

      delete[] sizes;
      delete[] offsets;
    } // if

    state::instance().set_serialized();
  } // if

} // send_to_one

#endif // FLOG_ENABLE_MPI

} // namespace log
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
