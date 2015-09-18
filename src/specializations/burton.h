/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef jali_burton_h
#define jali_burton_h

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace jali {

/*!
  \class burton burton.h
  \brief burton provides...
 */
class burton
{
public:

  //! Default constructor
  burton() {}

  //! Copy constructor (disabled)
  burton(const burton &) = delete;

  //! Assignment operator (disabled)
  burton & operator = (const burton &) = delete;

  //! Destructor
   ~burton() {}

  /*!
    \brief Returns the number of vertices in the mesh.
   */
  size_t num_vertices() const {}

  /*!
    \brief Returns the number of edges in the mesh.
   */
  size_t num_edges() const {}

  /*!
    \brief Returns the number of faces in the mesh.
   */
  size_t num_faces() const {}

  /*!
    \brief Returns the number of facets in the mesh.
   */
  size_t num_facets() const {}

  /*!
    \brief Returns the number of cells in the mesh.
   */
  size_t num_cells() const {}

private:

}; // class burton

} // namespace jali

#endif // jali_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
