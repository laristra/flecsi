#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <flecsi/execution/context.h>
#include <flecsi/supplemental/coloring/add_colorings.h>

namespace flecsi {
namespace supplemental {

void add_colorings_unified(coloring_map_t &map); 
  
} // namespace execution
} // namespace flecsi
