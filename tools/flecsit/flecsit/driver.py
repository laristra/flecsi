#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#------------------------------------------------------------------------------#

from __future__ import generators
import random
import sys
import argparse
from flecsit.__init__ import __version__
from flecsit.factory import *
from flecsit.base import *
from flecsit.utils import *
from flecsit.services import *

#------------------------------------------------------------------------------#
# Internal main routine.  This is the top-level once we are inside of the
# flecsit module.
#------------------------------------------------------------------------------#

def main():

    """
    """

    driver = create_driver()
    return driver.main()

# main

#------------------------------------------------------------------------------#
# Create the command-line driver.
#------------------------------------------------------------------------------#

def create_driver():

    """
    """

    driver = FleCSITDriver()
    return driver

# create_driver

#------------------------------------------------------------------------------#
# FleCSITDriver class
#------------------------------------------------------------------------------#

class FleCSITDriver():

    """
    """

    def __init__(self):

        """
        """

        # initialize empty services dictionary
        self.services = dict()

        # create top-level argument parser
        self.parser = argparse.ArgumentParser()

        # create subparsers object to pass into services
        self.subparsers = self.parser.add_subparsers(help='sub-command help')

        # create all available services
        create_services(self.services, self.subparsers)

        # add command-line options
        self.parser.add_argument('-v', '--version', action='version',
            version='flecsit version: ' + __version__)

    # __init__

    def main(self, args=None):

        """
        """

        # parse arguments and call the appropriate function
        # as set by the Service subclass.
        args = self.parser.parse_args()

        try:
            func = args.func
        except AttributeError:
            self.parser.error("too few arguments")

        func(args)

    # main

# class FleCSITDriver
