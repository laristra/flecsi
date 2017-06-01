//
// Created by ollie on 5/26/17.
//

#include "flecsi/execution/serial/execution_policy.h"
#include "flecsi/execution/task.h"

namespace flecsi {
namespace execution {

serial_execution_policy_t::runtime_state_t &
serial_execution_policy_t::runtime_state(void * task)
{
  return reinterpret_cast<flecsi::execution::base_task_t *>
    (task)->runtime_state_;
} // serial_execution_policy_t::runtime_state

} // namespace execution
} // namespace flecsi
