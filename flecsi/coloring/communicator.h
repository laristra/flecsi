/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <set>

#include <flecsi/coloring/coloring_types.h>

namespace flecsi {
namespace coloring {

/*!
 Provide interfaces for communicating graph information between different
 distributed-memory colors.

 @ingroup coloring
 */

class communicator_t {
public:
  /*!
   Default constructor
   */

  communicator_t() {}

  /*!
   Copy constructor (disabled)
   */

  communicator_t(const communicator_t &) = delete;

  /// Assignment operator (disabled)

  communicator_t & operator=(const communicator_t &) = delete;

  /// Destructor

  virtual ~communicator_t() {}

  /*!
   Return the size of the communicatora
   @ingroup coloring
   */

  virtual size_t size() const = 0;

  /*!
   Return the rank of the communicator
   @ingroup coloring
   */

  virtual size_t rank() const = 0;

  // I don't know where this belongs yet, but I want to work on the
  // interface so I'm putting it here for now. It probably doesn't really
  // belong in this interface definition. For one thing, the specialization
  // should have a shot at defining how this type of operation happens. We
  // will also most likely use Legion to arbitrate this type of communication
  // as soon as possible.
  //
  // The point of this method is to get primary ownership information
  // from adjacent ranks.

  /*!
   Return information about entities that belong to other colors.
   */

  virtual std::pair<std::vector<std::set<size_t>>, std::set<entity_info_t>>
  get_primary_info(
      const std::set<size_t> & primary,
      const std::set<size_t> & request_indices) = 0;

  /*!
   Get the 1-to-1 intersection between all colorings of the given set.
  
   \return A map with an entry for each non-empty intersection containing
           the intersection between the calling color and an
           intersecting color.
   */

  virtual std::unordered_map<size_t, std::set<size_t>>
  get_intersection_info(const std::set<size_t> & request_indices) = 0;

  /*!
   Return a map of the reduced index information across all colors.
  
   @param local_indices The indices of the calling color.
   */

  virtual std::unordered_map<size_t, std::set<size_t>>
  get_entity_reduction(const std::set<size_t> & local_indices) = 0;

  //--------------------------------------------------------------------------//
  // Same admonishment as for get_primary_info...
  //
  // The point of this method is to get entity offsets from the
  // owning ranks.
  //--------------------------------------------------------------------------//

  /*!
   FIXME documantation
   */
  virtual std::vector<std::set<size_t>> get_entity_info(
      const std::set<entity_info_t> & entity_info,
      const std::vector<std::set<size_t>> & request_indices) = 0;

  /*!
   Return size across all colors.
   */
  
  virtual std::vector<size_t> gather_sizes(const size_t & size) = 0;

  virtual std::unordered_map<size_t, coloring_info_t>
  gather_coloring_info(coloring_info_t & color_info) = 0;

private:
}; // class communicator_t

} // namespace coloring
} // namespace flecsi
