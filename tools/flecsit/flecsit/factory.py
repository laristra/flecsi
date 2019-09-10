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

from flecsit.services import *

#------------------------------------------------------------------------------#
# Service factory to create instances of classes that derive from Service.
#------------------------------------------------------------------------------#

class ServiceFactory:

    """
    Service object factory for instantiating available services discovered
    as subclasses of Service.
    """

    # A dict of factories to be used to access subclass constructors.
    factories = {}

    def create_service(id, subparsers):

        """
        Create the service indicated by id.  This method uses a special naming
        convention for subclass factory classes and creation methods:


            class SubClass(Service):
                class Factory:
                    def create(self): return SubClass()

        Using this convention, new service classes can be instantiated without
        altering the command-line driver logic.  Subclasses are automatically
        identified and added.

        Args:
            id: The qualified name of the subclass to instantiate.

            subparsers: A subparsers object to be used to add command-line
            arguments for the service.
        """

        # If we haven't seen this class, add it to the dict
        if not id in ServiceFactory.factories:
            ServiceFactory.factories[id] = \
                eval(id + '.Factory()')

        # Return the newly created instance
        return ServiceFactory.factories[id].create(subparsers);
    # create_service

    # Cause this method to be static so that it may be called without an
    # instance of ServiceFactory.
    create_service = staticmethod(create_service)

# class ServiceFactory
