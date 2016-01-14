/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partitioner_h
#define flecsi_partitioner_h

#if 0
/*!
 * \file partitioner.h
 * \authors bergen
 * \date Initial file creation: Oct 26, 2015
 */

namespace flecsi {

#if 0
struct partition_info_t {
  cell_ids
  vertex_ids
  cell_closure
  vertex_closure
}; // struct partition_info_t
#endif

/*!
  \class partitioner partitioner.h
  \brief partitioner provides...
 */
class partitioner
{
public:

  //! Default constructor
  partitioner() {}

  //! Copy constructor (disabled)
  partitioner(const partitioner &) = delete;

  //! Assignment operator (disabled)
  partitioner & operator = (const partitioner &) = delete;

  //! Destructor
  virtual ~partitioner() {}

protected:

private:

}; // class partitioner

} // namespace flecsi

#endif
#endif // flecsi_partitioner_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
