#! /usr/bin/env python2
#------------------------------------------------------------------------------#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#------------------------------------------------------------------------------#

import os
import re
import glob
import flecsit
from distutils.core import setup

# This is for future requirements
requires = []

# convenience function to find __init__.py files
def find_packages(path='.'):
    ret = []
    for root, dirs, files in os.walk(path):
        if '__init__.py' in files:
            ret.append(re.sub('^[^A-z0-9_]+', '', root.replace('/', '.')))
    return ret

# setup
setup_options = dict(
	# module name
	name = 'flecsit',

	# module version
	version = flecsit.__version__,

	# description
	description = 'Command Line Tool for FleCSI',

	# long description from README
	long_description = open('README.rst').read(),

	# author
	author = 'Ben Bergen',

	# contact email
	author_email = 'bergen@lanl.gov',

	# scripts to install
	scripts = ['bin/flecsit'],

	# packages
	packages = find_packages(),

	# package dir
	package_dir = { 'flecsit' : 'flecsit' },

	# license
	license = 'NONE'
)

import sys
if not sys.version_info[0] == 2:
    print "Sorry, Python 3 is not supported (yet)"
    sys.exit(1)

# call setup
setup(**setup_options)
