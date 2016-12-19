/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_launch_h
#define flecsi_execution_common_launch_h

///
// \file launch.h
// \authors demeshko
// \date Initial file creation: Aug 23, 2016
///

namespace flecsi {
namespace execution {

// FIXME: Finish Doxygen

///
// The processor_t enum defines the different Legion task
// launch types that are supported by FleCSI's execution model.
///

enum launch_t : size_t {
  single,
  index,
  any
}; // enum launch_t


} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
