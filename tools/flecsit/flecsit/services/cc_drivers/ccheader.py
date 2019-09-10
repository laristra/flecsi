#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

from string import Template

cc_header_template = Template(
"""
#pragma once

/*! @file */

${NAMESPACE_START}/*!
  \\todo Add Documentation!
 */

${TEMPLATE}class ${CLASSNAME}
{
public:

${SPACES}/*!
${SPACES}${SPACES}Default constructor
${SPACES} */

${SPACES}${CLASSNAME}() {}

${SPACES}/*!
${SPACES}${SPACES}Copy constructor (disabled)
${SPACES} */

${SPACES}${CLASSNAME}(const ${CLASSNAME} &) = delete;

${SPACES}/*!
${SPACES}${SPACES}Assignment operator (disabled)
${SPACES} */

${SPACES}${CLASSNAME} & operator = (const ${CLASSNAME} &) = delete;

${SPACES}/*!
${SPACES}${SPACES}Destructor
${SPACES} */

${SPACES}${VIRTUAL}~${CLASSNAME}() {}

${PROTECTED}private:

}; // class ${CLASSNAME}

${NAMESPACE_END}
""")

#------------------------------------------------------------------------------#
# vim: set tabstop=2 shiftwidth=2 expandtab :
#------------------------------------------------------------------------------#
