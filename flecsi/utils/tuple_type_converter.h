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

  template<typename T>
  struct convert_tuple_type_{
    using type = T;
  };

  template<typename... Args>
  struct convert_tuple_type;

  template<typename... Args>
  struct convert_tuple_type<std::tuple<Args...>>{
    using type = std::tuple<typename convert_tuple_type_<Args>::type...>;
  };

  template<typename T, typename TO, bool E>
  struct base_convert_tuple_type_{
    using type = T;
  };

  template<typename T, typename TO>
  struct base_convert_tuple_type_<T, TO, true>{
    using type = TO;
  };

  template<typename T, typename TO>
  struct base_convert_tuple_type_<T, TO, false>{
    using type = T;
  };

  template<class B, typename TO, typename... Args>
  struct base_convert_tuple_type;

  template<class B, typename TO, typename... Args>
  struct base_convert_tuple_type<B, TO, std::tuple<Args...>>{
    using type = std::tuple<typename base_convert_tuple_type_<Args, TO, 
      std::is_base_of<B, Args>::value>::type...>;
  };

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_tuple_type_converter_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
