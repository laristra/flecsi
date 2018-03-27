#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from cmakelists import cmakelists_template

import os
import shutil
import subprocess
import sys
import tempfile

def execute(verbose, build):

    """
    """

    # Build the compile command
#    command = build['compiler'] + ' ' + \
#              build['flags'] + ' ' + \
#              build['defines'] + ' ' + \
#              build['includes'] + ' ' + \
#              build['main'] + ' ' + \
#              build['prefix'] + '/share/FleCSI/runtime/runtime_driver.cc ' + \
#              build['driver'] + ' ' + \
#              '-o ' + build['deck'] + ' ' + \
#              '-L' + build['prefix'] + '/' + build['libprefix'] + ' ' + \
#              build['flecsi'] + ' ' + \
#              build['libraries']

#    if verbose:
#        print command

#    subprocess.call(command.split())
    
    cwd = os.getcwd()
    tmpdir = tempfile.mkdtemp(dir=cwd)
    os.chdir(tmpdir)

    cmakelists_txt = cmakelists_template.substitute(
        CMAKE_MINIMUM_REQUIRED='2.8',
        PROJECT='flecsit_compile_' + build['deck'],
        TARGET=build['deck'],
        DRIVER=cwd + '/' + build['driver'],
        FLECSI_RUNTIME_MAIN=build['main'],
        FLECSI_RUNTIME_DRIVER=build['prefix'] +
            '/share/FleCSI/runtime/runtime_driver.cc',
        INSTALL_PREFIX=cwd,
        FLECSI_DEFINES=build['defines'],
        FLECSI_LIBRARIES=build['libraries']
    )

    fd = open('CMakeLists.txt', 'w')
    fd.write(cmakelists_txt[1:-1])
    fd.close()

    devnull = open(os.devnull, 'w')
    subprocess.call(['/usr/bin/cmake', '.'], stdout=devnull, stderr=devnull)
    subprocess.call(['/usr/bin/make', 'install'], stdout=devnull,
        stderr=devnull)

    #shutil.rmtree(tmpdir)
# execute

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
