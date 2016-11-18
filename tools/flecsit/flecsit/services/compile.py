#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys
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
        self.parser.add_argument('-I', '--include', action="append",
            help='Specify an include path. This argument may be given' +
                 ' multiple times. Arguments may be of the form' +
                 ' -I/path/to/include, -I /path/to/include, or' +
                 ' --include /path/to/include.'
        )

        self.parser.add_argument('-v', '--verbose', action="store_true",
            help='Turn on verbose output.')

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

        # Add any user-provided include paths to build
        for include in args.include or []:
            build['includes'] += ' -I' + include

        # Add FleCSI include
        build['includes'] += ' -I' + build['prefix'] + '/include'

        # Add current directory to includes
        build['includes'] += ' -I./'

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
