/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_processor_h
#define flecsi_execution_common_processor_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 02, 2016
//----------------------------------------------------------------------------//

#include <bitset>
#include <iostream>

#include "flecsi/utils/debruijn.h"

namespace flecsi {

//----------------------------------------------------------------------------//
//! Enumeration of task processor types. Not all of these may be supported
//! by all runtimes. Unsupported processor information will be ignored.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

enum processor_type_t : size_t {
  loc,
  toc,
  mpi
}; // enum processor_type_t

//----------------------------------------------------------------------------//
//! Convenience method to print processor_type_t instances.
//----------------------------------------------------------------------------//

inline
std::ostream &
operator << (
  std::ostream & stream,
  const processor_type_t & variant
)
{
  switch(variant) {
    case processor_type_t::loc:
      stream << "loc";
      break;
    case processor_type_t::toc:
      stream << "toc";
      break;
    case processor_type_t::mpi:
      stream << "mpi";
      break;
  } // switch

  return stream;
} // operator <<

} // namespace flecsi

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
