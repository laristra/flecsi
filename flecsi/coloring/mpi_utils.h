/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_coloring_mpi_utils_h
#define flecsi_coloring_mpi_utils_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 23, 2016
//----------------------------------------------------------------------------//

#include <flecsi-config.h>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

namespace flecsi {
namespace coloring {

//----------------------------------------------------------------------------//
//! Wrapper to convert from C++ types to MPI types.
//!
//! @tparam TYPE The C++ P.O.D. type, e.g., double.
//!
//! @ingroup coloring
//----------------------------------------------------------------------------//

template<typename TYPE> struct mpi_typetraits__
{
   
   inline static
   MPI_Datatype
   type()
   {
     static MPI_Datatype data_type = MPI_DATATYPE_NULL;

     if (data_type == MPI_DATATYPE_NULL) {
         MPI_Type_contiguous(sizeof(TYPE), MPI_BYTE, &data_type);
         MPI_Type_commit(&data_type);
     }
     return data_type;
   }
};

template<>
struct mpi_typetraits__<size_t>
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
}; // mpi_typetraits__

template<>
struct mpi_typetraits__<int>
{
  inline static
  MPI_Datatype
  type()
  {
    return MPI_INT;
  }
}; // mpi_typetraits__

template<>
struct mpi_typetraits__<double>
{
  inline static
  MPI_Datatype
  type()
  {
    return MPI_DOUBLE;
  }
}; // mpi_typetraits__

} // namespace coloring
} // namespace flecsi

#endif // flecsi_coloring_mpi_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
