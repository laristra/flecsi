#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from flecsit.base import Service
from flecsit.services.cc_drivers.generate import *

#------------------------------------------------------------------------------#
# Source handler.
#------------------------------------------------------------------------------#

class CINCHSource(Service):

    #--------------------------------------------------------------------------#
    # Initialization.
    #--------------------------------------------------------------------------#

    def __init__(self, subparsers):

        """
        """

        # get a command-line parser
        self.parser = subparsers.add_parser('cc',
            help='Service to generate c++ file templates.')

        # add command-line options
        self.parser.add_argument('-t', '--template', action="store_true",
            help='Create a templated class prototype')

        self.parser.add_argument('-b', '--baseclass', action="store_true",
            help='Create a base class from which other classes can derive. ' +
                'In addition to adding a protected section, this flag ' +
                'causes the desctructor to be virtual.')

        self.parser.add_argument('-c', '--ccfile', action="store_true",
            help='Genefate a c++ source file (in addition to the header)')

        self.parser.add_argument('-s', '--source', action="store_true",
            help='Genefate a c++ source file (no header)')

        self.parser.add_argument('-n', '--namespace', action="store",
            help='Namespace name.' +
                ' If this argument is provided, the created class ' +
                'definitions will be wrapped in the given namespace. ' +
                'Namespace may be nested, e.g., ns1::ns2::ns3 ' +
                '(These will be expanded to the proper syntax.)')

        self.parser.add_argument('basename',
            help='A name to use to create the class and output filenames.' +
                ' This name will be used for the output file' +
                ' unless the -f option is specified explicitly. Type or ' +
                'template identifiers will be appended to create the ' +
                'type name, e.g., my_type -> my_type_t or my_type__ ' +
                '(Double underscore is used to identify unqualified ' +
                'template types.)')

        self.parser.add_argument('-f', '--filename', action="store",
            help='The output file base name.' +
                ' If this argument is not provided,' +
                ' output file names will be created using the basename.')

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
        def create(self, subparsers): return CINCHSource(subparsers)

# class CINCHSource

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
