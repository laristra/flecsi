/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_hexahedron_definition_h
#define flexi_hexahedron_definition_h

/*!
 * \file hexahedron_definition.h
 * \authors bergen
 * \date Initial file creation: Nov 12, 2015
 */

#include "entity_definition.h"

namespace flexi {

/*!
  \class hexahedron_definition hexahedron_definition.h
  \brief hexahedron_definition provides...
 */
class hexahedron_definition_t : public entity_definition_t
{
public:

  //! Default constructor
  hexahedron_definition_t() {}

  //! Copy constructor (disabled)
  hexahedron_definition_t(const hexahedron_definition_t &) = delete;

  //! Assignment operator (disabled)
  hexahedron_definition_t &
  operator = (const hexahedron_definition_t &) = delete;

  //! Destructor
   ~hexahedron_definition_t() {}

  size_t dimension() override { return 3; }

  size_t sub_entities(size_t dimension) override {
    switch(dimension) {
      case 0:
        return 8;
      case 1:
        return 12;
      case 2:
        return 6;
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

  static constexpr size_t vertex_map_[8][3] = {
    {0, 0, 0},
    {1, 0, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
  }; // vertex_map_

  static constexpr size_t edge_map_[12][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7},
    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4}
  }; // edge_map_

  static constexpr size_t face_map_[6][4] = {
    {0, 1, 2, 3},
    {0, 1, 5, 4},
    {1, 2, 6, 5},
    {2, 3, 7, 6},
    {3, 0, 4, 7},
    {4, 5, 6, 7}
  }; // face_map_

}; // class hexahedron_definition_t

constexpr size_t hexahedron_definition_t::vertex_map_[8][3];
constexpr size_t hexahedron_definition_t::edge_map_[12][2];
constexpr size_t hexahedron_definition_t::face_map_[6][4];

} // namespace flexi

#endif // flexi_hexahedron_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
