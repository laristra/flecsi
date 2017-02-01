/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>

#include "flecsi/utils/reflection.h"

struct test_type_t
{
  declare_reflected (
    (double) r1,
    (int) r2
  )
}; // struct test_type_t

using namespace flecsi::utils;

DEVEL(reflection) {
  test_type_t t;

  t.r1 = 1.0;

  clog(info) << t.r1 << std::endl;

  clog(info) << reflection::num_variables<test_type_t>::value << std::endl;

  t.r1 = 2.0;

  clog(info) << reflection::variable<0>(t).get() << std::endl;
} // DEVEL

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
