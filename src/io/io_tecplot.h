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

#ifndef flecsi_io_tecplot_h
#define flecsi_io_tecplot_h

#include "io_base.h"

/*!
 * \file io_tecplot.h
 * \authors wohlbier
 * \date Initial file creation: Oct 07, 2015
 */

namespace flecsi
{
/*!
  \class io_tecplot_binary_t io_tecplot.h
  \brief io_tecplot_binary_t provides a derived type of io_base.h and registrations
    of the tecplot file extensions.

  \tparam mesh_t Mesh to template io_base_t on.
 */
template <typename mesh_t>
struct io_tecplot_binary_t : public io_base_t<mesh_t> {
  //! Default constructor
  io_tecplot_binary_t() {}

  /*!
    \brief Prototype of tecplot mesh read. Implementation provided in
      specialization.

    \param[in] name Read mesh from \e name.
    \param[out] m Create mesh \e m from \e name.

    \return Error code. 0 on success.
   */
  int32_t read(const std::string & name, mesh_t & m);

  /*!
    \brief Prototype of tecplot mesh write. Implementation provided in
      specialization.

    \param[in] name Write mesh to \e name.
    \param[in] m Write mesh \e m to \e name.

    \return Error code. 0 on success.
   */
  // FIXME: should allow for const mesh_t & in all of the following.
  // int32_t write(const std::string &name, const mesh_t &m);
  int32_t write(const std::string & name, mesh_t & m);

}; // struct io_tecplot_binary_t

/*!
  \class io_tecplot_ascii_t io_tecplot.h
  \brief io_tecplot_ascii_t provides a derived type of io_base.h and registrations
    of the tecplot file extensions.

  \tparam mesh_t Mesh to template io_base_t on.
 */
template <typename mesh_t>
struct io_tecplot_ascii_t : public io_base_t<mesh_t> {
  //! Default constructor
  io_tecplot_ascii_t() {}

  /*!
    \brief Prototype of tecplot mesh read. Implementation provided in
      specialization.

    \param[in] name Read mesh from \e name.
    \param[out] m Create mesh \e m from \e name.

    \return Error code. 0 on success.
   */
  int32_t read(const std::string & name, mesh_t & m);

  /*!
    \brief Prototype of tecplot mesh write. Implementation provided in
      specialization.

    \param[in] name Write mesh to \e name.
    \param[in] m Write mesh \e m to \e name.

    \return Error code. 0 on success.
   */
  // FIXME: should allow for const mesh_t & in all of the following.
  // int32_t write(const std::string &name, const mesh_t &m);
  int32_t write(const std::string & name, mesh_t & m);

}; // struct io_tecplot_ascii_t

/*!
  \brief Create an io_tecplot_binary_t and return a pointer to the base class.

  \tparam mesh_t Mesh type for io_tecplot_binary_t.

  \return Pointer to io_base_t base class of io_tecplot_binary_t.
 */
template <typename mesh_t>
io_base_t<mesh_t> * create_io_tecplot_binary()
{
  return new io_tecplot_binary_t<mesh_t>;
} // create_io_tecplot

/*!
  \brief Create an io_tecplot_binary_t and return a pointer to the base class.

  \tparam mesh_t Mesh type for io_tecplot_binary_t.

  \return Pointer to io_base_t base class of io_tecplot_binary_t.
 */
template <typename mesh_t>
io_base_t<mesh_t> * create_io_tecplot_ascii()
{
  return new io_tecplot_ascii_t<mesh_t>;
} // create_io_tecplot

} // namespace flecsi

#endif // flecsi_io_tecplot_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
