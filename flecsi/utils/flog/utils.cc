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

  std::cerr << "Executing send_to_one" << std::endl;

  if(flog_t::instance().initialized()) {

    int * sizes = flog_t::instance().process() == 0
                    ? new int[flog_t::instance().processes()]
                    : nullptr;

    binary_serializer_t serializer;
    serializer << flog_t::instance().packets();

    int bytes = serializer.bytes();

    MPI_Gather(&bytes, 1, MPI_INT, sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(flog_t::instance().process() == 0) {
      for(size_t i{0}; i<flog_t::instance().processes(); ++i) {
        std::cout << "bytes: " << sizes[i] << std::endl;
      } // for
    } // if

    // binary_serializer_t serializer;
    // serializer << flog_t::instance().

    //MPI_Gatherv();
  } // if

} // send_to_one

#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
