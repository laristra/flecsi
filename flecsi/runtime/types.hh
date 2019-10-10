/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <limits>

#include <flecsi-config.h>

/*----------------------------------------------------------------------------*
  This section works with the build system to select the correct runtime
  implemenation for the task model. If you add to the possible runtimes,
  remember to edit config/packages.cmake to include a definition using
  the same convention, e.g., -DFLECSI_RUNTIME_MODEL_new_runtime.
 *----------------------------------------------------------------------------*/

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion

#include <legion.h>

namespace flecsi {

using field_id_t = Legion::FieldID;
const field_id_t FIELD_ID_MAX = LEGION_MAX_APPLICATION_FIELD_ID;

using task_id_t = Legion::TaskID;
const task_id_t TASK_ID_MAX = LEGION_MAX_APPLICATION_TASK_ID;

} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpi

#include <cstddef>
#include <limits>

namespace flecsi {

using field_id_t = size_t;
const field_id_t FIELD_ID_MAX = std::numeric_limits<size_t>::max();

using task_id_t = size_t;
const task_id_t TASK_ID_MAX = std::numeric_limits<size_t>::max();

} // namespace flecsi

#elif FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx

namespace flecsi {

#include <cstddef>
#include <limits>

using field_id_t = size_t;
const field_id_t FIELD_ID_MAX = std::numeric_limits<size_t>::max();

using task_id_t = size_t;
const task_id_t TASK_ID_MAX = std::numeric_limits<size_t>::max();

} // namespace flecsi

#endif // FLECSI_RUNTIME_MODEL

#include <flecsi/utils/common.hh>

namespace flecsi {

/*!
  Unique counter for field ids.
 */

struct fid_counter_t {};
using unique_fid_t =
  utils::unique_id<fid_counter_t, field_id_t, FLECSI_GENERATED_ID_MAX>;

/*!
  Unique counter for topology ids.
 */

using topology_id_t = size_t;
const topology_id_t TOPOLOGY_ID_MAX = std::numeric_limits<size_t>::max();

struct tid_counter_t {};
using unique_tid_t =
  utils::unique_id<tid_counter_t, topology_id_t, FLECSI_GENERATED_ID_MAX>;

/*!
  Unique counter for index space ids.
 */

struct isid_counter_t {};
using unique_isid_t =
  utils::unique_id<isid_counter_t, field_id_t, FLECSI_GENERATED_ID_MAX>;

/*!
  Unique counter for reduction operator ids.
 */

struct oid_counter_t {};
using unique_oid_t =
  utils::unique_id<oid_counter_t, field_id_t, FLECSI_GENERATED_ID_MAX>;

} // namespace flecsi
