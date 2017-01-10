/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_index_partition_h
#define flecsi_partition_index_partition_h

#include <cassert>

#include <vector>
#include <cereal/types/vector.hpp>

#include "flecsi/partition/communicator.h"

///
/// \file
/// \date Initial file creation: Aug 17, 2016
///

namespace flecsi {
namespace dmp {

///
/// \class partition__ index_partition.h
/// \brief partition__ provides...
///
template<typename T>
struct index_partition__
{
  using identifier_t = T;
  using entry_info_t = flecsi::dmp::entry_info_t;

  //------------------------------------------------------------------------//
  // Data members.
  //------------------------------------------------------------------------//

  // Vector of mesh ids of the primary partition
  std::set<size_t> primary;

  // Vector of entry_info_t type of the exclusive partition
  std::set<entry_info_t > exclusive;

  // Vector of entry_info_t type of the shared partition
  std::set<entry_info_t> shared;

  // Vector of entry_info_t type of the ghost partition
  std::set<entry_info_t> ghost;

  ///
  /// Equality operator.
  ///
  /// \param ip The index_partition_t to compare with \e this.
  ///
  /// \return True if \e ip is equivalent to \e this, false otherwise.
  ///
  bool
  operator == (
    const index_partition__ & ip
  ) const
  {
    return (
      this->primary == ip.primary &&
      this->exclusive == ip.exclusive &&
      this->shared ==  ip.shared &&
      this->ghost == ip.ghost);
  } // operator ==

  ///
  /// Cereal serialization method.
  ///
  template<typename A>
  void serialize(A & archive) {
    archive(exclusive, shared, ghost);
  } // serialize

}; // struct index_partition__

} // namespace dmp
} // namespace flecsi

#endif // flecsi_partition_index_partition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
