/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/utils/common.h"

#include <cstdlib>
#include <memory>

#ifdef __GNUG__
	#include <cxxabi.h>
#endif

/*!
 * \file common.cc
 * \authors bergen
 * \date Initial file creation: Aug 01, 2016
 */

namespace flecsi {

#ifdef __GNUG__
std::string demangle(const char* name) {
	int status = -4;

	std::unique_ptr<char, void(*)(void*)> res {
		abi::__cxa_demangle(name, NULL, NULL, &status), std::free };

	return (status==0) ? res.get() : name ;
} // demangle

#else

// does nothing if not g++
std::string demangle(const char* name) {
	return name;
} // demangle

#endif

} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
