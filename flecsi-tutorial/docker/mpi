# FleCSI Tutorial Dockerfile

#------------------------------------------------------------------------------#
# Start with plain Fedora
#------------------------------------------------------------------------------#

FROM fedora:latest

#------------------------------------------------------------------------------#
# Install Fedora packages
#------------------------------------------------------------------------------#

RUN dnf install -y make cmake automake autoconf libtool git which vim-enhanced gcc-c++ gcc-gfortran flex patch doxygen graphviz pandoc python2 openmpi-devel redhat-rpm-config hdf5-openmpi-devel exodusii-devel cereal-devel lapack-devel scotch-devel metis-devel environment-modules python-pip clang llvm compiler-rt ccache texlive-epstopdf-bin ghostscript-core texlive-latex-bin-bin texlive-collection-fontsrecommended texlive-fancyhdr findutils texlive-booktabs gdb wget curl lcov boost-devel hwloc-devel

#------------------------------------------------------------------------------#
# Add the flecsi user
#------------------------------------------------------------------------------#

RUN groupadd -r flecsi
RUN useradd -r -m -g flecsi flecsi

#------------------------------------------------------------------------------#
# Set up build environment
#------------------------------------------------------------------------------#

ENV PATH=/usr/lib64/ccache:/usr/lib64/openmpi/bin/${PATH:+:}${PATH}
ENV LD_LIBRARY_PATH=/usr/lib64/openmpi/lib/${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH}
ENV PYTHONPATH=/usr/local/lib/python2.7/site-packages${PYTHONPATH:+:}${PYTHONPATH}
ENV OPENMPI=true

#------------------------------------------------------------------------------#
# Build flecsi-third-party
#------------------------------------------------------------------------------#

USER root
WORKDIR /home/root
RUN git clone --recursive https://github.com/laristra/flecsi-third-party.git
RUN \
   cd flecsi-third-party && \
   mkdir -p build && \
   cd build && \
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/flecsi-tpl \
      -DENABLE_HPX=OFF \
      .. && \
   make -j2;

#------------------------------------------------------------------------------#
# Build flecsi
#------------------------------------------------------------------------------#

USER root
WORKDIR /home/root
RUN rm -rf flecsi-third-party
RUN git clone --recursive https://github.com/laristra/flecsi.git
RUN \
   cd flecsi && \
   mkdir -p build && \
   cd build && \
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/opt/flecsi \
      -DFLECSI_RUNTIME_MODEL=mpi \
      -DENABLE_COLORING=ON \
      -DENABLE_FLECSIT=ON \
      -DCMAKE_PREFIX_PATH=/opt/flecsi-tpl \
      .. && \
   make -j2 && \
   make install;

RUN cp /opt/flecsi/bin/flecsi-tutorial /usr/share/modulefiles/
RUN mkdir -p /home/flecsi/flecsi-tutorial
RUN cp -r flecsi/flecsi-tutorial/??-* /home/flecsi/flecsi-tutorial/

#------------------------------------------------------------------------------#
# Environment setup
#------------------------------------------------------------------------------#

USER root
WORKDIR /home/flecsi
COPY configfiles/ .

WORKDIR /home
RUN chown -R flecsi.flecsi flecsi

WORKDIR /home/flecsi

# vim: set ts=3 expandtab :
