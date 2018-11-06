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

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

namespace flecsi {
namespace utils {

/*!
 Wrapper to convert from C++ types to MPI types.

 @tparam TYPE The C++ P.O.D. type, e.g., double.

 @ingroup coloring
 */

template<typename TYPE>
struct mpi_typetraits_u {

  inline static MPI_Datatype type() {
    static MPI_Datatype data_type = MPI_DATATYPE_NULL;

    if (data_type == MPI_DATATYPE_NULL) {
      MPI_Type_contiguous(sizeof(TYPE), MPI_BYTE, &data_type);
      MPI_Type_commit(&data_type);
    }
    return data_type;
  }
};

template<>
struct mpi_typetraits_u<size_t> {
  inline static MPI_Datatype type() {
    if (sizeof(size_t) == 8) {
      return MPI_UNSIGNED_LONG_LONG;
    } else {
      return MPI_UNSIGNED;
    } // if
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<char> {
  inline static MPI_Datatype type() {
    return MPI_SIGNED_CHAR;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<unsigned char> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED_CHAR;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<short> {
  inline static MPI_Datatype type() {
    return MPI_SHORT;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<unsigned short> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED_SHORT;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<int> {
  inline static MPI_Datatype type() {
    return MPI_INT;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<unsigned> {
  inline static MPI_Datatype type() {
    return MPI_UNSIGNED;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<long> {
  inline static MPI_Datatype type() {
    return MPI_LONG;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<double> {
  inline static MPI_Datatype type() {
    return MPI_DOUBLE;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<float> {
  inline static MPI_Datatype type() {
    return MPI_FLOAT;
  }
}; // mpi_typetraits_u

template<>
struct mpi_typetraits_u<long double> {
  inline static MPI_Datatype type() {
    return MPI_LONG_DOUBLE;
  }
}; // mpi_typetraits_u

} // namespace utils
} // namespace flecsi
