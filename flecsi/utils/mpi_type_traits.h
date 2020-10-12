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

#include <complex>
#include <cstddef> // byte
#include <cstdint>
#include <type_traits>

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
  // NB: OpenMPI's predefined handles are not constant expressions.
  template<bool Make = true>
  static MPI_Datatype type() {
    using namespace std;
    static_assert(is_same_v<TYPE, remove_cv_t<remove_reference_t<TYPE>>>);
    // List is from MPI 3.2 draft.
    // TIP: Specializations would collide for, say, int32_t==int.
    // The constexpr-if tree saves on template instantiations.
    if constexpr(is_arithmetic_v<TYPE>) {
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
      else
        return make<Make>();
    }
    else if constexpr(is_same_v<TYPE, complex<float>>)
      return MPI_CXX_FLOAT_COMPLEX;
    else if constexpr(is_same_v<TYPE, complex<double>>)
      return MPI_CXX_DOUBLE_COMPLEX;
    else if constexpr(is_same_v<TYPE, complex<long double>>)
      return MPI_CXX_LONG_DOUBLE_COMPLEX;

    // MSVC is confused without the explicit std:: qualification
    else if constexpr(is_same_v<TYPE, std::byte>)
      return MPI_BYTE;
    else
      return make<Make>();
  }

private:
  template<bool Make>
  static MPI_Datatype make() {
    static_assert(Make, "type not predefined");
    // TODO: destroy at MPI_Finalize
    static const MPI_Datatype ret = [] {
      MPI_Datatype data_type;
      MPI_Type_contiguous(sizeof(TYPE), MPI_BYTE, &data_type);
      MPI_Type_commit(&data_type);
      std::cerr << "type not predefined" << std::endl;
      MPI_Abort(MPI_COMM_WORLD, -1);
      return data_type;
    }();
    return ret;
  }
};

template<class T>
MPI_Datatype
mpi_type() {
  return mpi_typetraits_u<T>::type();
}
// Use this to restrict to predefined types before MPI_Init:
template<class T>
MPI_Datatype
mpi_static_type() {
  return mpi_typetraits_u<T>::template type<false>();
}

} // namespace utils
} // namespace flecsi
