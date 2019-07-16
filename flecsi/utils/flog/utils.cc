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

#include <flecsi/utils/flog/packet.hh>
#include <flecsi/utils/flog/state.hh>
#include <flecsi/utils/flog/utils.hh>

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace utils {
namespace flog {

#if defined(FLOG_ENABLE_MPI)

void
send_to_one() {

  if(flog_t::instance().initialized()) {
    std::lock_guard<std::mutex> guard(flog_t::instance().packets_mutex());

    int * sizes = flog_t::instance().process() == 0
                    ? new int[flog_t::instance().processes()]
                    : nullptr;

    binary_serializer_t serializer;
    serializer << flog_t::instance().packets();
    serializer.flush();

    int bytes = serializer.bytes();

    MPI_Gather(&bytes, 1, MPI_INT, sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int * offsets = nullptr;
    int sum{0};

    if(flog_t::instance().process() == 0) {
      offsets = new int[flog_t::instance().processes()];

      for(size_t p{0}; p < flog_t::instance().processes(); ++p) {
        offsets[p] = sum;
        sum += sizes[p];
      } // for
    } // if

    char * buffer = flog_t::instance().process() == 0 ? new char[sum] : nullptr;

    MPI_Gatherv(serializer.data(),
      bytes,
      MPI_CHAR,
      buffer,
      sizes,
      offsets,
      MPI_CHAR,
      0,
      MPI_COMM_WORLD);

    if(flog_t::instance().process() == 0) {

      flog_t::instance().packets().clear();

      for(size_t p{0}; p < flog_t::instance().processes(); ++p) {

        if(flog_t::instance().one_process()) {
          if(p == flog_t::instance().output_process()) {
            binary_deserializer_t deserializer(&buffer[offsets[p]], sizes[p]);
            std::vector<packet_t> remote_packets;
            deserializer >> remote_packets;

            std::vector<packet_t> & packets = flog_t::instance().packets();

            packets.reserve(packets.size() + remote_packets.size());
            packets.insert(
              packets.end(), remote_packets.begin(), remote_packets.end());
          } // if
        }
        else {
          binary_deserializer_t deserializer(&buffer[offsets[p]], sizes[p]);
          std::vector<packet_t> remote_packets;
          deserializer >> remote_packets;

          std::vector<packet_t> & packets = flog_t::instance().packets();

          packets.reserve(packets.size() + remote_packets.size());
          packets.insert(
            packets.end(), remote_packets.begin(), remote_packets.end());
        } // if
      } // for

      delete[] sizes;
      delete[] offsets;
      delete[] buffer;
    }
    else {
      flog_t::instance().packets().clear();
    } // if

    flog_t::instance().set_serialized();
  } // if

} // send_to_one

#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
