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

/*! @file */

#include <flecsi-config.h>

#include "flecsi/util/serialize.hh"

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

namespace flecsi {
namespace util {

/*!
 Wrapper to convert from C++ types to MPI types.

 @tparam TYPE The C++ P.O.D. type, e.g., double.

 @ingroup coloring
 */

template<typename TYPE>
struct mpi_typetraits {

  inline static MPI_Datatype type() {
    static MPI_Datatype data_type = MPI_DATATYPE_NULL;

    if(data_type == MPI_DATATYPE_NULL) {
      MPI_Type_contiguous(sizeof(TYPE), MPI_BYTE, &data_type);
      MPI_Type_commit(&data_type);
    }
    return data_type;
  }
};

template<>
struct mpi_typetraits<std::size_t> {
  inline static MPI_Datatype type() {
    if(sizeof(std::size_t) == 8) {
      return MPI_UNSIGNED_LONG_LONG;
    }
    else {
      return MPI_UNSIGNED;
    } // if
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<std::byte> {
  inline static MPI_Datatype type() {
    return MPI_BYTE;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<char> {
  inline static MPI_Datatype type() {
    return MPI_SIGNED_CHAR;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<unsigned char> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED_CHAR;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<short> {
  inline static MPI_Datatype type() {
    return MPI_SHORT;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<unsigned short> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED_SHORT;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<int> {
  inline static MPI_Datatype type() {
    return MPI_INT;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<unsigned> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<long> {
  inline static MPI_Datatype type() {
    return MPI_LONG;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<float> {
  inline static MPI_Datatype type() {
    return MPI_FLOAT;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<double> {
  inline static MPI_Datatype type() {
    return MPI_DOUBLE;
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<long double> {
  inline static MPI_Datatype type() {
    return MPI_LONG_DOUBLE;
  }
}; // mpi_typetraits

namespace mpi {

// Convenience variables
inline const auto size_type = mpi_typetraits<std::size_t>::type();
inline const auto byte_type = mpi_typetraits<std::byte>::type();
inline const auto char_type = mpi_typetraits<char>::type();
inline const auto unsigned_char_type = mpi_typetraits<unsigned char>::type();
inline const auto short_type = mpi_typetraits<short>::type();
inline const auto unsigned_short_type = mpi_typetraits<unsigned short>::type();
inline const auto int_type = mpi_typetraits<int>::type();
inline const auto unsigend_type = mpi_typetraits<unsigned>::type();
inline const auto long_type = mpi_typetraits<long>::type();
inline const auto float_type = mpi_typetraits<float>::type();
inline const auto double_type = mpi_typetraits<double>::type();
inline const auto long_double_type = mpi_typetraits<long double>::type();

/*!
  Convenience function to get basic MPI communicator information.
 */

inline auto
info(MPI_Comm comm = MPI_COMM_WORLD) {
  int rank, size;
  MPI_Group group;

  MPI_Comm_size(comm, &size);
  MPI_Comm_group(comm, &group);
  MPI_Group_rank(group, &rank);

  return std::make_tuple(rank, size, group);
} // info

/*!
  One-to-All (variable) communication pattern.

  This function uses the FleCSI serialization interface with a packing functor
  to communicate data from the root rank (0) to all other ranks.

  @tparam F The packing functor type, which must define a \em return_type,
            and the \emph () operator, taking \em rank and \em size as integer
            arguments. The \em return_type is the type return by the packing
            functor.

  @param f    An instance of the packing functor.
  @param comm An MPI communicator.

  @return For all ranks besides the root rank (0), the communicated data. For
          the root rank (0), the functor applied for the root rank (0) and size.
 */

template<typename F>
inline auto
one_to_allv(F const & f, MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size, group] = info(comm);

  std::vector<MPI_Request> requests;

  if(rank == 0) {
    for(size_t r{1}; r < std::size_t(size); ++r) {
      std::vector<std::byte> data(serial_put(f(r, size)));
      const std::size_t bytes = data.size();

      requests.resize(requests.size() + 1);
      MPI_Isend(&bytes, 1, mpi::size_type, r, 0, comm, &requests.back());
      requests.resize(requests.size() + 1);
      MPI_Isend(
        data.data(), bytes, mpi::byte_type, r, 0, comm, &requests.back());
    } // for

    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), requests.data(), status.data());
  }
  else {
    std::size_t bytes{0};
    MPI_Status status;
    MPI_Recv(&bytes, 1, mpi::size_type, 0, 0, comm, &status);
    std::vector<std::byte> data(bytes);
    MPI_Recv(data.data(), bytes, mpi::byte_type, 0, 0, comm, &status);
    auto const * p = data.data();
    return serial_get<typename F::return_type>(p);
  } // if

  return f(0, size);
} // one_to_allv

/*!
  All-to-All (variable) communication pattern implemented with non-blocking
  send and receive operations.

  This function uses the FleCSI serialization interface with a packing functor
  to communicate data from all ranks to all other ranks.

  @tparam F The packing functor type, which must define a \em return_type,
            a \em count() method, taking the calling rank as an argument,
            and the \emph () operator, taking \em rank and \em size as integer
            arguments. The \em return_type is the type return by the packing
            functor.

  @param f    An instance of the packing functor.
  @param comm An MPI communicator.

  @return A std::vector<return_type>.
 */

template<typename F>
inline auto
all_to_allv(F const & f, MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size, group] = info(comm);

  std::vector<std::size_t> counts;
  counts.reserve(size);
  std::vector<std::size_t> bytes(size);

  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    counts.emplace_back(f.count(r));
  } // for

  MPI_Alltoall(
    counts.data(), 1, mpi::size_type, bytes.data(), 1, mpi::size_type, comm);

  std::vector<MPI_Request> requests;

  std::vector<std::vector<std::byte>> send_bufs(size);
  std::vector<std::vector<std::byte>> recv_bufs(size);

  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    if(bytes[r] > 0) {
      recv_bufs[r].resize(bytes[r]);
      requests.resize(requests.size() + 1);
      MPI_Irecv(recv_bufs[r].data(),
        recv_bufs[r].size(),
        mpi::byte_type,
        r,
        0,
        comm,
        &requests.back());
    } // if
  } // for

  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    if(counts[r] > 0) {
      requests.resize(requests.size() + 1);
      send_bufs[r] = serial_put(f(r, size));
      MPI_Isend(send_bufs[r].data(),
        send_bufs[r].size(),
        mpi::byte_type,
        r,
        0,
        comm,
        &requests.back());
    } // if
  } // for

  std::vector<MPI_Status> status(requests.size());
  MPI_Waitall(requests.size(), requests.data(), status.data());

  std::vector<typename F::return_type> result;
  result.reserve(size);
  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    if(recv_bufs[r].size() > 0) {
      auto const * p = recv_bufs[r].data();
      result.emplace_back(serial_get<typename F::return_type>(p));
    }
  } // for

  return result;
} // all_to_allv

} // namespace mpi

} // namespace util
} // namespace flecsi
