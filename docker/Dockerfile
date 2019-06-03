FROM laristra/flecsi-third-party:fedora

ARG MINIMAL
ARG LEGION
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
RUN cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
          -DENABLE_LEGION=$LEGION \
          -DFLECSI_RUNTIME_MODEL=$RUNTIME \
          ${MINIMAL:+-DCMAKE_DISABLE_FIND_PACKAGE_METIS=ON} \
	  -DFLECSI_ENABLE_TUTORIAL=$([ "$RUNTIME" = "hpx" ] && \
                     echo OFF || echo ON) \
          -DENABLE_UNIT_TESTS=ON \
          -DENABLE_PARMETIS=ON \
          -DENABLE_COLORING=ON \
          -DENABLE_DOXYGEN=ON \
          -DENABLE_DOCUMENTATION=ON \
	  -DENABLE_FLECSTAN=ON \
          ${COVERAGE:+-DENABLE_COVERAGE_BUILD=ON} ..

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
WORKDIR /home/flecsi
