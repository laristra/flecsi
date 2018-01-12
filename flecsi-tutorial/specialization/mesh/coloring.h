#pragma once

namespace flecsi {
namespace tutorial {

struct coloring_map_t
{
  size_t vertices;
  size_t cells;
}; // struct coloring_map_t

void add_colorings(coloring_map_t map);

} // namespace tutorial
} // namespace flecsi
