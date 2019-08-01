#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from flecsit.base import Service
from flecsit.services.unit_drivers.generate import *

#------------------------------------------------------------------------------#
# Unit test handler.
#------------------------------------------------------------------------------#

class CINCHUnitTest(Service):

    #--------------------------------------------------------------------------#
    # Initialization.
    #--------------------------------------------------------------------------#

    def __init__(self, subparsers):

        """
        """

        # get a command-line parser
        self.parser = subparsers.add_parser('unit',
            help='Service to generate unit-test templates.')

        self.parser.add_argument('case',
            help='The name of the unit test case. ' +
                'A unit case may contain several related tests.')

        self.parser.add_argument('name', nargs='?',
            help='The name of the unit test. ' +
                'A unit case may contain several related tests.')

        self.parser.add_argument('-s', '--simple', action="store_true",
            help='Generate a unit test skeleton without ' +
                'documentation or extra tests.')

        self.parser.add_argument('-ts', '--tabstop', action="store",
            default=2, help='set the default tabstop width ')

        # set the callback for this sub-command
        self.parser.set_defaults(func=self.main)

    # __init__

    #--------------------------------------------------------------------------#
    # Main.
    #--------------------------------------------------------------------------#

    def main(self, args=None):

        """
        """

        generate(args)

    # main

    #--------------------------------------------------------------------------#
    # Object factory for service creation.
    #--------------------------------------------------------------------------#

    class Factory:
        def create(self, subparsers): return CINCHUnitTest(subparsers)

# class CINCHUnitTest

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
