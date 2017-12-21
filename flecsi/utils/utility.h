/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#pragma once

//!
//! \file
//! \date Initial file creation: Jan 10, 2017
//!

namespace flecsi {
namespace utils {

//!
//! Forms lvalue reference to const of type T.
//!
//! This function will be deprecated in c++17.
//!
template<typename CT, typename T>
struct as_const {
  using type = T;
}; // struct as_const

} // namespace utils
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
