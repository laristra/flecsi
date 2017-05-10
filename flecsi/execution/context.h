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

#ifndef flecsi_execution_context_h
#define flecsi_execution_context_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 19, 2015
//----------------------------------------------------------------------------//

#include <cstddef>
#include <unordered_map>

#include "cinchlog.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/coloring/index_coloring.h"
#include "flecsi/coloring/coloring_types.h"

clog_register_tag(context);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The context__ type provides a high-level runtime context interface that
//! is implemented by the given context policy.
//!
//! @tparam CONTEXT_POLICY The backend context policy.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<class CONTEXT_POLICY>
struct context__ : public CONTEXT_POLICY
{
  using index_coloring_t = flecsi::coloring::index_coloring_t;
  using coloring_info_t = flecsi::coloring::coloring_info_t;

  //---------------------------------------------------------------------------/
  //! Myer's singleton instance.
  //!
  //! @return The single instance of this type.
  //---------------------------------------------------------------------------/

  static
  context__ &
  instance()
  {
    static context__ context;
    return context;
  } // instance

  //---------------------------------------------------------------------------/
  //! Add an index coloring.
  //!
  //! @param key The map key.
  //! @param coloring The index coloring to add.
  //! @param coloring The index coloring information to add.
  //---------------------------------------------------------------------------/

  void
  add_coloring(
    size_t key,
    index_coloring_t & coloring,
    std::unordered_map<size_t, coloring_info_t> & coloring_info
  )
  {
    clog_assert(colorings_.find(key) == colorings_.end(),
      "color index already exists");

    colorings_[key] = coloring;
    coloring_info_[key] = coloring_info;
  } // add_coloring

  //---------------------------------------------------------------------------/
  //! Return the index coloring referenced by key.
  //!
  //! @param key The key associated with the coloring to be returned.
  //---------------------------------------------------------------------------/

  const index_coloring_t &
  coloring(
    size_t key
  )
  {
    if(colorings_.find(key) == colorings_.end()) {
      clog(fatal) << "invalid key " << key << std::endl;
    } // if

    return colorings_[key];
  } // coloring

  //---------------------------------------------------------------------------/
  //! Return the index coloring information referenced by key.
  //!
  //! @param key The key associated with the coloring information
  //!            to be returned.
  //---------------------------------------------------------------------------/

  const std::unordered_map<size_t, coloring_info_t> &
  coloring_info(
    size_t key
  )
  {
    if(coloring_info_.find(key) == coloring_info_.end()) {
      clog(fatal) << "invalid key " << key << std::endl;
    } // if

    return coloring_info_[key];
  } // coloring_info

  //---------------------------------------------------------------------------/
  //! Return the coloring map (convenient for iterating through all
  //! of the colorings.
  //!
  //! @return The map of index colorings.
  //---------------------------------------------------------------------------/

  const std::unordered_map<size_t, index_coloring_t> &
  coloring_map()
  const
  {
    return colorings_;
  } // colorings

  //---------------------------------------------------------------------------/
  //! Return the coloring info map (convenient for iterating through all
  //! of the colorings.
  //!
  //! @return The map of index coloring information.
  //---------------------------------------------------------------------------/

  const std::unordered_map<
    size_t,
    std::unordered_map<size_t, coloring_info_t>
  > &
  coloring_info_map()
  const
  {
    return coloring_info_;
  } // colorings

private:

  // Default constructor
  context__() : CONTEXT_POLICY() {}

  // Destructor
  ~context__() {}

  // We don't need any of these
  context__(const context__ &) = delete;
  context__ & operator = (const context__ &) = delete;
  context__(context__ &&) = delete;
  context__ & operator = (context__ &&) = delete;

  // key: virtual index space id
  // value: coloring indices (exclusive, shared, ghost)
  std::unordered_map<size_t, index_coloring_t> colorings_;

  // key: virtual index space.
  // value: map of color to coloring info
  std::unordered_map<size_t,
    std::unordered_map<size_t, coloring_info_t>> coloring_info_;

}; // class context__

} // namespace execution
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_CONTEXT_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi_runtime_context_policy.h"

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The context_t type is the high-level interface to the FleCSI runtime
//! context.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

using context_t = context__<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
