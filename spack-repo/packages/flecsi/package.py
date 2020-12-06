# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)


from spack import *
import os


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
    git      = 'https://github.com/laristra/flecsi.git'

    version('devel', branch='devel', submodules=False, preferred=False)

    # These are ordered in the same way as ccmake, i.e., alphabetically

    variant('build_type', default='Release',
            values=('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel'),
            description='The build type to build', multi=False)

    variant('flog', default=False,
            description='Enable FLOG logging utility')

    variant('graphviz', default=False,
            description='Enable GraphViz Support')

    variant('hdf5', default=True,
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

    depends_on('cmake@3.12:3.18.4')

    depends_on('mpich@3.2.1', when='^mpich')
    depends_on('openmpi@3.1.6', when='^openmpi')
    depends_on('legion@ctrl-rep-8:ctrl-rep-99',when='backend=legion')
    depends_on('hpx@1.3.0 cxxstd=17 malloc=system',when='backend=hpx')

    depends_on('legion build_type=Debug',
        when='backend=legion +debug_backend +hdf5')
    depends_on('legion build_type=Debug',
        when='backend=legion +debug_backend ~hdf5')
    depends_on('legion build_type=Release',
        when='backend=legion ~debug_backend +hdf5')
    depends_on('legion build_type=Release',
        when='backend=legion ~debug_backend ~hdf5')

    depends_on('hpx@1.3.0 cxxstd=17 malloc=system build_type=Debug',
        when='backend=hpx +debug_backend')
    depends_on('hpx@1.3.0 cxxstd=17 malloc=system build_type=Release',
        when='backend=hpx ~debug_backend')

#    for back in 'legion','hpx':
#        depends_on('mpi', when='backend='+back)
#        for debug,bt in ('+','Debug'),('~','Release'):
#            depends_on(back+' build_type='+bt,
#                       when='backend=%s %sdebug_backend'%(back,debug))
#    del back,debug,bt

    depends_on('mpi', when='backend=mpi')
    depends_on('legion+hdf5',when='backend=legion +hdf5')
    depends_on('hdf5@1.10.7:',when='backend=legion +hdf5')

    depends_on('boost@1.70.0: cxxstd=17 +program_options +atomic '
        '+filesystem +regex +system')
    depends_on('metis@5.1.0:')
    depends_on('parmetis@4.0.3:')
    depends_on('graphviz', when='+graphviz')
    depends_on('kokkos@3.2.00:', when='+kokkos')
    depends_on('hdf5+mpi', when='+hdf5')

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

        if self.run_tests:
            options.append('-DENABLE_FLOG=ON')
            options.append('-DENABLE_UNIT_TESTS=ON')
        else:
            options.append('-DENABLE_FLOG=OFF')
            options.append('-DENABLE_UNIT_TESTS=OFF')

        if '+openmp' in spec:
            options.append('-DENABLE_OPENMP=ON')
        else:
            options.append('-DENABLE_OPENMP=OFF')

        if '+shared' in spec:
            options.append('-DBUILD_SHARED_LIBS=ON')
        else:
            options.append('-DBUILD_SHARED_LIBS=OFF')

        if '+flog' in spec:
            options.append('-DENABLE_FLOG=ON')

        if '+graphviz' in spec:
            options.append('-DENABLE_GRAPHVIZ=ON')

        if '+hdf5' in spec and spec.variants['backend'].value != 'hpx':
            options.append('-DENABLE_HDF5=ON')
        else:
            options.append('-DENABLE_HDF5=OFF')

        return options
