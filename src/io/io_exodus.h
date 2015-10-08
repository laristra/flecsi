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

#include <exodusII.h>

#include "io_base.h"
#include "../specializations/burton.h"

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
  struct io_exodus_t : public io_base_t {

    //! Default constructor
    io_exodus_t() {}

    //! implementation of read
    int32_t read(const std::string & filename, burton_mesh_t & m) {
      return 0;
    } // read

    //! implementation of write
    int32_t write(const std::string & filename, const burton_mesh_t & m);

  }; // struct io_exodus_t

  //! create an io_exodus_t and return a pointer to the base class
  io_base_t * create_io_exodus() {
    return new io_exodus_t;
  } // create_io_exodus

  //! register file extension g with factory
  bool exodus_g_registered
    = io_factory_t::instance().registerType("g", create_io_exodus);

  //! register file extension exo with factory
  bool exodus_exo_registered
    = io_factory_t::instance().registerType("exo", create_io_exodus);

  //! implementation of write
  int32_t io_exodus_t::write(const std::string & filename, 
    const burton_mesh_t & m) {

    std::cout << "Writing mesh to file: " << filename << std::endl;

    auto CPU_word_size = 0, IO_word_size = 0;
    auto exoid = ex_create(filename.c_str(), EX_CLOBBER,
			   &CPU_word_size, &IO_word_size);
    auto d = m.dimension();
    auto num_nodes = m.numVertices();
    auto num_elem = m.numCells();
    auto num_elem_blk = 1;
    auto num_node_sets = 0;
    auto num_side_sets = 0;

    // initialize the file.
    int error = ex_put_init(exoid, "test", d, num_nodes, num_elem,
      num_elem_blk, num_node_sets, num_side_sets);

    return 0;
  } // io_exodus_t::write

} // namespace flexi

#endif // flexi_io_exodus_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
