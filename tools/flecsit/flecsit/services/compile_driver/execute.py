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

    # Set the absolute path to the driver
    if build['driver'].split('/')[0] in ['.', '..']:
        driver_path = cwd + '/' + build['driver']
    else:
        driver_path = build['driver']

    # Expand packages arguments for substitution
    required_packages = ''
    packages = build['packages'].split()
    if packages:
        required_packages += \
            '\n# Adding package dependencies specified by the user.\n'

        for package in packages:
            if package == "hpx":
                package = "HPX"
                runtime_dir = "-DHPX_DIR=" + build['hpx_dir']
            elif package == "legion":
                package = "Legion"
                runtime_dir = "-DLegion_DIR=" + build['legion_dir']
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
        DRIVER = driver_path,
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
    cxx_debug_flags = '-DCMAKE_CXX_FLAGS_DEBUG=' + build['debug_flags']
    flecsi_dir = "-DFleCSI_DIR=" + build['prefix'] + "/lib64/cmake/FleCSI/"
    print("FLECSI", flecsi_dir)

    # Echo the subprocess call
    if verbose:
        print 'Invoking:'
        print '/usr/bin/cmake ' + cxx_compiler + ' ' + cxx_flags + ' ' + \
            cxx_debug_flags + ' ' + flecsi_dir + ' ' + runtime_dir + ' .'

    # Call CMake and make to build the example
    subprocess.call(['/usr/bin/cmake', cxx_compiler, cxx_flags,
        cxx_debug_flags, flecsi_dir, runtime_dir, '.'], stdout=devnull)

    if verbose:
        print 'Invoking:'
        print '/usr/bin/make install VERBOSE=1'
        subprocess.call(['/usr/bin/make', 'VERBOSE=1'])
        subprocess.call(['/usr/bin/make', 'install', 'VERBOSE=1'],
            stdout=devnull)
    else:
        subprocess.call(['/usr/bin/make'])
        subprocess.call(['/usr/bin/make', 'install'], stdout=devnull)

    if not debug:
        shutil.rmtree(tmpdir)
# execute

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
