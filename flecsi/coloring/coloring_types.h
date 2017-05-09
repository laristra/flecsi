/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_coloring_coloring_types_h
#define flecsi_coloring_coloring_types_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 18, 2017
//----------------------------------------------------------------------------//

#include <set>

namespace flecsi {
namespace coloring {

//----------------------------------------------------------------------------//
//! Type for collecting aggregate index space information.
//----------------------------------------------------------------------------//

struct coloring_info_t {

  //! The number of exclusive indices.
  size_t exclusive;
  
  //! The number of shared indices.
  size_t shared;

  //! The number of ghost indices.
  size_t ghost;

  //! The aggregate set of colors that depend on our shared indices.
  std::set<size_t> shared_users;

  //! The aggregate set of colors that we depend on for ghosts.
  std::set<size_t> ghost_owners;
}; // struct coloring_info_t

inline
std::ostream &
operator << (
  std::ostream & stream,
  const coloring_info_t & ci
)
{
  stream << "exclusive: " << ci.exclusive << " shared: " << ci.shared <<
    " ghost: " << ci.ghost;
  
  stream << " users [ ";
  for(auto i: ci.shared_users) {
    stream << i << " ";
  } // for
  stream << "]";

  stream << " owners [ ";
  for(auto i: ci.ghost_owners) {
    stream << i << " ";
  } // for
  stream << "]" << std::endl;
} // operator <<

///
/// Type for passing coloring information about a single entity.
///
struct entity_info_t {
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
  entity_info_t(
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
    const entity_info_t & c
  ) const
  {
    return id < c.id;
  } // operator <

  ///
  /// Comparision operator for equivalence.
  ///
  bool
  operator == (
    const entity_info_t & c
  ) const
  {
    return id == c.id &&
      rank == c.rank &&
      offset == c.offset &&
      shared == c.shared;
  } // operator ==

}; // struct entity_info_t

///
/// Helper function to output an entity_info_t.
///
inline
std::ostream &
operator << (
  std::ostream & stream,
  const entity_info_t & e
)
{
  stream << e.id << " " << e.rank << " " << e.offset << " [ ";
  for(auto i: e.shared) {
    stream << i << " "; 
  } // for
  stream << "]";
  return stream;
} // operator <<


} // namespace coloring
} // namespace flecsi

#endif // flecsi_coloring_coloring_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
