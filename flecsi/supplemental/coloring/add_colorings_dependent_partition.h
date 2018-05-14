#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <flecsi/execution/context.h>

namespace flecsi {
namespace execution {

void
dependent_partition_tlt_init(Legion::Context ctx, Legion::Runtime * runtime, context_t & context_); 
  
} // namespace execution
} // namespace flecsi