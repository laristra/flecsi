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
    Add options for compilation, e.g., -I, -L, and -l.
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

    parser.add_argument('-L', '--ldflag', action='append',
        help='Specify a linker path. This argument may be given' +
             ' multiple times. Arguments may be of the form' +
             ' -L/path/to/link, -L /path/to/link, or' +
             ' --ldflag /path/to/include. Linker paths may' +
             ' also be specified by setting the FLECSIT_LDFLAGS' +
             ' environment variable. If FLECSIT_LDFLAGS is set,' +
             ' it will override any ldflag passed as' +
             ' command line arguements.'
    )

    parser.add_argument('-l', '--library', action='append',
        help='Specify a link library. This argument may be given' +
             ' multiple times. Arguments may be of the form' +
             ' -lname, -l name, --library name, or' +
             ' -l/full/path/libname.{a,so}, -l /full/path/libname.{a,so},' +
             ' , or --library /full/path/libname.{a,so}. Libraries may' +
             ' also be specified by setting the FLECSIT_LIBRARIES' +
             ' environment variable. If FLECSIT_LIBRARIES is set,' +
             ' it will override any libraries passed as' +
             ' command line arguements.'
    )

# add_compiler_options

#------------------------------------------------------------------------------#
# Generate a compile string from config information and user arguements.
#------------------------------------------------------------------------------#

def generate_compiler_options(config, paths, environment, option):

    """
    General function to process compiler flags for include, ldflags,
    and libraries.
    """

    options = ""

    # Add paths from cmake config
    if config:
        for path in config.split(' ') or []:
            options += option + path + ' '

    # Read the environment variable
    envflags = os.getenv(environment)

    # If the environment variable is set, use it. Otherwise,
    # use the command-line options passed by the user.
    if envflags is not None:
        for flag in envflags.split(':') or []:
            options += option + flag + ' '
    else:
        if paths:
            for path in paths or []:
                options += option + path + ''

    return options.strip()

# generate_compiler_options

#------------------------------------------------------------------------------#
# Formatting options for emacs and vim.
# vim: set tabstop=4 shiftwidth=4 expandtab :
#------------------------------------------------------------------------------#
