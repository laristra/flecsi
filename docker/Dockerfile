FROM laristra/spack-buildenv:fedora30

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive \
    HOME=/home/laristra \
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
RUN chown -R laristra:laristra $HOME/flecsi $HOME/.ccache
USER laristra

WORKDIR $HOME
RUN export SPACK_SPEC="flecsi@1.4%gcc $([ $BUILD_TYPE == 'Debug' ] && echo +debug_backend || echo '' ) backend=$RUNTIME ^$MPI_PACKAGE" && \
    spack repo add --scope site $HOME/flecsi/spack-repo && \
    spack find && \
    echo "spack install --show-log-on-error $SPACK_SPEC" && \
    spack install --show-log-on-error $SPACK_SPEC && \
    spack module tcl loads parmetis ^$MPI_PACKAGE | tee -a deps.sh && \
    spack module tcl loads --dependencies $SPACK_SPEC | tee -a deps.sh
RUN echo ". $HOME/deps.sh" >> ~/.bashrc
RUN echo ". $HOME/spack/share/spack/setup-env.sh && . $HOME/deps.sh" >> $HOME/load_deps.sh

RUN source $HOME/load_deps.sh && module list

WORKDIR $HOME/flecsi
RUN mkdir build

WORKDIR build
RUN ${CC} --version
RUN ccache -z
RUN source $HOME/load_deps.sh && \
    cmake -DMPIEXEC_PREFLAGS=$([ "$MPI_PACKAGE" = "openmpi" ] && echo '--oversubscribe') \
          -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
          -DFLECSI_RUNTIME_MODEL=$RUNTIME \
          -DHPX_IGNORE_COMPILER_COMPATIBILITY=ON \
          -DENABLE_FLECSIT=$([ "$RUNTIME" = "hpx" ] echo OFF || echo ON) \
          -DENABLE_FLECSI_TUTORIAL=$([ "$RUNTIME" = "hpx" ] echo OFF || echo ON) \
          -DENABLE_UNIT_TESTS=ON \
          -DENABLE_DOXYGEN=ON \
          ${COVERAGE:+-DENABLE_COVERAGE_BUILD=ON} \
          .. && \
    make VERBOSE=1 -j2
RUN ccache -s

RUN source $HOME/load_deps.sh && \
    make doxygen && \
    if [[ ${CLANG_FORMAT} ]]; then \
      make format && git diff --exit-code --ignore-submodules; \
    fi && \
    if [ ${COVERAGE} ]; then \
      python -m coverxygen --xml-dir doc/doxygen/xml/ --src-dir .. --output doxygen.coverage.info; \
      wget -O codecov.sh https://codecov.io/bash; \
      bash codecov.sh -X gcov -f doxygen.coverage.info -F documentation; \
      doxy-coverage --threshold 24 doc/doxygen/xml/; \
    fi && \
    make test ARGS="-V" && \
    make install DESTDIR=${PWD}/install && rm -rf ${PWD}/install && \
    cd .. && \
    if [ ${COVERAGE} ]; then \
      if [ ${CC} = clang ]; then \
        $HOME/.local/bin/codecov -F "${CC}" --gcov-exec "llvm-cov gcov"; \
      else \
        $HOME/.local/bin/codecov -F "${CC}"; \
      fi; \
    fi && cd -

USER root
RUN make install
USER laristra
WORKDIR $HOME

