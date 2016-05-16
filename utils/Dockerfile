from fedora:latest

RUN dnf install -y make cmake git gcc-c++ openmpi-devel lapack-devel metis-devel scotch-devel 

RUN groupadd -r flecsi && useradd -r -m -g flecsi flecsi
USER flecsi

WORKDIR /tmp
RUN git clone --recursive https://github.com/losalamos/flecsi.git
WORKDIR flecsi
RUN mkdir build

ENV PATH=$PATH:/usr/lib64/openmpi/bin/

WORKDIR build
RUN cmake -DENABLE_MPI=ON -DENABLE_UNIT_TESTS=ON ..
RUN make -j 2
RUN make test

USER root
make install