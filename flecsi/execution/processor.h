/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_processor_h
#define flecsi_processor_h

/*!
 * \file processor.h
 * \authors bergen
 * \date Initial file creation: Aug 02, 2016
 */

namespace flecsi {
namespace execution {

// FIXME: Finish Doxygen

/*!
 */
enum processor_t : size_t {
  loc,
  toc,
  mpi
}; // enum processor_t

} //namespace execution 
} // namespace flecsi

#endif // flecsi_processor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
