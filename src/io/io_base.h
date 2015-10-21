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

#ifndef flexi_io_base_h
#define flexi_io_base_h

#include "../specializations/burton.h"
#include "../utils/factory.h"

/*!
 * \file io_base.h
 * \authors wohlbier
 * \date Initial file creation: Oct 07, 2015
 */

namespace flexi {

/*!
  \class io_base_t io_base.h
  \brief io_base_t provides a base class using the object factory with pure
         virtual functions for read and write.
 */
class io_base_t {
public:
  //! Default constructor
  io_base_t() {}

  //! pure virtual read
  virtual int32_t read(const std::string &filename, burton_mesh_t &m) = 0;
  //! pure virtual write
  //FIXME: should allow for const burton_mesh_t &
  //virtual int32_t write(
  //    const std::string &filename, const burton_mesh_t &m) = 0;
  virtual int32_t write(
      const std::string &filename, burton_mesh_t &m) = 0;

}; // struct io_base_t

//! define factory type paramaterized on io_base_t and string.
using io_factory_t = flexi::Factory_<io_base_t, std::string>;

} // namespace flexi

#endif // flexi_io_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
