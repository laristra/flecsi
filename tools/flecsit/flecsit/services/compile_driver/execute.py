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
              build['main'] + ' ' + \
              build['prefix'] + '/share/FleCSI/runtime/runtime_driver.cc ' + \
              build['driver'] + ' ' + \
              '-o ' + build['deck'] + ' ' + \
              '-L' + build['prefix'] + '/' + build['libprefix'] + ' ' + \
              build['flecsi'] + ' ' + \
              build['libraries']

    if verbose:
        print command

    subprocess.call(command.split())

# execute

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
