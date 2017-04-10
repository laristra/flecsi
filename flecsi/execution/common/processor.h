/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_processor_h
#define flecsi_execution_common_processor_h

///
/// \file
/// \date Initial file creation: Aug 02, 2016
///

#include <bitset>

#include "flecsi/utils/debruijn.h"

namespace flecsi {
namespace execution {

// This will be used by the task_hash_t type to create hash keys for
// task registration. If you add more processor types below, you will need
// to increase the processor_bits accordingly, i.e., processor_bits must
// be greater than or equal to the number of bits in the bitset for
// processor_t below.
constexpr size_t processor_bits = 4;

///
/// Use a std::bitset to store processor information
///
/// \note This will most likely use 4 bytes of data for efficiency. If
///       you need to add new processor types, just increase the number
///       of bits here and above.
///
using processor_t = std::bitset<4>;

///
/// Enumeration of various task processor types. Not all of these may be
/// supported by all runtimes. Unsupported processor information will be
/// ignored.
///
enum class processor_type_t : size_t {
  loc,
  toc,
  mpi
}; // enum processor_t

///
/// Bitmasks for processor types.
///
/// \note Currently, the maximum number of processor types is limited
///       to 32. We could increase this. However, it would require adding
///       a druijn64_t type or larger.
///
/// \note This enumeration is not scoped so that users can do things
///       like:
///       \code
///       processor_t p(loc | toc);
///       \endcode
///
enum processor_mask_t : size_t {
  loc = 1 << 0,
  toc = 1 << 1,
  mpi = 1 << 2
}; // enum processor_mask_t

///
/// Convert a processor mask to a processor type.
///
inline
processor_type_t
mask_to_type(
  processor_mask_t m
)
{
  return static_cast<processor_type_t>(utils::debruijn32_t::index(m));
} // mask_to_type

///
/// Macro to create repetitive interfaces
///
#define test_boolean_interface(name)                                           \
  inline bool processor_ ## name(const processor_t & p) {                      \
    return p.test(static_cast<size_t>(processor_type_t::name));                \
  }

test_boolean_interface(loc)
test_boolean_interface(toc)
test_boolean_interface(mpi)

#undef test_boolean_interface

///
/// Static processor_t creation function.
///
template<
  bool LOC = false,
  bool TOC = false,
  bool MPI = false
>
processor_t
make_processor()
{
  return {
    (LOC ? 1 << 0 : 0) |
    (TOC ? 1 << 1 : 0) |
    (MPI ? 1 << 2 : 0)
  };
} // make_processor

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
