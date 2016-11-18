#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import subprocess

def execute(verbose, build):

    """
    """

    # Build the compile command
    command = build['compiler'] + ' ' + \
              build['flags'] + ' ' + \
              build['defines'] + ' ' + \
              build['includes'] + ' ' + \
              build['prefix'] + '/share/flecsi/runtime/runtime_main.cc ' + \
              build['prefix'] + '/share/flecsi/runtime/runtime_driver.cc ' + \
              '-o ' + build['deck'] + '.' + build['system'] + ' ' + \
              '-L' + build['prefix'] + '/' + build['libprefix'] + \
              ' -lflecsi ' + \
              build['libraries']

    if verbose:
        print command

    subprocess.call(command.split())

# execute

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
