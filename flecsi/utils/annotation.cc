#include "annotation.h"

namespace flecsi {
namespace utils {

const std::string annotation::runtime_setup::name{"set-up"};
const std::string annotation::spl_tlt_init::name{"spl-tlt-init"};
const std::string annotation::create_regions::name{"create-regions"};
const std::string annotation::spl_spmd_init::name{"spl-spmd-init"};
const std::string annotation::driver::name{"driver"};
const std::string annotation::runtime_finish::name{"driver"};

const std::string annotation::execute_task_init::tag{"init-handles"};
const std::string annotation::execute_task_user::tag{"user"};
const std::string annotation::execute_task_finalize::tag{"finalize-handles"};
const std::string annotation::execute_task_initargs::tag{"init-args"};
const std::string annotation::execute_task_prolog::tag{"prolog"};
const std::string annotation::execute_task_epilog::tag{"epilog"};

}}
