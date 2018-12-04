/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_hpx_task_wrapper_h
#define flecsi_execution_hpx_task_wrapper_h

clog_register_tag(wrapper);

namespace flecsi {
namespace execution
{

template <
  typename FUNCTOR_TYPE
>
struct functor_task_wrapper_u
{
};

template <
  size_t KEY,
  typename RETURN,
  typename ARG_TUPLE,
  RETURN (* DELEGATE)(ARG_TUPLE)
>
struct task_wrapper_u
{
  //--------------------------------------------------------------------------//
  //! The task_args_t type defines a task argument type for task
  //! execution through the HPX runtime.
  //--------------------------------------------------------------------------//

//  using task_args_t =
//    typename utils::base_convert_tuple_type<
//    accessor_base_t, data_handle_u<void, 0, 0, 0>, ARG_TUPLE>::type;


};

}
}
#endif //flecsi_execution_hpx_task_wrapper_h
