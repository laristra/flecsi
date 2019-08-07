# Copyright 2013-2019 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


import os
from spack import *


class FlecsiDeps(Package):
    '''Deps for flecsi'
    '''
    homepage = 'http://flecsi.lanl.gov/'
    git      = 'https://github.com/laristra/flecsi.git'

    version('develop', branch='master', submodules=False)
    variant('backend', default='mpi', values=('serial', 'mpi', 'legion'),
            description='Backend to use for distributed memory')
    variant('caliper', default=False,
            description='Enable Caliper Support')
    variant('graphviz', default=False,
            description='Enable GraphViz Support')
    variant('tutorial', default=False,
            description='Build FleCSI Tutorials')
    variant('flecstan', default=False,
            description='Build FleCSI Static Analyzer')

    depends_on('cmake@3.12.4',  type='build')
    # Requires cinch > 1.0 due to cinchlog installation issue
    depends_on('cinch@1.01:', type='build')
    depends_on('mpi')
    depends_on('gasnet@2019.3.0 ~pshm', when='backend=legion')
    depends_on('legion@ctrl-rep +shared +mpi', when='backend=legion')
    depends_on('boost@1.59.0: cxxstd=11 +program_options')
    depends_on('metis@5.1.0:')
    depends_on('parmetis@4.0.3:')
    depends_on('caliper', when='+caliper')
    depends_on('graphviz', when='+graphviz')
    depends_on('python@3.0:', when='+tutorial')
    depends_on('llvm', when='+flecstan')

    def install(self, spec, prefix):
        with open(os.path.join(spec.prefix, 'package-list.txt'), 'w') as out:
            for dep in spec.dependencies(deptype='build'):
                out.write("%s\n" % dep.format(
                    format_string='${PACKAGE} ${VERSION}'))
                os.symlink(dep.prefix, os.path.join(spec.prefix, dep.name))
