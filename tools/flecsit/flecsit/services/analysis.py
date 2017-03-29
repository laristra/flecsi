#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import sys

from flecsit.base import Service
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

        # add command-line options
        self.parser.add_argument('-v', '--verbose', action='store_true',
            help='Turn on verbose output.'
        )

        self.parser.add_argument('pack', nargs='*', action='append',
            help='The files to anaylze. Each file pack should be' +
            ' of the form: source:\"defines string\":\"include paths string\"'
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

        # Execute analysis
        execute(args.verbose, args.pack, build)

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
