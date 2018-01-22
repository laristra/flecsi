#pragma once

#include <cstdlib>

namespace flecsi {
namespace tutorial {

// These determine the extents of the input mesh. The mesh format is
// unstructured. However, the input mesh for the tutorial is treated as a
// structured mesh in several places (initialization and output). If the mesh
// input file is changed, these values will need to be updated.

constexpr size_t input_mesh_dimension_x = 16;
constexpr size_t input_mesh_dimension_y = 16;

// The FLECSI_TUTORIAL_INPUT_MESH environment variable should be set by the
// environment scripts provided for the tutorial. This value should never be set
// by the user. It is intedended to allow different installation paths for
// FleCSI.

inline char * input_mesh_file() {
  char * tmp = getenv("FLECSI_TUTORIAL_INPUT_MESH");
  if(tmp == nullptr) {
    std::cerr << "FLECSI_TUTORIAL_INPUT_MESH unset in environment"
      << std::endl;
    std::exit(1);
  } // if

  return tmp;
} // input_mesh_file

} // namespace tutorial
} // namespace flecsi
