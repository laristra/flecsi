/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_minimal_entity_types_h
#define flexi_minimal_entity_types_h

#include "flexi/mesh/mesh_types.h"
#include "flexi/specializations/minimal/minimal_mesh_traits.h"

/*!
 * \file minimal_entity_types.h
 * \authors bergen
 * \date Initial file creation: Dec 26, 2015
 */

namespace flexi {

/*!
 */
class minimal_vertex_t
  : public mesh_entity_t<0, minimal_mesh_traits_t::num_domains>
{
public:

}; // class minimal_vertex_t

class minimal_edge_t
  : public mesh_entity_t<1, minimal_mesh_traits_t::num_domains>
{
public:

}; // class minimal_edge_t

class minimal_cell_t
  : public mesh_entity_t<2, minimal_mesh_traits_t::num_domains>
{
public:

  std::pair<size_t, std::vector<size_t>> create_entities(size_t dimension,
    std::vector<id_t> & entities, id_t * vertices, size_t vertex_count) {
    return {0,{0}};
  } // create_entities

}; // class minimal_cell_t

} // namespace flexi

#endif // flexi_minimal_entity_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
