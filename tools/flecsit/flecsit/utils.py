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


from flecsit.factory import *
from flecsit.base import *
from flecsit.services import *

#------------------------------------------------------------------------------#
# create_services
#------------------------------------------------------------------------------#

def create_services(services, subparsers):

    """
    Populate the input dict with available services discovered by
    following the subclasses of Service.

    Args:
        services: Dict to fill with service class instances.
    """

    # Clear the dictionary
    services.clear()

    # Go through the subclasses of Service and create a new instance for
    # each using the ServiceFactory class.
    for s in Service.__subclasses__():

        # This is a little fragile: the name passed to the ServiceFactory
        # create_service method must be qualified relative to the directory
        # in which the utils.py file lives.  The following constructs the
        # relative class name from the subclass __module__ and __name__
        # attributes to satisfy this requirement.  This will need to be
        # updated if the directory structure changes.
        root = s.__module__.split('.')[-1]
        services[s.__name__] = \
            ServiceFactory.create_service(root + '.' + s.__name__, subparsers)
    # for

# create_services
