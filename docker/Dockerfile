FROM laristra/flecsi-third-party:fedora

USER root
RUN cd /home
COPY builds /builds

WORKDIR /builds/onurcaylak/flecsi/build
RUN cd /builds/onurcaylak/flecsi/build
RUN make install
COPY /builds/onurcaylak/flecsi /home/flecsi
RUN rm -rf /builds
USER flecsi
WORKDIR /home/flecsi
