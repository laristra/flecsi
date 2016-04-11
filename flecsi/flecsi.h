/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_h
#define flecsi_h

#include "partition/partitioner.h"
#include "geometry/point.h"
#include "geometry/space_vector.h"
#include "io/io_exodus.h"
#include "io/io_base.h"
#include "io/io.h"
#include "mesh/mesh_utils.h"
#include "mesh/mesh_topology.h"
#include "mesh/mesh_types.h"
#include "execution/context_legion.h"
#include "execution/context.h"
#include "execution/mpi_execution_policy.h"
#include "execution/default_execution_policy.h"
#include "execution/legion_execution_policy.h"
#include "execution/task.h"
#include "specializations/burton/burton_entity_types.h"
#include "specializations/burton/burton_mesh.h"
#include "specializations/burton/burton.h"
#include "specializations/burton/burton_mesh_traits.h"
#include "specializations/burton/burton_types.h"
#include "specializations/burton/burton_io_exodus.h"
#include "specializations/unstruct/unstruct_mesh.h"
#include "specializations/unstruct/unstruct.h"
#include "specializations/unstruct/unstruct_types.h"
#include "utils/bitfield.h"
#include "utils/check_sig.h"
#include "utils/dimensioned_array.h"
#include "utils/index_space.h"
#include "utils/dummy.h"
#include "utils/factory.h"
#include "utils/zip.h"
#include "utils/tuple_zip.h"
#include "utils/doxygen_main.h"
#include "utils/static_list.h"
#include "utils/static_for.h"
#include "utils/tuple_for_each.h"
#include "utils/id.h"
#include "utils/tuple_visit.h"
#include "utils/check_types.h"
#include "utils/iterator.h"
#include "utils/common.h"
#include "utils/detail/zip.h"
#include "utils/detail/tuple_zip.h"
#include "utils/detail/static_for.h"
#include "utils/detail/tuple_for_each.h"
#include "utils/detail/tuple_visit.h"
#include "utils/detail/check_types.h"
#include "utils/const_string.h"
#include "data/default_storage_policy.h"
#include "data/data.h"
#include "data/default_meta_data.h"
#include "data/data_constants.h"

#endif // flecsi_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for Emacs and vim.
 *
 * mode:c++
 * indent-tabs-mode:t
 * c-basic-offset:4
 * tab-width:4
 * vim: set tabstop=4 shiftwidth=4 expandtab :
 *~-------------------------------------------------------------------------~-*/
