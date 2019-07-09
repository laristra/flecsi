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

#if 0
  if(flog_t::instance().initialized()) {

    binary_serializer_t serializer;
    //serializer << flog_t::instance().

    int * sizes = flog_t::instance().rank() == 0 ?
      new int[flog_t::instance().size()] : nullptr;

    MPI_Gather();
    MPI_Gatherv();
  } // if
#endif

} // send_to_one

#endif // FLOG_ENABLE_MPI

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
