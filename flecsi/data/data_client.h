/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_client_h
#define flecsi_data_client_h

///
/// \file
/// \date Initial file creation: Mar 23, 2016
///

#include "flecsi/utils/common.h"
 
namespace flecsi {
namespace data {

///
/// \class data_client_t data_client.h
/// \brief data_client_t provides...
///
class data_client_t
{
public:

  /// Copy constructor (disabled)
  data_client_t(const data_client_t &) = delete;

  /// Assignment operator (disabled)
  data_client_t & operator = (const data_client_t &) = delete;

  /// Allow move construction
  data_client_t(data_client_t && dc);

  /// Allow move assignment
  data_client_t & operator = (data_client_t && dc);

  ///
  /// Return a unique runtime identifier for namespace access to the
  /// data manager.
  ///
  uintptr_t
  runtime_id() const
  {
    return (reinterpret_cast<uintptr_t>(this) << 4) ^ id_;
  } // runtime_id

  virtual ~data_client_t() { reset(); }

protected:

  void reset();

  ///
  /// Define a dummy type so that we get our own counter below.
  ///
  struct id_t {};

  ///
  /// Default constructor.
  ///
  /// \note This is protected so that the class cannot be instantiated
  ///       directly. Normally, this would be accomplished by including
  ///       a pure-virtual function. However, this doesn't make sense
  ///       for this type, as the runtime_id method doesn't need to
  ///       be overridden by derived types.
  data_client_t()
  :
    id_(utils::unique_id_t<id_t>::instance().next())
  {}

private:

  size_t id_;

}; // class data_client_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_client_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
