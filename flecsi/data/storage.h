/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_storage_h
#define flecsi_data_storage_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! The storage__ type provides a high-level data model context interface that
//! is implemented by the given storage policy.
//!
//! @tparam USER_META_DATA A user-defined meta data type.
//! @tparam STORAGE_POLICY The backend storage policy.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<
  typename STORAGE_POLICY
>
struct storage__ : public STORAGE_POLICY {

	//--------------------------------------------------------------------------//
  //! Myer's singleton instance.
  //!
  //! @return The single instance of this type.
	//--------------------------------------------------------------------------//

  static storage__ &
  instance()
  {
    static storage__ d;
    return d;
  } // instance

	//--------------------------------------------------------------------------//
  // FIXME: What are these for?
	//--------------------------------------------------------------------------//

  void
  move(
    uintptr_t from,
    uintptr_t to
  )
  {}

  void
  reset(
    uintptr_t runtime_namespace
  )
  {}

private:

  // Default constructor
  storage__() : STORAGE_POLICY() {}

  // Destructor
  ~storage__() {}

  // We don't need any of these
  storage__(const storage__ &) = delete;
  storage__ & operator = (const storage__ &) = delete;
  storage__(storage__ &&) = delete;
  storage__ & operator = (storage__ &&) = delete;

}; // class storage__

} // namespace data
} // namespace flecsi

#include "flecsi/runtime/flecsi_runtime_storage_policy.h"

namespace flecsi {
namespace data {

using storage_t = storage__<FLECSI_RUNTIME_STORAGE_POLICY>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_storage_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
