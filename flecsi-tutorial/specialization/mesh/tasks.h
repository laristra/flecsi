#pragma once

#include <flecsi-config.h>

#include <flecsi/execution/context.h>
#include <specialization/mesh/inputs.h>
#include <specialization/mesh/mesh.h>
#include <specialization/mesh/types.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Mesh Initialization
//----------------------------------------------------------------------------//

inline void initialize_mesh(mesh<wo> m) {
  auto & context { execution::context_t::instance() };

  auto & vertex_map { context.index_map(index_spaces::vertices) };
  auto & reverse_vertex_map
    { context.reverse_index_map(index_spaces::vertices) };
  auto & cell_map { context.index_map(index_spaces::cells) };

  std::vector<vertex_t *> vertices;

  const size_t width { input_mesh_dimension_x };
  const double dt { 1.0/width };

  for(auto & vm: vertex_map) {
    const size_t mid { vm.second };
    const size_t row { mid/(width+1) };
    const size_t column { mid%(width+1) };

    point_t p { column*dt, row*dt };
    vertices.push_back(m.make<vertex_t>(p));
  } // for

  size_t count{0};
  for(auto & cm: cell_map) {
    const size_t mid { cm.second };

    const size_t row { mid/width };
    const size_t column { mid%width };

    const size_t v0 { (column    ) + (row    ) * (width + 1) };
    const size_t v1 { (column + 1) + (row    ) * (width + 1) };
    const size_t v2 { (column + 1) + (row + 1) * (width + 1) };
    const size_t v3 { (column    ) + (row + 1) * (width + 1) };

    const size_t lv0 { reverse_vertex_map[v0] };
    const size_t lv1 { reverse_vertex_map[v1] };
    const size_t lv2 { reverse_vertex_map[v2] };
    const size_t lv3 { reverse_vertex_map[v3] };

    auto c { m.make<cell_t>() };
    m.init_cell<0>(c, { vertices[lv0], vertices[lv1],
      vertices[lv2], vertices[lv3] });
  } // for

  m.init<0>();
} // initizlize_mesh

} // namespace tutorial
} // namespace flecsi
