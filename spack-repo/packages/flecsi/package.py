# Copyright 2013-2019 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *


class Flecsi(CMakePackage):

    '''FleCSI is a compile-time configurable framework designed to support
       multi-physics application development. As such, FleCSI attempts to
       provide a very general set of infrastructure design patterns that can
       be specialized and extended to suit the needs of a broad variety of
       solver and data requirements. Current support includes multi-dimensional
       mesh topology, mesh geometry, and mesh adjacency information,
       n-dimensional hashed-tree data structures, graph partitioning
       interfaces,and dependency closures.
    '''

    homepage = 'http://flecsi.org'
    git      = 'https://github.com/laristra/flecsi.git'

    version('develop', branch='master', submodules=False)
    version('refactor', branch='feature/refactor', submodules=False)

    variant('runtime', default='legion', values=('legion', 'mpi'),
            description='Backend runtime to use for distributed memory')
    variant('caliper', default=False,
            description='Enable Caliper Support')
    variant('graphviz', default=False,
            description='Enable GraphViz Support')
    variant('tutorial', default=False,
            description='Build FleCSI Tutorials')

    depends_on('cmake', type='build')
    depends_on('mpi', when='runtime=mpi')
    depends_on('mpi', when='runtime=legion')
    depends_on('legion@ctrl-rep +shared +mpi', when='runtime=legion')
    depends_on('boost')
    depends_on('parmetis')
    depends_on('caliper', when='+caliper')
    depends_on('graphviz', when='+graphviz')
    depends_on('python', when='+tutorial')

    def cmake_args(self):
        options = ['-DCMAKE_BUILD_TYPE=debug']

        if self.spec.variants['runtime'].value == 'legion':
            options.append('-DFLECSI_RUNTIME_MODEL=legion')
        elif self.spec.variants['runtime'].value == 'mpi':
            options.append('-DFLECSI_RUNTIME_MODEL=mpi')

        if '+tutorial' in self.spec:
            options.append('-DENABLE_FLECSIT=ON')
            options.append('-DENABLE_FLECSI_TUTORIAL=ON')
        else:
            options.append('-DENABLE_FLECSIT=OFF')
            options.append('-DENABLE_FLECSI_TUTORIAL=OFF')

        if '+caliper' in self.spec:
            options.append('-DENABLE_CALIPER=ON')
        else:
            options.append('-DENABLE_CALIPER=OFF')

        return options
