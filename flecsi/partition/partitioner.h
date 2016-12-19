/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_partitioner_h
#define flecsi_partition_partitioner_h

#include "flecsi/partition/dcrs.h"

///
/// \file
/// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
/// \class partitioner_t partitioner.h
/// \brief partitioner_t provides...
///
class partitioner_t
{
public:

  /// Default constructor
  partitioner_t() {}

  /// Copy constructor (disabled)
  partitioner_t(const partitioner_t &) = delete;

  /// Assignment operator (disabled)
  partitioner_t & operator = (const partitioner_t &) = delete;

  /// Destructor
  virtual ~partitioner_t() {}

  virtual
  std::set<size_t>
  partition(
    const dcrs_t & dcrs
  ) = 0;

private:

}; // class partitioner_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_partition_partitioner_h
 
/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
