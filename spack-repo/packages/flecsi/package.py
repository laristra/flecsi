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

    variant('build_type', default='Release', values=('Debug', 'Release'),
            description='The build type to build')
    variant('runtime', default='legion', values=('hpx', 'mpi', 'legion'),
            description='Backend runtime to use for distributed memory')
    variant('shared', default=True, description='Build shared libraries')
    variant('hdf5', default=False,
            description='Enable HDF5 Support')
    variant('caliper', default=False,
            description='Enable Caliper Support')
    variant('graphviz', default=False,
            description='Enable GraphViz Support')
    variant('tutorial', default=False,
            description='Build FleCSI Tutorials')

    depends_on('cmake@3.12:', type='build')
    depends_on('mpi', when='runtime=mpi')
    depends_on('mpi', when='runtime=legion')
    depends_on('hpx@master cxxstd=14', when='runtime=hpx')
    depends_on('legion@ctrl-rep-2 +shared +mpi +hdf5', when='runtime=legion +hdf5')
    depends_on('legion@ctrl-rep-2 +shared +mpi', when='runtime=legion ~hdf5')
    depends_on('boost@1.70.0: cxxstd=14 +program_options', when='runtime=hpx')
    depends_on('boost@1.59.0: cxxstd=11 +program_options', when='runtime~hpx')
    depends_on('parmetis@4.0.3:')
    depends_on('hdf5', when='+hdf5')
    depends_on('caliper', when='+caliper')
    depends_on('graphviz', when='+graphviz')
    depends_on('python@3.0:', when='+tutorial')

    def cmake_args(self):
        options = ['-DBUILD_SHARED_LIBS=%s' % ('+shared' in self.spec)]

        if self.spec.variants['build_type'].value == 'Debug':
            options.append('-DCMAKE_BUILD_TYPE=Debug')
        elif self.spec.variants['build_type'].value == 'Release':
            options.append('-DCMAKE_BUILD_TYPE=Release')

        if self.spec.variants['runtime'].value == 'legion':
            options.append('-DFLECSI_RUNTIME_MODEL=legion')
        elif self.spec.variants['runtime'].value == 'mpi':
            options.append('-DFLECSI_RUNTIME_MODEL=mpi')
        elif self.spec.variants['runtime'].value == 'hpx':
            options.append('-DFLECSI_RUNTIME_MODEL=hpx')
            options.append('-DENABLE_MPI=ON')

        if '+tutorial' in self.spec:
            options.append('-DENABLE_FLECSIT=ON')
            options.append('-DENABLE_FLECSI_TUTORIAL=ON')
        else:
            options.append('-DENABLE_FLECSIT=OFF')
            options.append('-DENABLE_FLECSI_TUTORIAL=OFF')

        if '+hdf5' in self.spec:
            options.append('-DENABLE_HDF5=ON')
        else:
            options.append('-DENABLE_HDF5=OFF')

        if '+caliper' in self.spec:
            options.append('-DENABLE_CALIPER=ON')
        else:
            options.append('-DENABLE_CALIPER=OFF')

        return options
