#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys
import os
from os.path import basename, splitext

from flecsit.base import Service
from flecsit.services.compile_driver.execute import *

#------------------------------------------------------------------------------#
# Documentation handler.
#------------------------------------------------------------------------------#

class FleCSIT_Analysis(Service):

    #--------------------------------------------------------------------------#
    # Initialization.
    #--------------------------------------------------------------------------#

    def __init__(self, subparsers):

        """
        """

        # get a command-line parser
        self.parser = subparsers.add_parser('compile',
            help='Service for compiling user driver files.'
        )

        # add command-line options
        self.parser.add_argument('-I', '--include', action='append',
            help='Specify an include path. This argument may be given' +
                 ' multiple times. Arguments may be of the form' +
                 ' -I/path/to/include, -I /path/to/include, or' +
                 ' --include /path/to/include. Include paths may' +
                 ' also be specified by setting the FLECSIT_INCLUDES' +
                 ' environment variable. If FLECSIT_INCLUDES is set,' +
                 ' it will override any include paths passed as' +
                 ' command line arguements.'
        )

        self.parser.add_argument('-L', '--ldflags', action='append',
            help='Specify a linker path. This argument may be given' +
                 ' multiple times. Arguments may be of the form' +
                 ' -L/path/to/link, -L /path/to/link, or' +
                 ' --ldflags /path/to/include. Linker paths may' +
                 ' also be specified by setting the FLECSIT_LDFLAGS' +
                 ' environment variable. If FLECSIT_LDFLAGS is set,' +
                 ' it will override any ldflags passed as' +
                 ' command line arguements.'
        )

        self.parser.add_argument('-l', '--library', action='append',
            help='Specify a link library. This argument may be given' +
                 ' multiple times. Arguments may be of the form' +
                 ' -lname, -l name, --library name, or' +
                 ' -l/full/path/libname.{a,so}, -l /full/path/libname.{a,so},' +
                 ' , or --library /full/path/libname.{a,so}. Libraries may' +
                 ' also be specified by setting the FLECSIT_LIBRARIES' +
                 ' environment variable. If FLECSIT_LIBRARIES is set,' +
                 ' it will override any libraries passed as' +
                 ' command line arguements.'
        )

        self.parser.add_argument('-v', '--verbose', action='store_true',
            help='Turn on verbose output.'
        )

        self.parser.add_argument('driver',
            help='The file containing the user driver definition.'
        )

        # set the callback for this sub-command
        self.parser.set_defaults(func=self.main)

    # __init__

    #--------------------------------------------------------------------------#
    # Main.
    #--------------------------------------------------------------------------#

    def main(self, build, args=None):

        """
        """

        #----------------------------------------------------------------------#
        # Process command-line arguments
        #----------------------------------------------------------------------#

        # Read environment variables
        env_includes = os.getenv('FLECSIT_INCLUDES')
        env_ldflags = os.getenv('FLECSIT_LDFLAGS')
        env_libraries = os.getenv('FLECSIT_LIBRARIES')

        # Add any user-provided include paths to build
        if env_includes is not None:
            for include in env_includes.split(':') or []:
                build['includes'] += ' -I' + include
        else:
            for include in args.include or []:
                build['includes'] += ' -I' + include

        # Add FleCSI include
        build['includes'] += ' -I' + build['prefix'] + '/include'

        # Add current directory to includes
        build['includes'] += ' -I./'

        # Add any user-provided ldflags to build
        if env_ldflags is not None:
            for path in env_ldflags.split(':') or []:
                build['libraries'] += ' -L' + path
        else:
            for path in args.ldflags or []:
                build['libraries'] += ' -L' + path

        # Add any user-provided libraries to build
        if env_libraries is not None:
            for lib in env_libraries.split(':') or []:
                build['libraries'] += ' -l' + lib
        else:
            for lib in args.library or []:
                build['libraries'] += ' -l' + lib

        # Add driver to build defines
        build['defines'] += ' -DFLECSI_DRIVER=' + args.driver

        # Get the base inptut deck name
        build['deck'] = splitext(basename(args.driver))[0]

        # Execute build
        execute(args.verbose, build)

    # main

    #--------------------------------------------------------------------------#
    # Object factory for service creation.
    #--------------------------------------------------------------------------#

    class Factory:
        def create(self, subparsers): return FleCSIT_Analysis(subparsers)
    # class Factory

# class FleCSIT_Analysis

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
