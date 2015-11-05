/*~--------------------------------------------------------------------------~*
 * placeholder
 *~--------------------------------------------------------------------------~*/

#ifndef type_h
#define type_h

#include <flexi/specializations/burton/burton.h>
#include <flexi/execution/task.h>

using burton_mesh_t = flexi::burton_mesh_t;
using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;
using real_t = burton_mesh_t::real_t;

const size_t height(2);
const size_t width(5);

// FIXME: Need to expose this type in a simpler way...
using accessor_t =
  flexi::burton_mesh_traits_t::mesh_state_t::accessor_t<real_t>;
using flexi::persistent;

#endif // type_h

/*~-------------------------------------------------------------------------~-*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
