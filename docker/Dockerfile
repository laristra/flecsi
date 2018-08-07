FROM laristra/flecsi-third-party:fedora

ARG CI_PROJECT_DIR

USER root
RUN cd /home
COPY builds /builds

WORKDIR ${CI_PROJECT_DIR}/build
RUN cd ${CI_PROJECT_DIR}/build
RUN make install
COPY ${CI_PROJECT_DIR} /home/flecsi
RUN rm -rf /builds
USER flecsi
WORKDIR /home/flecsi
