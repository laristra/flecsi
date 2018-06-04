#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import os.path

#------------------------------------------------------------------------------#
# Query the user before overwriting an existing file.
#------------------------------------------------------------------------------#

def overwrite_existing(filename):

    """
    Query the user before overwrite an existing file.
    """

    if os.path.exists(filename):
        return True if raw_input(filename +
            ' exists.  Overwrite? [y/N]: ') is 'y' else False
    else:
        return True

# overwrite_existing

#------------------------------------------------------------------------------#
# Replace tabs with spaces.
#------------------------------------------------------------------------------#

def tab_spaces(args):

    """
    Replace tabs with spaces.
    """

    return ''.zfill(int(args.tabstop)).replace('0', ' ')

# tab_spaces

#------------------------------------------------------------------------------#
# Add options for compilation, e.g., -I, -L, and -l.
#------------------------------------------------------------------------------#

def add_command_line_compiler_options(parser):
    
    """
    Add options for compilation, e.g., -I and -l.
    """

    # add command-line options
    parser.add_argument('-I', '--include', action='append',
        help='Specify an include path. This argument may be given' +
             ' multiple times. Arguments may be of the form' +
             ' -I/path/to/include, -I /path/to/include, or' +
             ' --include /path/to/include. Include paths may' +
             ' also be specified by setting the FLECSIT_INCLUDES' +
             ' environment variable. If FLECSIT_INCLUDES is set,' +
             ' it will override any include paths passed as' +
             ' command line arguements.'
    )

    parser.add_argument('-D', '--define', action='append',
        help='Specify a preprocessor define. This argument may be given' +
             ' multiple times. Arguments may be of the form' +
             ' -DDEFINE, -D DEFINE, or' +
             ' --define DEFINE. Defines may' +
             ' also be specified by setting the FLECSIT_DEFINES' +
             ' environment variable. If FLECSIT_DEFINES is set,' +
             ' it will override any defines passed as' +
             ' command line arguements.'
    )

    parser.add_argument('-l', '--library', action='append',
        help='Specify a link library. This argument may be given' +
             ' multiple times. Arguments must be of the form' +
             ' -l/full/path/to/libname.{a,so}, ' +
             ' -l /full/path/to/libname.{a,so}, ' +
             ' or --library /full/path/libname.{a,so}. Libraries may' +
             ' also be specified by setting the FLECSIT_LIBRARIES' +
             ' environment variable. If FLECSIT_LIBRARIES is set,' +
             ' it will override any libraries passed as' +
             ' command line arguements.'
    )

    parser.add_argument('-p', '--package', action='append',
        help='Specify a CMake package dependency. This argument may be given' +
             ' multiple times. Arguments must be of the form' +
             ' -ppackage, -p package, ' +
             ' or --package package. Packages may' +
             ' also be specified by setting the FLECSIT_PACKAGES' +
             ' environment variable. If FLECSIT_PACKAGES is set,' +
             ' it will override any packages passed as' +
             ' command line arguements.'
    )

# add_compiler_options

#------------------------------------------------------------------------------#
# Generate a compile string from config information and user arguements.
#------------------------------------------------------------------------------#

def generate_compiler_options(config, entries, environment, option):

    """
    General function to process compiler flags for include, ldflags,
    libraries, and packages.
    """

    opts = {}

    # Add entries from cmake config
    if config:
        opts.update(dict(s.split("=") for s in config.split(' ') if "=" in s))
        opts.update(dict([s, ""] for s in config.split(' ') if not "=" in s))

    # Add command-line entries
    if entries:
        opts.update(dict(s.split("=") for s in entries if "=" in s))
        opts.update(dict([s, ""] for s in entries if not "=" in s))

    # Add environment variable entries
    envflags = os.getenv(environment)

    if envflags is not None:
        opts.update(dict(s.split("=") for s in envflags.split(':') if "=" in s))
        opts.update(dict([s, ""] for s in envflags.split(':') if not "=" in s))

    options = ""

    for k,v in opts.items():
        if v is "":
            options += option + k + ' '
        else:
            options += option + k + '=' + v + ' '

    return options.strip()

# generate_compiler_options

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
