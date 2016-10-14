/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_kernel_h
#define flecsi_execution_kernel_h


#include "flecsi/topology/index_space.h"

///
// \file kernel.h
// \authors nickm, bergen
// \date Initial file creation: Oct 06, 2016
///

namespace flecsi {
namespace execution {

///
//
// \tparam T The entity type of the associated index space.
// \tparam is_storage A boolean indicating whether or not the associated
//                    index space has storage for the referenced entity types.
// \tparam is_owned A boolean indicating whether or not the entity data are
//                  owned by the associated index space.
// \tparam is_sorted A boolean indicating whether or not the associated index
//                   space is sorted.
// \tparam P An optional predicate function used to select indices matching
//           particular criteria.
// \tparam F The calleable object type.
//
// \param is The index space over which to execute the calleable object.
// \param f The calleable object instance.
///
template<
  typename T,
  bool is_storage,
  bool is_owned,
  bool is_sorted,
  typename P,
  typename F
>
void
for_each__(
  flecsi::topology::index_space<T, is_storage, is_owned, is_sorted, P> & is,
  F && f
)
{
  const size_t end = is.end_offset();

  for(size_t i(is.begin_offset()); i<end; ++i) {
    f(std::forward<T>(is.get_offset(i)));
  } // for
} // foreach__

///
//
///
template<
	typename T,
  bool is_storage,
  bool is_owned,
  bool is_sorted,
  typename P,
  class F,
  class S
>
void
reduce_each__(
  flecsi::topology::index_space<T, is_storage, is_owned, is_sorted, P>& is,
  S & variable,
  F && f
)
{
  size_t end = is.end_offset();
  for(size_t i(is.begin_offset()); i<end; ++i) {
    f(std::forward<T>(is.get_offset(i)), variable);
  } // for
} // reduce_each__

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_kernel_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
