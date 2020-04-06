# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
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
    homepage = 'http://flecsi.org/'
    #git      = 'https://github.com/laristra/flecsi.git'
    git      = 'https://github.com/tuxfan/flecsi.git'

    version('devel', branch='flog-tutorial', submodules=False, preferred=False)

    # These are ordered in the same way as ccmake, i.e., alphabetically

    variant('build_type', default='Release',
            values=('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel'),
            description='The build type to build', multi=False)

    variant('flog', default=False,
            description='Enable flog testing')

    variant('graphviz', default=False,
            description='Enable GraphViz Support')

    variant('hdf5', default=False,
             description='Enable HDF5 Support')

    variant('kokkos', default=False,
             description='Enable Kokkos Support')

    variant('openmp', default=False,
             description='Enable OpenMP Support')

    variant('backend', default='legion',
            values=('legion', 'mpi', 'hpx', 'charmpp'),
            description='Backend to use for distributed memory', multi=False)

    # Spack-specific variants

    variant('debug_backend', default=False,
            description='Build Backend with Debug Mode')

    variant('shared', default=True,
            description='Build shared libraries')

    # Dependencies

    depends_on('cmake@3.12:',  type='build')

    depends_on('mpi', when='backend=legion')
    depends_on('mpi', when='backend=mpi')
    depends_on('mpi', when='backend=hpx')

    depends_on('legion@ctrl-rep+shared+mpi+hdf5 build_type=Debug',
        when='backend=legion +debug_backend +hdf5')
    depends_on('legion@ctrl-rep+shared+mpi build_type=Debug',
        when='backend=legion +debug_backend ~hdf5')
    depends_on('legion@ctrl-rep+shared+mpi+hdf5 build_type=Release',
        when='backend=legion ~debug_backend +hdf5')
    depends_on('legion@ctrl-rep+shared+mpi build_type=Release',
        when='backend=legion ~debug_backend ~hdf5')

    depends_on('hpx@1.3.0 cxxstd=14 malloc=system build_type=Debug',
        when='backend=hpx +debug_backend')
    depends_on('hpx@1.3.0 cxxstd=14 malloc=system build_type=Release',
        when='backend=hpx ~debug_backend')

    depends_on('boost@1.70.0: cxxstd=14 +program_options')
    depends_on('metis@5.1.0:')
    depends_on('parmetis@4.0.3:')
    depends_on('hdf5+mpi', when='+hdf5')
    depends_on('graphviz', when='+graphviz')

    def cmake_args(self):
        spec = self.spec
        options = []

        if spec.variants['backend'].value == 'legion':
            options.append('-DFLECSI_RUNTIME_MODEL=legion')
            options.append('-DENABLE_MPI=ON')
        elif spec.variants['backend'].value == 'mpi':
            options.append('-DFLECSI_RUNTIME_MODEL=mpi')
            options.append('-DENABLE_MPI=ON')
        elif spec.variants['backend'].value == 'hpx':
            options.append('-DFLECSI_RUNTIME_MODEL=hpx')
            options.append('-DENABLE_MPI=ON')

        if '+flog' in spec:
            options.append('-DENABLE_FLOG=ON')

        if self.run_tests:
            options.append('-DENABLE_FLOG=ON')
            options.append('-DENABLE_UNIT_TESTS=ON')

        if '+shared' in spec:
            options.append('-DBUILD_SHARED_LIBS=ON')
        else:
            options.append('-DBUILD_SHARED_LIBS=OFF')

        if '+hdf5' in spec and spec.variants['backend'].value != 'hpx':
            options.append('-DENABLE_HDF5=ON')
        else:
            options.append('-DENABLE_HDF5=OFF')

        return options
