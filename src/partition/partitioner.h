/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_partitioner_h
#define flexi_partitioner_h

#if 0
/*!
 * \file partitioner.h
 * \authors bergen
 * \date Initial file creation: Oct 26, 2015
 */

namespace flexi {

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

} // namespace flexi

#endif
#endif // flexi_partitioner_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
