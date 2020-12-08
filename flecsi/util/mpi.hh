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

#include <complex>
#include <cstddef> // byte
#include <cstdint>
#include <type_traits>

#include <mpi.h>

namespace flecsi {
namespace util {
namespace mpi {

// NB: OpenMPI's predefined handles are not constant expressions.
template<class TYPE>
auto
maybe_static() {
  using namespace std;
  static_assert(is_same_v<TYPE, remove_cv_t<remove_reference_t<TYPE>>>);
  // List is from MPI 3.2 draft.
  // TIP: Specializations would collide for, say, int32_t==int.
  if constexpr(is_arithmetic_v<TYPE>) { // save on template instantiations
    if constexpr(is_same_v<TYPE, char>)
      return MPI_CHAR;
    else if constexpr(is_same_v<TYPE, short>)
      return MPI_SHORT;
    else if constexpr(is_same_v<TYPE, int>)
      return MPI_INT;
    else if constexpr(is_same_v<TYPE, long>)
      return MPI_LONG;
    else if constexpr(is_same_v<TYPE, long long>)
      return MPI_LONG_LONG;
    else if constexpr(is_same_v<TYPE, signed char>)
      return MPI_SIGNED_CHAR;
    else if constexpr(is_same_v<TYPE, unsigned char>)
      return MPI_UNSIGNED_CHAR;
    else if constexpr(is_same_v<TYPE, unsigned short>)
      return MPI_UNSIGNED_SHORT;
    else if constexpr(is_same_v<TYPE, unsigned>)
      return MPI_UNSIGNED;
    else if constexpr(is_same_v<TYPE, unsigned long>)
      return MPI_UNSIGNED_LONG;
    else if constexpr(is_same_v<TYPE, unsigned long long>)
      return MPI_UNSIGNED_LONG_LONG;
    else if constexpr(is_same_v<TYPE, float>)
      return MPI_FLOAT;
    else if constexpr(is_same_v<TYPE, double>)
      return MPI_DOUBLE;
    else if constexpr(is_same_v<TYPE, long double>)
      return MPI_LONG_DOUBLE;
    else if constexpr(is_same_v<TYPE, wchar_t>)
      return MPI_WCHAR;
#ifdef INT8_MIN
    else if constexpr(is_same_v<TYPE, int8_t>)
      return MPI_INT8_T;
    else if constexpr(is_same_v<TYPE, uint8_t>)
      return MPI_UINT8_T;
#endif
#ifdef INT16_MIN
    else if constexpr(is_same_v<TYPE, int16_t>)
      return MPI_INT16_T;
    else if constexpr(is_same_v<TYPE, uint16_t>)
      return MPI_UINT16_T;
#endif
#ifdef INT32_MIN
    else if constexpr(is_same_v<TYPE, int32_t>)
      return MPI_INT32_T;
    else if constexpr(is_same_v<TYPE, uint32_t>)
      return MPI_UINT32_T;
#endif
#ifdef INT64_MIN
    else if constexpr(is_same_v<TYPE, int64_t>)
      return MPI_INT64_T;
    else if constexpr(is_same_v<TYPE, uint64_t>)
      return MPI_UINT64_T;
#endif
    else if constexpr(is_same_v<TYPE, MPI_Aint>)
      return MPI_AINT;
    else if constexpr(is_same_v<TYPE, MPI_Offset>)
      return MPI_OFFSET;
    else if constexpr(is_same_v<TYPE, MPI_Count>)
      return MPI_COUNT;
    else if constexpr(is_same_v<TYPE, bool>)
      return MPI_CXX_BOOL;
  }
  else if constexpr(is_same_v<TYPE, complex<float>>)
    return MPI_CXX_FLOAT_COMPLEX;
  else if constexpr(is_same_v<TYPE, complex<double>>)
    return MPI_CXX_DOUBLE_COMPLEX;
  else if constexpr(is_same_v<TYPE, complex<long double>>)
    return MPI_CXX_LONG_DOUBLE_COMPLEX;
  else if constexpr(is_same_v<TYPE, byte>)
    return MPI_BYTE;
  // else: void
}

template<class T>
MPI_Datatype
type() {
  if constexpr(!std::is_void_v<decltype(maybe_static<T>())>)
    return maybe_static<T>();
  else {
    // Unfortunately, std::tuple<int> is not trivially copyable:
    static_assert(std::is_trivially_copy_assignable_v<T> ||
                  std::is_copy_assignable_v<T> &&
                    std::is_trivially_copy_constructible_v<T>);
    // TODO: destroy at MPI_Finalize
    static const MPI_Datatype ret = [] {
      MPI_Datatype data_type;
      MPI_Type_contiguous(sizeof(T), MPI_BYTE, &data_type);
      MPI_Type_commit(&data_type);
      return data_type;
    }();
    return ret;
  }
}
// Use this to restrict to predefined types before MPI_Init:
template<class T,
  class = std::enable_if_t<!std::is_void_v<decltype(maybe_static<T>())>>>
MPI_Datatype
static_type() {
  return maybe_static<T>();
}

/*!
  Convenience function to get basic MPI communicator information.
 */

inline auto
info(MPI_Comm comm = MPI_COMM_WORLD) {
  int rank, size;
  MPI_Comm_size(comm, &size);
  MPI_Comm_rank(comm, &rank);
  return std::make_pair(rank, size);
} // info

/*!
  One-to-All (variable) communication pattern.

  This function uses the FleCSI serialization interface with a packing
  callable object to communicate data from the root rank (0) to all
  other ranks.

  @tparam F The packing functor type with signature \em (rank, size).

  @param f    A callable object.
  @param comm An MPI communicator.

  @return For all ranks besides the root rank (0), the communicated data.
          For the root rank (0), the callable object applied for the root
          rank (0) and size.
 */

template<typename F>
inline auto
one_to_allv(F const & f, MPI_Comm comm = MPI_COMM_WORLD) {
  using return_type = decltype(f(int(0), int(1)));

  auto [rank, size] = info(comm);

  std::vector<std::vector<std::byte>> data(size);

  if(rank == 0) {
    std::vector<MPI_Request> requests;
    requests.reserve(2 * (size - 1));

    for(size_t r{1}; r < std::size_t(size); ++r) {
      data[r] = serial_put(f(r, size));
      const std::size_t bytes = data[r].size();

      requests.resize(requests.size() + 1);
      MPI_Isend(&bytes, 1, type<std::size_t>(), r, 0, comm, &requests.back());
      requests.resize(requests.size() + 1);
      MPI_Isend(
        data[r].data(), bytes, type<std::byte>(), r, 0, comm, &requests.back());
    } // for

    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), requests.data(), status.data());
  }
  else {
    std::size_t bytes{0};
    MPI_Status status;
    MPI_Recv(&bytes, 1, type<std::size_t>(), 0, 0, comm, &status);
    std::vector<std::byte> data(bytes);
    MPI_Recv(data.data(), bytes, type<std::byte>(), 0, 0, comm, &status);
    auto const * p = data.data();
    return serial_get<return_type>(p);
  } // if

  return f(0, size);
} // one_to_allv

/*!
  All-to-All (variable) communication pattern implemented with non-blocking
  send and receive operations.

  This function uses the FleCSI serialization interface with a packing
  callable object to communicate data from all ranks to all other ranks.

  @tparam F The packing type with signature \em (rank, size).

  @param f    A callable object.
  @param comm An MPI communicator.

  @return A std::vector<return_type>, where \rm return_type is the type
          returned by the callable object.
 */

template<typename F>
inline auto
all_to_allv(F const & f, MPI_Comm comm = MPI_COMM_WORLD) {
  using return_type = decltype(f(int(0), int(1)));

  auto [rank, size] = info(comm);

  std::vector<std::size_t> counts;
  counts.reserve(size);
  std::vector<std::size_t> bytes(size);

  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    counts.emplace_back(serial_size<return_type>(f(r, size)));
  } // for

  MPI_Alltoall(counts.data(),
    1,
    type<std::size_t>(),
    bytes.data(),
    1,
    type<std::size_t>(),
    comm);

  std::vector<std::vector<std::byte>> recv_bufs(size);

  {
    std::vector<MPI_Request> requests;
    requests.reserve(2 * size);
    std::vector<std::vector<std::byte>> send_bufs(size);

    for(std::size_t r{0}; r < std::size_t(size); ++r) {
      if(bytes[r] > 0) {
        recv_bufs[r].resize(bytes[r]);
        requests.resize(requests.size() + 1);
        MPI_Irecv(recv_bufs[r].data(),
          recv_bufs[r].size(),
          type<std::byte>(),
          r,
          0,
          comm,
          &requests.back());
      } // if
    } // for

    bytes.clear();

    for(std::size_t r{0}; r < std::size_t(size); ++r) {
      if(counts[r] > 0) {
        requests.resize(requests.size() + 1);
        send_bufs[r] = serial_put(f(r, size));
        MPI_Isend(send_bufs[r].data(),
          send_bufs[r].size(),
          type<std::byte>(),
          r,
          0,
          comm,
          &requests.back());
      } // if
    } // for

    counts.clear();

    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), requests.data(), status.data());
  } // scope

  std::vector<return_type> result;
  result.reserve(size);
  for(std::size_t r{0}; r < std::size_t(size); ++r) {
    if(recv_bufs[r].size() > 0) {
      auto const * p = recv_bufs[r].data();
      result.emplace_back(serial_get<return_type>(p));
    }
  } // for

  return result;
} // all_to_allv

} // namespace mpi

} // namespace util
} // namespace flecsi
