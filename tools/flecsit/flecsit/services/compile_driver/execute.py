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

def execute(verbose, debug, build):

    """
    """

    # Create a temporary directory for the build
    cwd = os.getcwd()
    tmpdir = tempfile.mkdtemp(dir=cwd)
    os.chdir(tmpdir)

    # Expand packages arguments for substitution
    required_packages = ''
    packages = build['packages'].split()
    if packages:
        required_packages += \
            '\n# Adding package dependencies specified by the user.\n'

        for package in packages:
            required_packages += \
                'find_package(' + package + ' REQUIRED)\n' + \
                'include_directories(${' + package + '_INCLUDE_DIRS})\n'
            build['libraries'] += ' ${' + package + '_LIBRARIES}'

    # Do substitutions to create the CMakeLists.txt file
    cmakelists_txt = cmakelists_template.substitute(
        CMAKE_MINIMUM_REQUIRED = '2.8',
        REQUIRED_PACKAGES = required_packages,
        PROJECT = 'flecsit_compile_' + build['deck'],
        TARGET = build['deck'],
        DRIVER = cwd + '/' + build['driver'],
        FLECSI_RUNTIME_MAIN = build['main'],
        FLECSI_RUNTIME_DRIVER = build['prefix'] +
            '/share/FleCSI/runtime/runtime_driver.cc',
        INSTALL_PREFIX = cwd,
        FLECSI_DEFINES = build['defines'],
        FLECSI_LIBRARIES = build['libraries']
    )

    fd = open('CMakeLists.txt', 'w')
    fd.write(cmakelists_txt[1:-1])
    fd.close()

    if verbose:
        devnull = None
    else:
        devnull = open(os.devnull, 'w')
    # if

    # Setup compiler and flags
    cxx_compiler = '-DCMAKE_CXX_COMPILER=' + build['compiler']
    cxx_flags = '-DCMAKE_CXX_FLAGS=' + build['flags']

    if debug:
        cxx_debug_flags = '-DCMAKE_BUILD_TYPE=Debug'
    else:
        cxx_debug_flags = '-DCMAKE_BUILD_TYPE=Release'

    # Echo the subprocess call
    if verbose:
        print 'Invoking:'
        print 'cmake ' + cxx_compiler + ' ' + cxx_flags + ' ' + \
            cxx_debug_flags + ' .'

    # Call CMake and make to build the example
    subprocess.call(['cmake', cxx_compiler, cxx_flags,
        cxx_debug_flags, '.'], stdout=devnull)

    if verbose:
        print 'Invoking:'
        print 'make VERBOSE=1'
        subprocess.call(['/usr/bin/make', 'VERBOSE=1'])
        print 'make install VERBOSE=1'
        subprocess.call(['make', 'install', 'VERBOSE=1'],
            stdout=devnull)
    else:
        subprocess.call(['make'])
        subprocess.call(['make', 'install'], stdout=devnull)

    if not debug:
        shutil.rmtree(tmpdir)
# execute

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
