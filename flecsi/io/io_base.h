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

#ifndef flecsi_io_io_base_h
#define flecsi_io_io_base_h

#include "flecsi/utils/factory.h"

///
/// \file
/// \date Initial file creation: Oct 07, 2015
///

namespace flecsi {
namespace io {

///
/// \class io_base_t io_base.h
/// \brief io_base_t provides a base class using the object factory with pure
///        virtual functions for read and write.

/// \tparam mesh_t Mesh to template io_base_t on.
///
template <typename mesh_t>
class io_base_t
{
 public:
  /// Default constructor
  io_base_t() {}

  virtual ~io_base_t() {}

  ///
  /// \brief Pure virtual mesh read.
  ///
  /// \param[in] name Read mesh from \e name.
  /// \param[out] m Create mesh \e m from \e name.
  ///
  /// \return Error code. 0 on success.
  ///
  virtual int32_t read(const std::string & name, mesh_t & m) = 0;

  ///
  /// \brief Pure virtual mesh write.
  ///
  /// \param[in] name Write mesh to \e name.
  /// \param[in] m Write mesh \e m to \e name.

  /// \return Error code. 0 on success.
  ///
  /// \note should allow for const mesh_t & in all of the following.
  ///       virtual int32_t write(const std::string &name, const mesh_t &m) = 0;
  virtual int32_t write(const std::string & name, mesh_t & m) = 0;

  ///
  /// \brief Pure virtual mesh write.
  ///
  /// \param[in] name Write mesh to \e name.
  /// \param[in] m Write mesh \e m to \e name.
  /// \param[in] binary write in binary or not.
  ///
  /// \return Error code. 0 on success.
  ///
  /// \remark this version allows specifying binary or ascii
  ///
  virtual int32_t write(const std::string & name, mesh_t & m, bool)
  { return write(name, m); }

}; // struct io_base_t

///
/// \brief Factory type definition for io. flecsi::Factory_ templated on
///        io_base_t and string. Used for registering a file extenstion
///        with a factory.
///
/// \tparam mesh_t Mesh to template io factory on.
///
template <typename mesh_t>
using io_factory_t = flecsi::utils::Factory_<io_base_t<mesh_t>, std::string>;

} // namespace io
} // namespace flecsi

#endif // flecsi_io_io_base_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
