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

        # get a command-line parser
        self.parser = subparsers.add_parser('compile',
            help='Service for compiling user driver files.'
        )

        add_command_line_compiler_options(self.parser)

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

    def main(self, config, args=None):

        """
        """

        includes = generate_compiler_options(config['includes'],
            args.include, 'FLECSIT_INCLUDES', '-I')
        ldflags = generate_compiler_options(config['ldpath'],
            args.ldflags, 'FLECSIT_LDFLAGS', '-L')
        libraries = generate_compiler_options(config['libraries'],
            args.libraries, 'FLECSIT_LIBRARIES', '-l')

        print "INCLUDES: ", includes
        print "LDFLAGS: ", ldflags
        print "LIBRARIES: ", ldflags

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
                config['includes'] += ' -I' + include
        else:
            for include in args.include or []:
                config['includes'] += ' -I' + include

        # Add FleCSI include
        config['includes'] += ' -I' + config['prefix'] + '/include'

        # Add current directory to includes
        config['includes'] += ' -I./'

        # Add any user-provided ldflags to config
        if env_ldflags is not None:
            for path in env_ldflags.split(':') or []:
                config['libraries'] += ' -L' + path
        else:
            for path in args.ldflags or []:
                config['libraries'] += ' -L' + path

        # Add any user-provided libraries to build
        if env_libraries is not None:
            for lib in env_libraries.split(':') or []:
                config['libraries'] += ' -l' + lib
        else:
            for lib in args.library or []:
                config['libraries'] += ' -l' + lib

        # Add driver to build defines
        config['defines'] += ' -DFLECSI_DRIVER=' + args.driver

        # Get the base inptut deck name
        config['deck'] = splitext(basename(args.driver))[0]

        # Execute build
        execute(args.verbose, config)

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
