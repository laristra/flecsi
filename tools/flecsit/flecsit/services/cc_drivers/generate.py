#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import datetime
import re
from flecsit.services.service_utils import *

#------------------------------------------------------------------------------#
# Generate C++ header and source files
#------------------------------------------------------------------------------#

def generate(args):

  """
  """

  # Set virtual destructor and add protected section if this is a base class.
  virtual = 'virtual ' if args.baseclass else ''
  protected = 'protected:\n\n' if args.baseclass else ''

  # Setup template keywords if the class is templated.
  template = 'template<typename T>\n' if args.template else ''

  # Setup namespace keywords if a namespace was given.
  namespace_guard = \
    args.namespace.replace("::", "_") + '_' if args.namespace != None else ''

  namespace_start = "namespace " + \
    args.namespace.replace("::", " {\nnamespace ") + \
    " {\n\n" if args.namespace != None else ''

  namespace_end = "} // namespace " + \
    "\n} // namespace ".join(args.namespace.split("::")[::-1]) + "\n\n" \
    if args.namespace != None else ''

  # Setup the type names    
  typename = args.basename + '__' if args.template else args.basename + '_t'    

  # Setup output file names
  hfile = (args.filename if args.filename != None else args.basename) + '.h'

  # Get the current user and date
  date = datetime.datetime.now().strftime("%b %d, %Y")

  # Setup up spaces to use for tabs
  spaces = tab_spaces(args)

  #----------------------------------------------------------------------------#
  # Do substitutions on header template
  #----------------------------------------------------------------------------#
  header_output = cc_header_template.substitute(
    DATE=date,
    SPACES=spaces,
    TABSTOP=args.tabstop,
    BASENAME=args.basename,
    CLASSNAME=typename,
    VIRTUAL=virtual,
    PROTECTED=protected,
    TEMPLATE=template,
    FILENAME=hfile,
    NAMESPACE_START=namespace_start,
    NAMESPACE_END=namespace_end,
    NAMESPACE_GUARD=namespace_guard
  )

  # Output to file (will overwrite if it exists)
  if not args.source:
    fd = open(hfile, 'w')
    fd.write(header_output[1:-1])
    fd.close()
  # if

  # Write a source file if requested.
  if args.ccfile:
    # Setup template keywords if the class is templated.
    template = 'template<typename T>\n' if args.template else ''
    template_type = '<T>' if args.template else ''

    cfile = (args.filename if args.filename != None
      else args.basename) + '.cc'

    namespace_start = "\nnamespace " + \
      args.namespace.replace("::", " {\nnamespace ") + \
      " {\n" if args.namespace != None else ''

    namespace_end = "\n} // namespace " + \
      "\n} // namespace ".join(args.namespace.split("::")[::-1]) + "\n" \
      if args.namespace != None else ''

    #--------------------------------------------------------------------------#
    # Do substitutions on source template
    #--------------------------------------------------------------------------#
    source_output = cc_source_template.substitute(
      DATE=date,
      SPACES=spaces,
      TABSTOP=args.tabstop,
      BASENAME=args.basename,
      CLASSNAME=typename,
      VIRTUAL=virtual,
      PROTECTED=protected,
      TEMPLATE=template,
      TEMPLATE_TYPE=template_type,
      HEADER=hfile,
      FILENAME=cfile,
      NAMESPACE_START=namespace_start,
      NAMESPACE_END=namespace_end,
      NAMESPACE_GUARD=namespace_guard
      )

    # Output to file (will overwrite if it exists)
    fd = open(cfile, 'w')
    fd.write(source_output[1:-1])
    fd.close()
  # if

  # Write a stand-alone source file if requested.
  if args.source:
    # Setup template keywords if the class is templated.
    cfile = (args.filename if args.filename != None
      else args.basename) + '.cc'

    namespace_start = "\nnamespace " + \
      args.namespace.replace("::", " {\nnamespace ") + \
      " {\n" if args.namespace != None else ''

    namespace_end = "\n} // namespace " + \
      "\n} // namespace ".join(args.namespace.split("::")[::-1]) + "\n" \
      if args.namespace != None else ''

    #--------------------------------------------------------------------------#
    # Do substitutions on stand-alone source template
    #--------------------------------------------------------------------------#
    source_output = cc_stand_alone_template.substitute(
      DATE=date,
      SPACES=spaces,
      TABSTOP=args.tabstop,
      FILENAME=cfile,
      NAMESPACE_START=namespace_start,
      NAMESPACE_END=namespace_end,
      NAMESPACE_GUARD=namespace_guard
      )

    # Output to file (will overwrite if it exists)
    fd = open(cfile, 'w')
    fd.write(source_output[1:-1])
    fd.close()
  # if
# generate

#------------------------------------------------------------------------------#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#------------------------------------------------------------------------------#
