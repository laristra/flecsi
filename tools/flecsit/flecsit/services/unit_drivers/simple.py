#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from string import Template

simple_unit_template = Template(
"""
/*----------------------------------------------------------------------------*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *----------------------------------------------------------------------------*/

#include <flecsi/utils/ftest.h>

int ${CASE}(int argc, char ** argv) {
  FTEST()

  return FTEST_RESULT();
} // ${CASE}
""")
