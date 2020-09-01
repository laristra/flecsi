/*~--------------------------------------------------------------------------~*
 *  * Copyright (c) 2017 Los Alamos National Security, LLC
 *   * All rights reserved.
 *    *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_unit_box_h
#define flecsi_topology_unit_box_h

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <vector>

#include "flecsi/topo/structured/box_utils.hh"

namespace flecsi {
namespace topo {
namespace structured_impl {

//----------------------------------------------------------------------------//
//! The box type is the (much) simplified version of unit_box type.
//!
//! @ingroup structured
//!
//----------------------------------------------------------------------------//

template<std::size_t MESH_DIMENSION>
class box
{
public:
  using id = FLECSI_ID_TYPE;
  using id_array = id[MESH_DIMENSION];

  /************************************************************************
   * Constructors/initializers/destructors
   *
   *************************************************************************/
  box() = default;

  box(std::vector<std::size_t> & upperbnds) {
    assert(upperbnds.size() == MESH_DIMENSION);
    for(std::size_t i = 0; i < MESH_DIMENSION; ++i)
      upperbnds_[i] = upperbnds[i];

    size_ = 1;
    for(std::size_t i = 0; i < MESH_DIMENSION; ++i)
      size_ *= upperbnds_[i] + 1;

    iterator = util::iota_view<id>(0, size_);
  }

  box(const id_array & upperbnds) {
    for(std::size_t i = 0; i < MESH_DIMENSION; ++i)
      upperbnds_[i] = upperbnds[i];

    size_ = 1;
    for(std::size_t i = 0; i < MESH_DIMENSION; ++i)
      size_ *= upperbnds_[i] + 1;

    iterator = util::iota_view<id>(0, size_);
  }

  /*****************************************************************************
   * Basic query methods: lower/upper bounds, strides, sizes, bounds-checking *
   *****************************************************************************/

  /* Upper bounds */
  auto upper_bounds() {
    return upperbnds_;
  } // upper_bounds

  /* Strides */
  template<std::size_t DIM>
  auto stride() const {
    return upperbnds_[DIM] + 1;
  } // stride

  auto stride(std::size_t dim) const {
    return upperbnds_[dim] + 1;
  } // stride

  /* Bounds checking */
  template<std::size_t DIM>
  bool check_bounds_index(id index) {
    return (index <= upperbnds_[DIM]);
  } // check_bounds_index

  bool check_bounds_index(std::size_t dim, id index) {
    return (index <= upperbnds_[dim]);
  } // check_bounds_index

  bool check_bounds_indices(const id_array indices) {
    bool within_bnds = true;
    for(std::size_t i = 0; i < MESH_DIMENSION; i++) {
      within_bnds = within_bnds && (indices[i] <= upperbnds_[i]);

      if(!within_bnds)
        return within_bnds;
      ;
    }
    return within_bnds;
  } // check_bounds_indices

  /* Sizes */
  auto size() {
    return size_;
  } // size

  /*****************************************************************************
   * Basic query methods: indices <-> offset              		      *
   *****************************************************************************/
  auto offset_from_indices(const id_array & indices) const {
    id value;
    if(MESH_DIMENSION == 1) {
      return value = indices[0];
    }
    else {
      value = indices[MESH_DIMENSION - 2] +
              stride(MESH_DIMENSION - 2) * indices[MESH_DIMENSION - 1];
      for(std::size_t i = MESH_DIMENSION - 2; i > 0; i--)
        value = indices[i - 1] + stride(i - 1) * value;
      return value;
    }
  } // offset_from_indices

  auto offset_from_indices(const id_array & indices) {
    return std::as_const(*this).offset_from_indices(indices);
  } // offset_from_indices

  void indices_from_offset(const id & offset, id_array & indices) const {
    id rem = offset;

    for(std::size_t i = 0; i < MESH_DIMENSION; ++i) {
      indices[i] = rem % stride(i);
      rem = (rem - indices[i]) / stride(i);
    }
  } // indices_from_offset

  void indices_from_offset(const id & offset, id_array & indices) {
    return std::as_const(*this).indices_from_offset(offset, indices);
  } // indices_from_offset

  /************************************************************************
   * Iterator
   *************************************************************************/
  util::iota_view<id> iterator;
  auto begin() {
    return iterator.begin();
  }
  auto end() {
    return iterator.end();
  }

private:
  id_array upperbnds_;
  std::size_t size_ = 0;
}; // box

} // namespace structured_impl
} // namespace topo
} // namespace flecsi
#endif
