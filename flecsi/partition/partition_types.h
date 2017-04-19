/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_partition_types_h
#define flecsi_partition_partition_types_h

///
/// \file
/// \date Initial file creation: Apr 18, 2017
///

#include <set>

namespace flecsi {
namespace dmp {

///
/// Type for collecting index space sizes.
///
struct partition_info_t {
  size_t exclusive;
  size_t shared;
  size_t ghost;
}; // struct partition_info_t

///
/// Type for passing partition information about a single entity.
///
struct entry_info_t {
  size_t id;
  size_t rank;
  size_t offset;
  std::set<size_t> shared;

  ///
  /// Constructor.
  ///
  /// \param id_ The entity id. This is generally specified by the
  ///            graph definition.
  /// \param rank_ The rank that owns this entity.
  /// \param offset_ The local id or offset of the entity.
  /// \param shared_ The list of ranks that share this entity.
  ///
  entry_info_t(
    size_t id_ = 0,
    size_t rank_ = 0,
    size_t offset_ = 0,
    std::set<size_t> shared_ = {}
  )
    : id(id_), rank(rank_), offset(offset_), shared(shared_) {}

  ///
  /// Comparision operator for container insertion. This sorts by the
  /// entity id, e.g., as set by the id_ parameter to the constructor.
  ///
  bool
  operator < (
    const entry_info_t & c
  ) const
  {
    return id < c.id;
  } // operator <

  ///
  /// Comparision operator for equivalence.
  ///
  bool
  operator == (
    const entry_info_t & c
  ) const
  {
    return id == c.id &&
      rank == c.rank &&
      offset == c.offset &&
      shared == c.shared;
  } // operator ==

}; // struct entry_info_t

///
/// Helper function to output an entry_info_t.
///
inline
std::ostream &
operator << (
  std::ostream & stream,
  const entry_info_t & e
)
{
  stream << e.id << " " << e.rank << " " << e.offset << " [ ";
  for(auto i: e.shared) {
    stream << i << " "; 
  } // for
  stream << "]";
  return stream;
} // operator <<


} // namespace dmp
} // namespace flecsi

#endif // flecsi_partition_partition_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
