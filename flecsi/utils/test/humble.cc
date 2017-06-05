/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

// includes: flecsi
#include "flecsi/utils/humble.h"

// includes: C++
#include <iostream>

// includes: other
#include <cinchtest.h>



// =============================================================================
// Test various constructs in humble.h
// =============================================================================

// TEST
TEST(humble, all)
{
   // test: HERE
   HERE("Here I am");
   HERE("Here " "I am again");

   // test: here_func()
   flecsi::utils::here_func
      (__FILE__, __FUNCTION__, __LINE__, "here again");
   flecsi::utils::here_func
      ("some file", "some function", __LINE__, "and yet again");

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
