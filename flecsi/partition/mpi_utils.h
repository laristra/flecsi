/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mpi_utils_h
#define flecsi_mpi_utils_h

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

///
/// \file
/// \date Initial file creation: Nov 23, 2016
///

namespace flecsi {
namespace dmp {

template<typename T> struct mpi_typetraits {};

template<>
struct mpi_typetraits<idx_t>
{
  inline static
  MPI_Datatype
  type()
  {
    if(sizeof(idx_t) == 8) {
      return MPI_UNSIGNED_LONG_LONG;
    }
    else {
      return MPI_UNSIGNED;
    } // if
  }
}; // mpi_typetraits

template<>
struct mpi_typetraits<size_t>
{
  inline static
  MPI_Datatype
  type()
  {
    if(sizeof(size_t) == 8) {
      return MPI_UNSIGNED_LONG_LONG;
    }
    else {
      return MPI_UNSIGNED;
    } // if
  }
}; // mpi_typetraits

} // namespace dmp
} // namespace flecsi

#endif // flecsi_mpi_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
