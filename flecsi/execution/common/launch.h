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


// FIXME: add reverse

///
/// Macro to convert boolean values for single and index into a valid
/// launch_t at compile time.
///
#define flecsi_bools_to_launch(S, I)                                           \
  S && I ?                                                                     \
    launch_t::any                                                              \
  : S ?                                                                        \
    launch_t::single                                        									 \
  :                                                                            \
    launch_t::index

///
/// Macro to convert launch_t into valid boolean for sinlge at compile time.
///
#define flecsi_launch_to_single(L)                                             \
  launch_t::L == launch_t::any ?                                               \
    true                                                                       \
  : launch_t::L == launch_t::single ?                                          \
    true                                                                       \
  :                                                                            \
    false

///
/// Macro to convert launch_t into valid boolean for index at compile time.
///
#define flecsi_launch_to_index(L)                                              \
  launch_t::L == launch_t::any ?                                               \
    true                                                                       \
  : launch_t::L == launch_t::index ?                                           \
    true                                                                       \
  :                                                                            \
    false

#endif // flecsi_execution_common_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
