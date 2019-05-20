#!/bin/bash

BUILD_TYPE=Debug

BASE_PATH=/home/nray/Projects/github/flecsi/

FLECSI_INSTALL_DIR=${BASE_PATH}/flecsi-examples/install-gcc-8.2.0
FLECSI_TPL_INSTALL_DIR=${BASE_PATH}/flecsi-third-party/install-gcc-8.2.0

INSTALL_DIR=${BASE_PATH}/flecsi-examples/examples/install-gcc-8.2.0

module purge
module load cmake/3.11.1 
module load openmpi/3.1.3-gcc_8.2.0

CC=`which mpicc`
CXX=`which mpic++`

cmake \
    -D CMAKE_C_COMPILER=${CC} \
    -D CMAKE_CXX_COMPILER=${CXX} \
    -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -D CMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -D FLECSI_DIR=${FLECSI_INSTALL_DIR} \
    -D CMAKE_PREFIX_PATH=${FLECSI_TPL_INSTALL_DIR} \
    ..
