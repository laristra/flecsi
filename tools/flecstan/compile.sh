#!/bin/bash

# ------------------------------------------------------------------------------
#   @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
#  /@@/////  /@@          @@////@@ @@////// /@@
#  /@@       /@@  @@@@@  @@    // /@@       /@@
#  /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
#  /@@////   /@@/@@@@@@@/@@       ////////@@/@@
#  /@@       /@@/@@//// //@@    @@       /@@/@@
#  /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
#  //       ///  //////   //////  ////////  //
#
#  Copyright (c) 2019, Triad National Security, LLC
#  All rights reserved.
# ------------------------------------------------------------------------------

# ------------------------
# Setup
# ------------------------

COMPILE='
   g++ -Isrc -std=c++17 -pthread -fno-rtti -pedantic
  -O3 -Wall -Wno-comment -Wno-strict-aliasing -s '

LIBRARIES='
 -Wl,--start-group
 -lclangAnalysis
 -lclangAST
 -lclangBasic
 -lclangDriver
 -lclangEdit
 -lclangFrontend
 -lclangLex
 -lclangParse
 -lclangSema
 -lclangSerialization
 -lclangTooling
 -lLLVMBinaryFormat
 -lLLVMBitReader
 -lLLVMCore
 -lLLVMDemangle
 -lLLVMMC
 -lLLVMMCParser
 -lLLVMOption
 -lLLVMProfileData
 -lLLVMSupport
 -lcurses
 -lz
 -Wl,--end-group'

mkdir -p build


# ------------------------
# Compile
# ------------------------

$COMPILE \
   src/flecstan-misc.cc \
   src/flecstan-arg.cc \
   src/flecstan-yaml.cc \
   src/flecstan-utility.cc \
   src/flecstan-diagnostic.cc \
   src/flecstan-analysis.cc \
   src/flecstan-prep.cc \
   src/flecstan-visitor.cc \
   src/flecstan-main.cc \
-o build/flecstan $LIBRARIES
