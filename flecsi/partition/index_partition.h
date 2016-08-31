/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_index_partition_h
#define flecsi_dmp_index_partition_h

#include <vector>
#include <cereal/types/vector.hpp>

///
// \file index_partition.h
// \authors bergen
// \date Initial file creation: Aug 17, 2016
///

namespace flecsi {
namespace dmp {

///
// \class partition__ index_partition.h
// \brief partition__ provides...
///
template<typename T>
struct index_partition__
{
  using identifier_t = T;

  //--------------------------------------------------------------------------//
  // Data members.
  //--------------------------------------------------------------------------//

  std::vector<identifier_t> exclusive;
  std::vector<identifier_t> shared;
  std::vector<identifier_t> ghost;

  ///
  // Equality operator.
  //
  // \param ip The index_partition_t to compare with \e this.
  //
  // \return True if \e ip is equivalent to \e this, false otherwise.
  ///
  bool
  operator == (
    const index_partition__ & ip
  ) const
  {
    return (this->exclusive == ip.exclusive &&
      this->shared ==  ip.shared &&
      this->ghost == ip.ghost);
  } // operator ==

  ///
  // Cereal serialization method.
  ///
  template<typename A>
  void serialize(A & archive) {
    archive(exclusive, shared, ghost);
  } // serialize

}; // class partition__

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_index_partition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
