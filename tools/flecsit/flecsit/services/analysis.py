#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys
import os

from flecsit.base import Service
from flecsit.services.service_utils import *
from flecsit.services.analysis_driver.execute import *


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
        self.parser = subparsers.add_parser('analyze',
            help='Service for static analysis.' +
                 ' With no flags, this command takes a list of' +
                 ' source files to process. The form should be' +
                 ' source:\"defines string\":\"include paths string\", e.g.,' +
                 ' foo.cc:\"-I/path/one -I/path/two\":\"-DDEF1 -DDEF2\"'
        )
        
        # Add general compiler options -I, -L, and -l
        add_command_line_compiler_options(self.parser)

        # add command-line options
        self.parser.add_argument('-v', '--verbose', action='store_true',
            help='Turn on verbose output.'
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

        #----------------------------------------------------------------------#
        # Process command-line arguments
        #----------------------------------------------------------------------#

        project_name = args.driver[0][0]
        includes = generate_compiler_options(config['includes'],
            args.include, 'FLECSIT_INCLUDES', '-I')
        ldflags = generate_compiler_options(config['ldflags'],
            args.ldflag, 'FLECSIT_LDFLAGS', '-L')
        libraries = generate_compiler_options(config['libraries'],
            args.library, 'FLECSIT_LIBRARIES', '-l')

        # Copy cmake config to initialize build dict
        build = config

        # Add driver to build defines
        build['driver'] = project_name + '.cc'

        # Execute build
        execute(args.verbose, project_name, build)

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
