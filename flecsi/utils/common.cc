/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//!
//! \file common.cc
//! \authors bergen
//! \date Initial file creation: Aug 01, 2016
//!

#include "flecsi/utils/common.h"

#include <cstdlib>
#include <memory>

#ifdef __GNUG__
	#include <cxxabi.h>
#endif

namespace flecsi {
namespace utils {

#ifdef __GNUG__

std::string demangle(const char * const name) {
	int status = -4;

	std::unique_ptr<char, void(*)(void*)> res {
		abi::__cxa_demangle(name, NULL, NULL, &status), std::free };

	return (status==0) ? res.get() : name ;
} // demangle

#else

// does nothing if not g++
std::string demangle(const char * const name) {
	return name;
} // demangle

#endif

} // namespace utils
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
