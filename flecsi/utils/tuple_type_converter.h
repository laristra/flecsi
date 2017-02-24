/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_tuple_type_converter_h
#define flecsi_utils_tuple_type_converter_h

#include <tuple>

/*!
 * \file
 * \authors nickm
 * \date Initial file creation: Feb 24, 2017
 */

namespace flecsi {
namespace utils {

  template<class T>
  struct convert_tuple_type_{
    using type = T;
  };

  template<typename... Args>
  struct convert_tuple_type;

  template <typename... Args>
  struct convert_tuple_type<std::tuple<Args...>>{
    using type = std::tuple<typename convert_tuple_type_<Args>::type...>;
  };

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_tuple_type_converter_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
