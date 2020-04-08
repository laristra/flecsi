FROM laristra/spack-buildenv:fedora30

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive \
    HOME=/home/flecsi \
    BASH_ENV=~/.bashrc

ARG DISTRO
ARG MPI_PACKAGE
ARG BUILD_TYPE
ARG RUNTIME
ARG COVERAGE
ARG CC
ARG CXX
ARG CXXFLAGS
ARG CLANG_FORMAT

#for coverage
ARG CI
ARG TRAVIS
ARG TRAVIS_BRANCH
ARG TRAVIS_JOB_NUMBER
ARG TRAVIS_PULL_REQUEST
ARG TRAVIS_JOB_ID
ARG TRAVIS_TAG
ARG TRAVIS_REPO_SLUG
ARG TRAVIS_COMMIT
ARG TRAVIS_OS_NAME
ARG IGNORE_NOCI

RUN rm -rf $HOME/.ccache
COPY flecsi $HOME/flecsi
COPY ccache/ $HOME/.ccache
USER root
RUN chown -R flecsi:flecsi $HOME/flecsi $HOME/.ccache
USER flecsi

WORKDIR $HOME
RUN spack module tcl loads --dependencies flecsi%gcc backend=$RUNTIME ^$MPI_PACKAGE | tee deps.sh && \
    echo ". $HOME/spack/share/spack/setup-env.sh && . $HOME/deps.sh" >> ~/.bashrc

WORKDIR $HOME/flecsi
RUN mkdir build

WORKDIR build
RUN ${CC} --version
RUN ccache -z
RUN cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
          -DFLECSI_RUNTIME_MODEL=$RUNTIME \
          -DENABLE_FLECSIT=$([ "$RUNTIME" = "hpx" ] echo OFF || echo ON) \
          -DENABLE_FLECSI_TUTORIAL=$([ "$RUNTIME" = "hpx" ] echo OFF || echo ON) \
          -DENABLE_UNIT_TESTS=ON \
          -DENABLE_DOXYGEN=ON \
          -DENABLE_DOCUMENTATION=ON \
          ${COVERAGE:+-DENABLE_COVERAGE_BUILD=ON} \
          ..

RUN make VERBOSE=1 -j2
RUN ccache -s
RUN make doxygen
RUN if [[ ${CLANG_FORMAT} ]]; then \
      make format && git diff --exit-code --ignore-submodules; \
    fi
RUN if [ ${COVERAGE} ]; then \
      python -m coverxygen --xml-dir doc/doxygen/xml/ --src-dir .. --output doxygen.coverage.info; \
      wget -O codecov.sh https://codecov.io/bash; \
      bash codecov.sh -X gcov -f doxygen.coverage.info -F documentation; \
      doxy-coverage --threshold 24 doc/doxygen/xml/; \
    fi
RUN make test ARGS="-V"
RUN make install DESTDIR=${PWD}/install && rm -rf ${PWD}/install
RUN cd .. && if [ ${COVERAGE} ]; then \
  if [ ${CC} = clang ]; then \
    $HOME/.local/bin/codecov -F "${CC}" --gcov-exec "llvm-cov gcov"; \
  else \
    $HOME/.local/bin/codecov -F "${CC}"; \
  fi; \
fi && cd -

USER root
RUN make install
USER flecsi
WORKDIR $HOME
