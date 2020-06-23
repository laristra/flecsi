#pragma once

/*! @file */

#include <limits>

#include <flecsi/execution/execution.h>

namespace flecsi {
namespace execution {
namespace reduction {
#define flecsi_register_operation_types(operation)                             \
  flecsi_register_reduction_operation(operation, int);                         \
  flecsi_register_reduction_operation(operation, long);                        \
  flecsi_register_reduction_operation(operation, short);                       \
  flecsi_register_reduction_operation(operation, unsigned);                    \
  flecsi_register_reduction_operation(operation, size_t);                      \
  flecsi_register_reduction_operation(operation, float);                       \
  flecsi_register_reduction_operation(operation, double);

flecsi_register_operation_types(sum);
flecsi_register_operation_types(min);
flecsi_register_operation_types(max);
flecsi_register_operation_types(product);
} // namespace reduction
} // namespace execution
} // namespace flecsi
