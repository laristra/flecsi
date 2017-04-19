/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_partition_communicator_h
#define flecsi_partition_communicator_h

#include <set>

#include "flecsi/partition/partition_types.h"

///
/// \file
/// \date Initial file creation: Dec 06, 2016
///

namespace flecsi {
namespace dmp {

///
/// \class communicator_t communicator.h
/// \brief communicator_t provides an interface for communicating
///        graph information between different distributed-memory
///        execution instances.
///
class communicator_t
{
public:

  /// Default constructor
  communicator_t() {}

  /// Copy constructor (disabled)
  communicator_t(const communicator_t &) = delete;

  /// Assignment operator (disabled)
  communicator_t & operator = (const communicator_t &) = delete;

  /// Destructor
  virtual ~communicator_t() {}

  // I don't know where this belongs yet, but I want to work on the
  // interface so I'm putting it here for now. It probably doesn't really
  // belong in this interface definition. For one thing, the specialization
  // should have a shot at defining how this type of operation happens. We
  // will also most likely use Legion to arbitrate this type of communication
  // as soon as possible.
  //
  // The point of this method is to get primary ownership information
  // from adjacent ranks.
  virtual
  std::pair<std::vector<std::set<size_t>>, std::set<entry_info_t>>
  get_primary_info(
    const std::set<size_t> & primary,
    const std::set<size_t> & request_indices
  ) = 0;

  ///
  /// Get the 1-to-1 intersection between all partitions of the given set.
  ///
  /// \return A map with an entry for each non-empty intersection containing
  ///         the intersection between the calling partition and an
  ///         intersecting partition.
  ///
  virtual
  std::unordered_map<size_t, std::set<size_t>>
  get_intersection_info(
    const std::set<size_t> & request_indices
  ) = 0;

  // Same admonishment as for get_primary_info...
  //
  // The point of this method is to get entity offsets from the
  // owning ranks.
  virtual
  std::vector<std::set<size_t>>
  get_entity_info(
    const std::set<entry_info_t> & entity_info,
    const std::vector<std::set<size_t>> & request_indices
  ) = 0;

  virtual
  std::unordered_map<size_t, size_t>
  get_set_sizes(
    const std::set<size_t> & index_set
  ) = 0;

private:

}; // class communicator_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_partition_communicator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
