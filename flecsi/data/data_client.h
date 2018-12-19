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

#include <flecsi/utils/export_definitions.h>

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! Base type to identify types that allow data registration.
//----------------------------------------------------------------------------//

class FLECSI_EXPORT data_client_t
{
public:
  /// Copy constructor (disabled)
  data_client_t(const data_client_t &) = delete;

  /// Assignment operator (disabled)
  data_client_t & operator=(const data_client_t &) = delete;

  /// Allow move construction
  data_client_t(data_client_t && dc) {}

  /// Allow move assignment
  data_client_t & operator=(data_client_t && dc) {
    return *this;
  }

  //--------------------------------------------------------------------------//
  //! Return a unique runtime identifier for namespace access to the
  //! data manager.
  //--------------------------------------------------------------------------//

  virtual ~data_client_t() {
    reset();
  }

protected:
  void reset() {}

  //--------------------------------------------------------------------------//
  //! Define a dummy type so that we get our own counter below.
  //--------------------------------------------------------------------------//

  struct id_t {};

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //!
  //! @note This is protected so that the class cannot be instantiated
  //!       directly. Normally, this would be accomplished by including
  //!       a pure-virtual function. However, this doesn't make sense
  //!       for this type, as the runtime_id method doesn't need to
  //!       be overridden by derived types.
  //--------------------------------------------------------------------------//

  data_client_t() {}

}; // class data_client_t

} // namespace data
} // namespace flecsi
