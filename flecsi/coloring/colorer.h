/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_coloring_colorer_h
#define flecsi_coloring_colorer_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 24, 2016
//----------------------------------------------------------------------------//

#include "flecsi/coloring/dcrs.h"

namespace flecsi {
namespace coloring {

///
/// \class colorer_t colorer.h
/// \brief colorer_t provides an interface for creating distributed-memory
///                      colorings from a distributed, compressed-row-storage
///                      graph representation.
///
class colorer_t
{
public:

  /// Default constructor
  colorer_t() {}

  /// Copy constructor (disabled)
  colorer_t(const colorer_t &) = delete;

  /// Assignment operator (disabled)
  colorer_t & operator = (const colorer_t &) = delete;

  /// Destructor
  virtual ~colorer_t() {}

  ///
  /// This method takes a distributed, compressed-row-storage representation
  /// of a graph and returns the indepdentent coloring on a per
  /// execution instance basis, e.g., for each rank or task.
  ///
  /// \param dcrs A distributed, compressed-row-storage representation
  ///             of the graph to color.
  ///
  /// \return The set of indices that belong to the current execution
  ///         instance.
  ///
  virtual
  std::set<size_t>
  color(
    const dcrs_t & dcrs
  ) = 0;

private:

}; // class colorer_t

} // namespace coloring
} // namespace flecsi

#endif // flecsi_coloring_colorer_h
 
/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
