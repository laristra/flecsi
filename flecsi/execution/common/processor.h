/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_processor_h
#define flecsi_execution_common_processor_h

///
// \file processor.h
// \authors bergen
// \date Initial file creation: Aug 02, 2016
///

namespace flecsi {
namespace execution {

// FIXME: Finish Doxygen

///
// The processor_t enum defines the different processor types that
// are supported by FleCSI's execution model. Some processor types
// are invalid with a particular backend.
//
// \note The number of processor types is currently limited to 32! This is
//       due to the hashing strategy being employed by some of the backends.
//       If it becomes necessary to support more processor types, the hashing
//       strategy will need to be changed to accommodate more types.
///
enum processor_t : size_t {
  loc,
  toc,
  mpi
}; // enum processor_t

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
