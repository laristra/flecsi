
# CMake
project(flecstan LANGUAGES CXX)
cmake_minimum_required(VERSION 3.8) # probably not necessary

# C++
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -s")

# Header file location
include_directories(src)

# Source files
file(GLOB SOURCES "src/*.cc")

# Executable
###set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/usr/local/bin") 
add_executable(flecstan ${SOURCES})

# Libraries
target_link_libraries(
   # executable...
   flecstan
   # libraries...
   clangAnalysis clangAST clangBasic clangDriver clangEdit clangFrontend
   clangLex clangParse clangSema clangSerialization clangTooling
   LLVMBinaryFormat LLVMBitReader LLVMCore LLVMMC LLVMMCParser LLVMOption
   LLVMProfileData LLVMSupport z
   # then the usual stupidity of repeating the libraries...
   clangAnalysis clangAST clangBasic clangDriver clangEdit clangFrontend
   clangLex clangParse clangSema clangSerialization clangTooling
   LLVMBinaryFormat LLVMBitReader LLVMCore LLVMMC LLVMMCParser LLVMOption
   LLVMProfileData LLVMSupport z
)
