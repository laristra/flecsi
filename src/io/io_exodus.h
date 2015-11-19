/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_io_exodus_h
#define flexi_io_exodus_h

#include "io_base.h"

/*!
 * \file io_exodus.h
 * \authors wohlbier
 * \date Initial file creation: Oct 07, 2015
 */

namespace flexi {

/*!
  \class io_exodus_t io_exodus.h
  \brief io_exodus_t provides a derived type of io_base.h and registrations
         of the file extensions.
 */
template <typename mesh_t>
struct io_exodus_t : public io_base_t<mesh_t> {

  //! Default constructor
  io_exodus_t() {}

  /*!
   * Prototype of exodus mesh read. Implementation provided in specialization.
   */
  int32_t read(const std::string &name, mesh_t &m);

  /*!
   * Prototype of exodus mesh write. Implementation provided in specialization.
   */
  //FIXME: should allow for const mesh_t & in all of the following.
  //int32_t write(const std::string &name, const mesh_t &m);
  int32_t write(const std::string &name, mesh_t &m);

}; // struct io_exodus_t

/*!
 * Create an io_exodus_t and return a pointer to the base class.
 */
template <typename mesh_t>
io_base_t<mesh_t> *create_io_exodus() { return new io_exodus_t<mesh_t>; } // create_io_exodus

} // namespace flexi

#endif // flexi_io_exodus_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
