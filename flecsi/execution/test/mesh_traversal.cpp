//
// Created by ollie on 8/30/17.
//

#include <cinchlog.h>
#include <cinchtest.h>

#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/supplemental/mesh/empty_mesh_2d.h"

#define INDEX_ID 0
#define VERSIONS 1

using namespace flecsi;
using namespace supplemental;

clog_register_tag(ghost_access);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
  flecsi::data::legion::dense_handle_t<T, EP, SP, GP>;
#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi
template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t =
flecsi::data::mpi::dense_handle_t<T, EP, SP, GP>;
#endif


using namespace flecsi;
using namespace topology;
