#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys
import os
from os.path import basename, splitext

from flecsit.services.service_utils import *
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

        # Get a command-line parser
        self.parser = subparsers.add_parser('compile',
            help='Service for compiling user driver files.'
        )

        # Add general compiler options -I, -L, and -l
        add_command_line_compiler_options(self.parser)

        self.parser.add_argument('-v', '--verbose', action='store_true',
            help='Turn on verbose output.'
        )

        # Required driver argument
        self.parser.add_argument('driver',
            help='The file containing the user driver definition.'
        )

        # Set the callback for this sub-command
        self.parser.set_defaults(func=self.main)

    # __init__

    #--------------------------------------------------------------------------#
    # Main.
    #--------------------------------------------------------------------------#

    def main(self, config, args=None):

        """
        """

        #----------------------------------------------------------------------#
        # Process command-line arguments
        #----------------------------------------------------------------------#

        includes = generate_compiler_options(config['includes'],
            args.include, 'FLECSIT_INCLUDES', '-I')
        ldflags = generate_compiler_options(config['ldflags'],
            args.ldflag, 'FLECSIT_LDFLAGS', '-L')
        libraries = generate_compiler_options(config['libraries'],
            args.library, 'FLECSIT_LIBRARIES', '-l')

        # Copy cmake config to initialize build dict
        build = config

        # Set command-line arguments
        build['includes'] = includes
        build['libraries'] = ldflags + libraries

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
