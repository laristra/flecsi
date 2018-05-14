#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <flecsi/execution/context.h>

namespace flecsi {
namespace execution {

struct coloring_map_dp_t
{
  size_t vertices;
  size_t cells;
}; // struct coloring_map_dp_t

void add_colorings_dependent_partition(); 
  
} // namespace execution
} // namespace flecsi