/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */
#include <flecsi/topology/mesh_definition.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
//! The mesh_definition_u type...
//!
//! @ingroup mesh-topology
//----------------------------------------------------------------------------//

template<size_t DIMENSION, typename REAL_TYPE = double>
class parallel_mesh_definition_u
  : public mesh_definition_u<DIMENSION, REAL_TYPE>
{
public:
  //! \brief a byte type used for migrating data
  using byte_t = unsigned char;

  //! \brief the floating point type
  using real_t = REAL_TYPE;

  virtual std::vector<size_t>
  entities(size_t from_dimension, size_t to_dimension, size_t id) const = 0;

  virtual const flecsi::coloring::crs_t & entities_crs(size_t from_dim,
    size_t to_dim) const = 0;

  virtual const std::vector<size_t> & local_to_global(size_t dim) const = 0;

  virtual const std::map<size_t, size_t> & global_to_local(
    size_t dim) const = 0;

  virtual void create_graph(size_t from_dimension,
    size_t to_dimension,
    size_t min_connections,
    flecsi::coloring::dcrs_t & dcrs) const = 0;

  virtual void pack(size_t dimension,
    size_t local_id,
    std::vector<byte_t> & buffer) const = 0;

  virtual void
  unpack(size_t dimension, size_t local_id, byte_t const *& buffer) = 0;

  virtual void erase(size_t dimension,
    const std::vector<size_t> & local_ids) = 0;

  virtual void build_connectivity() = 0;

  virtual void vertex(size_t id, real_t * coord) const = 0;

  virtual const std::vector<size_t> & face_owners() const = 0;
  virtual const std::vector<size_t> & region_ids() const = 0;

  virtual std::vector<size_t> element_sides(size_t id) const = 0;

  virtual const flecsi::coloring::crs_t & side_vertices() const = 0;
  virtual const std::vector<size_t> & side_ids() const = 0;
};

//----------------------------------------------------------------------------
// A utilitiy for casting to byte arrays
//----------------------------------------------------------------------------
template<typename DATA_TYPE, typename BUFFER_TYPE>
void
cast_insert(DATA_TYPE const * const data, size_t len, BUFFER_TYPE & buf) {
  using buf_value_t = std::decay_t<decltype(buf[0])>;
  auto n = sizeof(DATA_TYPE) * len;
  auto p = reinterpret_cast<const buf_value_t *>(data);
  buf.insert(buf.end(), p, p + n);
};

//----------------------------------------------------------------------------
// A utilitiy for casting from byte arrays
//----------------------------------------------------------------------------
template<typename BUFFER_TYPE, typename DATA_TYPE>
void
uncast(BUFFER_TYPE const *& buffer, size_t len, DATA_TYPE * data) {
  auto n = sizeof(DATA_TYPE) * len;
  auto p = reinterpret_cast<BUFFER_TYPE *>(data);
  std::copy(buffer, buffer + n, p);
  auto start = buffer + n;
  buffer += n;
};

} // namespace topology
} // namespace flecsi
