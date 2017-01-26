FROM laristra/flecsi-third-party:fedora

ARG MINIMAL
ARG MPI
ARG RUNTIME
ARG COVERAGE
ARG CC
ARG CXX
ARG CXXFLAGS

#for coverage
ENV CI true
ENV TRAVIS true
ARG TRAVIS_BRANCH
ARG TRAVIS_JOB_NUMBER
ARG TRAVIS_PULL_REQUEST 
ARG TRAVIS_JOB_ID
ARG TRAVIS_TAG
ARG TRAVIS_REPO_SLUG
ARG TRAVIS_COMMIT

RUN rm -rf /home/flecsi/.ccache
COPY flecsi /home/flecsi/flecsi
COPY ccache/ /home/flecsi/.ccache
USER root
RUN chown -R flecsi:flecsi /home/flecsi/flecsi /home/flecsi/.ccache
USER flecsi

WORKDIR /home/flecsi/flecsi
RUN mkdir build

WORKDIR build
RUN ccache -z
RUN cmake -DENABLE_MPI=$MPI \
          -DFLECSI_RUNTIME_MODEL=$RUNTIME \
          ${MINIMAL:+-DCMAKE_DISABLE_FIND_PACKAGE_EXODUSII=ON \
          -DCMAKE_DISABLE_FIND_PACKAGE_METIS=ON  \
          -DCMAKE_DISABLE_FIND_PACKAGE_LAPACKE=ON} \
          -DENABLE_UNIT_TESTS=ON \
          -DENABLE_FLECSIT=ON \
          -DENABLE_DOXYGEN=ON \
          -DENABLE_DOCUMENTATION=ON \
          ${COVERAGE:+-DENABLE_COVERAGE_BUILD=ON} ..

RUN make VERBOSE=1 -j2
RUN ccache -s
RUN ctest --verbose
RUN make install DESTDIR=${PWD}
RUN make doxygen
RUN cd .. && if [ ${COVERAGE} ]; then \
  if [ ${CC} = clang ]; then \
    $HOME/.local/bin/codecov --gcov-exec "llvm-cov gcov"; \
  else \
    $HOME/.local/bin/codecov; \
  fi; \
fi && cd -
USER root
RUN make install
USER flecsi
WORKDIR /home/flecsi
