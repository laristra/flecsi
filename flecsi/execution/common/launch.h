/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_common_launch_h
#define flecsi_execution_common_launch_h

///
/// \file
/// \date Initial file creation: Aug 23, 2016
///

namespace flecsi {
namespace execution {

///
/// The processor_t enum defines the different Legion task
/// launch types that are supported by FleCSI's execution model.
///

enum launch_t : size_t {
  single,
  index,
  any
}; // enum launch_t

} //namespace execution 
} // namespace flecsi

#define flecsi_bools_to_launch(S, I)                                           \
  S && I ?                                                                     \
    flecsi::execution::launch_t::any                                           \
  : S ?                                                                        \
    flecsi::execution::launch_t::single                                        \
  :                                                                            \
    flecsi::execution::launch_t::index

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
