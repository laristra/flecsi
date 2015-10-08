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

#ifndef flexi_io_h
#define flexi_io_h

#include "io_base.h"
#include "io_exodus.h"

/*!
 * \file io.h
 * \authors wohlbier
 * \date Initial file creation: Oct 07, 2015
 */

namespace flexi {

  /*!
  \brief generic file writer that calls the correct method based on the file
         suffix.
  \param filename file to write to
  \param m mesh to write to the file
  \return 0 on success
   */
  int32_t write_mesh(const std::string & filename, 
    const burton_mesh_t & m) {

    // get the file suffix.
    std::string suffix = filename.substr(filename.find_last_of(".")+1);
    // create the io instance with the factory using the file suffix.
    io_base_t * io = io_factory_t::instance().create(suffix);

    // call the io write function
    return io->write(filename, m);
  }

  /*!
  \brief generic file reader that calls the correct method based on the file
         suffix.
  \param filename file to read from
  \param m mesh to create from file
  \return 0 on success
   */
  int32_t read_mesh(const std::string & filename, 
    burton_mesh_t & m) {

    // get the file suffix.
    std::string suffix = filename.substr(filename.find_last_of(".")+1);
    // create the io instance with the factory using the file suffix.
    io_base_t * io = io_factory_t::instance().create(suffix);

    // call the io write function
    return io->read(filename, m);
  }

} // namespace flexi

#endif // flexi_io_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
