FROM laristra/flecsi-third-party:fedora

ARG MINIMAL
ARG MPI
ARG RUNTIME
ARG COVERAGE
ARG SONARQUBE
ARG SONARQUBE_TOKEN
ARG CC
ARG CXX
ARG CXXFLAGS

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

RUN rm -rf /home/flecsi/.ccache
COPY flecsi /home/flecsi/flecsi
COPY ccache/ /home/flecsi/.ccache
COPY sonar/ /home/flecsi/.sonar
USER root
RUN chown -R flecsi:flecsi /home/flecsi/flecsi /home/flecsi/.ccache /home/flecsi/.sonar
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

RUN ${SONARQUBE:+build-wrapper-linux-x86-64 --out-dir bw-output} make VERBOSE=1 -j2
RUN ccache -s
RUN make doxygen
RUN if [ ${COVERAGE} ]; then \
      python -m coverxygen --xml-dir doc/doxygen/xml/ --src-dir .. --output doxygen.coverage.info; \
      wget -O codecov.sh https://codecov.io/bash; \
      bash codecov.sh -X gcov -f doxygen.coverage.info -F documentation; \
      doxy-coverage --threshold 21 doc/doxygen/xml/; \
    fi
RUN ctest --verbose
RUN make install DESTDIR=${PWD}/install && rm -rf ${PWD}/install
RUN cd .. && if [ ${COVERAGE} ]; then \
  if [ ${CC} = clang ]; then \
    $HOME/.local/bin/codecov -F "${CC}" --gcov-exec "llvm-cov gcov"; \
  else \
    $HOME/.local/bin/codecov -F "${CC}"; \
  fi; \
fi && cd -
RUN cd .. && if [ ${TRAVIS_BRANCH} = master ] && [ ${TRAVIS_PULL_REQUEST} = false ] && [ ${SONARQUBE} ]; then \
  sonar-scanner \
    -Dsonar.projectKey=${TRAVIS_REPO_SLUG##*/} \
    -Dsonar.sources=. \
    -Dsonar.exclusions=build/CMakeFiles/**,cinch/**,build/doc/doxygen/**,build/include/** \
    -Dsonar.cfamily.build-wrapper-output=build/bw-output \
    -Dsonar.cfamily.gcov.reportsPath=. \
    -Dsonar.host.url=https://sonarqube.com \
    -Dsonar.organization=${TRAVIS_REPO_SLUG%%/*} \
    -Dsonar.login=${SONARQUBE_TOKEN}; \ 
fi && cd -
USER root
RUN make install
USER flecsi
WORKDIR /home/flecsi
