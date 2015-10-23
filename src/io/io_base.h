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

  /*!
   * Pure virtual mesh read.
   */
  virtual int32_t read(const std::string &name, burton_mesh_t &m) = 0;
  /*!
    Pure virtual mesh write.
  */
  //FIXME: should allow for const burton_mesh_t & in all of the following.
  //virtual int32_t write(
  //  const std::string &name, const burton_mesh_t &m) = 0;
  virtual int32_t write(
    const std::string &name, burton_mesh_t &m) = 0;

}; // struct io_base_t

/*!
 * Define factory type paramaterized on io_base_t and string.
 */
using io_factory_t = flexi::Factory_<io_base_t, std::string>;

} // namespace flexi

#endif // flexi_io_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
