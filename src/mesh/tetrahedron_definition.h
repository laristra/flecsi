/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_tetrahedron_definition_h
#define flexi_tetrahedron_definition_h

/*!
 * \file tetrahedron_definition.h
 * \authors bergen
 * \date Initial file creation: Nov 12, 2015
 */

#include "entity_definition.h"

namespace flexi {

/*!
  \class tetrahedron_definition tetrahedron_definition.h
  \brief tetrahedron_definition provides...
 */
class tetrahedron_definition_t : public entity_definition_t
{
public:

  //! Default constructor
  tetrahedron_definition_t() {}

  //! Copy constructor (disabled)
  tetrahedron_definition_t(const tetrahedron_definition_t &) = delete;

  //! Assignment operator (disabled)
  tetrahedron_definition_t &
  operator = (const tetrahedron_definition_t &) = delete;

  //! Destructor
   ~tetrahedron_definition_t() {}

  size_t dimension() override { return 3; }

  size_t sub_entities(size_t dimension) override {
    switch(dimension) {
      case 0:
        return 4;
      case 1:
        return 6;
      case 2:
        return 4;
      default:
        assert(false && "Invalid dimension");
    } // switch
  } // sub_entities

  size_t sub_entity_map(size_t dimension, size_t i, size_t p) override {
    switch(dimension) {
      case 0:
        return vertex_map_[i][p];
      case 1:
        return edge_map_[i][p];
      case 2:
        return face_map_[i][p];
      default:
        assert(false && "Invalid dimension");
    } // switch
  } // sub_entity_map

private:

  static constexpr size_t vertex_map_[4][3] = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
  }; // vertex_map_

  static constexpr size_t edge_map_[6][2] = {
    {0, 1},
    {1, 2},
    {2, 0},
    {0, 3},
    {1, 3},
    {2, 3}
  }; // edge_map_

  static constexpr size_t face_map_[4][3] = {
    {1, 3, 2},
    {0, 3, 2},
    {0, 1, 3},
    {0, 1, 2}
  }; // face_map_

}; // class tetrahedron_definition_t

constexpr size_t tetrahedron_definition_t::vertex_map_[4][3];
constexpr size_t tetrahedron_definition_t::edge_map_[6][2];
constexpr size_t tetrahedron_definition_t::face_map_[4][3];

} // namespace flexi

#endif // flexi_tetrahedron_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
