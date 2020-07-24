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
namespace topology {
namespace structured_impl {

// This routine is needed to compute number of boxes for
// an index space at compile time.
constexpr inline size_t
binom(size_t n, size_t k) noexcept {
  return (k > n) ? 0 : // out of range
           (k == 0 || k == n) ? 1 : // edge
             (k == 1 || k == n - 1) ? n : // first
               (k + k < n) ? // recursive:
                 (binom(n - 1, k - 1) * n) / k
                           : //  path to k=1   is faster
                 (binom(n - 1, k) * n) / (n - k); //  path to k=n-1 is faster
};

// template<typename T, typename U>
constexpr inline size_t
power(size_t base, size_t exponent) {
  // static_assert(std::is_integral<U>(), "exponent must be integral");
  return exponent == 0 ? 1 : base * power(base, exponent - 1);
}

//----------------------------------------------------------------------------//
//! The unit box type.
//! @ingroup
//! This type is the basic type used to define index-spaces for structured
//! topologies. The type implements the notion of bounding boxes covering
//! entities of dimension d in a logically structured mesh of dimension M.
//! Note that d >= 0 and d <= M.
//! Since a single box cannot cover all entities of an intermediate dimension
//! in the mesh with dimension M, this type supports multiple boxes to cover
//! such entities. For example, in a 2D structured mesh, all the edges can be
//! enumerated by defining two bounding boxes.
//!
//! This type supports two views of the enumerated entities, global view and
//! local view.
//! The lower and upper bounds of the box can be defined w.r.t to another box.
//! This type supports representing a box embedded in another box. As a result,
//! the lower/upper bounds can be defined w.r.t to the outer box. The strides
//! are the strides of the outer box.
//!
//!   *--------------------*
//!   |                    |
//!   |                    |
//!   |     x------x       |
//!   |     |      |       |
//!   |     |      |       |
//!   |     x------x       |
//!   |     ------->       |
//!   |    local_strides   |
//!   *--------------------*
//!   --------------------->
//!   global_strides
//!
//! Based on this, there are two views:
//! Global view: This view provides an enumeration(id) of the entities w.r.t
//! the outer box. Particularly,
//! global_offset: Unique offset enumerated by all the boxes in the type.
//! global_box_offset: Offset within a single box.
//! global_box_indices: Indices w.r.t a single box.
//!
//! Local view: This view provides an enumeration(id) of the entities w.r.t
//! the local box. Particularly,
//! local_offset: Unique local offset enumerated by all the boxes.
//! local_box_offset: Local offset within a single box.
//! local_box_indices: Local indices w.r.t a single box.
//!
//! Iterators: The iterators return the global id, instead of the local id.
//!
//! NOTE: Here global vs local nomenclature are not related to
//! distributed vs local partition. This type can be used to represent
//! such distributed local partitions. Here global means the bounds of outer box
//! in which the this inner box is embedded.
//!
//! @param MESH_DIMENSION Dimension of the mesh
//! @param ENTITY_DIMENSION Dimension of the entity represented by this type
//! @param NUM_BOXES Number of boxes required to cover all the entities of
//!                  ENTITY_DIMENSION
//----------------------------------------------------------------------------//

template<size_t MESH_DIMENSION,
  size_t ENTITY_DIMENSION,
  bool BND_TAGGED = false>
class unit_box_u
{
public:
  // using ENTITY = typename std::remove_pointer<ENTITY_TYPE>::type;

  static constexpr size_t NUM_BOXES = binom(MESH_DIMENSION, ENTITY_DIMENSION);

  using id_t = int64_t;
  using id_array_t = id_t[MESH_DIMENSION];
  using id_array_all_t = id_t[MESH_DIMENSION * NUM_BOXES];

  /************************************************************************
   * Constructors/initializers/destructors
   *
   *************************************************************************/
  unit_box_u() {
    for(size_t i = 0; i < MESH_DIMENSION * NUM_BOXES; ++i) {
      lowerbnds_[i] = 0;
      upperbnds_[i] = 0;
      strides_[i] = 0;
    }

    for(size_t i = 0; i < NUM_BOXES; ++i)
      sizes_[i] = 0;

    for(size_t i = 0; i < tagsz * NUM_BOXES; ++i)
      bnd_tags_[i] = false;
  };

  ~unit_box_u(){};

  // This initialize function takes as input the bounds of each box representing
  // this entity as well as the its boundary tags. f
  void initialize(std::vector<size_t> & lowerbnds,
    std::vector<size_t> & upperbnds,
    std::vector<size_t> & strides,
    bool & is_bndtagged,
    std::vector<bool> & bnd_tags,
    bool primary) {
    // Various checks to ensure that expected sized vectors are passed to the
    // initialization method.
    assert(lowerbnds.size() == MESH_DIMENSION * NUM_BOXES);
    assert(upperbnds.size() == MESH_DIMENSION * NUM_BOXES);
    assert(strides.size() == MESH_DIMENSION * NUM_BOXES);

    if(is_bndtagged)
      assert(bnd_tags.size() == tagsz * NUM_BOXES);

    primary_ = primary;

    for(size_t i = 0; i < lowerbnds.size(); i++) {
      lowerbnds_[i] = lowerbnds[i];
      upperbnds_[i] = upperbnds[i];
      strides_[i] = strides[i];
    }

    for(size_t i = 0; i < NUM_BOXES; i++) {
      sizes_[i] = 1;
      for(size_t j = 0; j < MESH_DIMENSION; j++)
        sizes_[i] *= upperbnds_[MESH_DIMENSION * i + j] -
                     lowerbnds_[MESH_DIMENSION * i + j] + 1;
    }

    if(is_bndtagged) {
      is_bndtagged_ = is_bndtagged;
      std::cout << "UB-INIT: btags = [";
      for(size_t i = 0; i < bnd_tags.size(); i++) {
        bnd_tags_[i] = bnd_tags[i];
        std::cout << bnd_tags[i] << " ";
      }
      std::cout << "]\n";
    }

    std::cout << "LBNDS = [";
    for(size_t i = 0; i < lowerbnds.size(); i++)
      std::cout << lowerbnds_[i] << " ";
    std::cout << "]\n";

    std::cout << "UBNDS = [";
    for(size_t i = 0; i < lowerbnds.size(); i++)
      std::cout << upperbnds_[i] << " ";
    std::cout << "]\n";

  } // initialize

  void initialize(std::vector<size_t> & lowerbnds,
    std::vector<size_t> & upperbnds,
    std::vector<size_t> & strides,
    bool & primary) {
    // Various checks to ensure that expected sized vectors are passed to the
    // initialization method.
    assert(lowerbnds.size() == MESH_DIMENSION * NUM_BOXES);
    assert(upperbnds.size() == MESH_DIMENSION * NUM_BOXES);
    assert(strides.size() == MESH_DIMENSION * NUM_BOXES);

    primary_ = primary;

    for(size_t i = 0; i < lowerbnds.size(); i++) {
      lowerbnds_[i] = lowerbnds[i];
      upperbnds_[i] = upperbnds[i];
      strides_[i] = strides[i];
    }

    for(size_t i = 0; i < NUM_BOXES; i++) {
      sizes_[i] = 1;
      for(size_t j = 0; j < MESH_DIMENSION; j++)
        sizes_[i] *= upperbnds_[MESH_DIMENSION * i + j] -
                     lowerbnds_[MESH_DIMENSION * i + j] + 1;
    }
  } // initialize

  /*****************************************************************************
   * Basic query methods: lower/upper bounds, strides, sizes, bounds-checking *
   *****************************************************************************/

  /* Lower bounds */
  auto global_lower_bounds() {
    return lowerbnds_;
  } // global_lower_bounds

  template<size_t BOX_ID>
  void global_lower_bounds(id_array_t & lbnds) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      lbnds[i] = lowerbnds_[MESH_DIMENSION * BOX_ID + i];
  } // global_lower_bounds

  void global_lower_bounds(size_t box_id, id_array_t & lbnds) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      lbnds[i] = lowerbnds_[MESH_DIMENSION * box_id + i];
  } // global_lower_bounds

  template<size_t BOX_ID, size_t DIM>
  auto global_lower_bound() {
    return lowerbnds_[MESH_DIMENSION * BOX_ID + DIM];
  } // global_lower_bound

  auto global_lower_bound(size_t box_id, size_t dim) {
    return lowerbnds_[MESH_DIMENSION * box_id + dim];
  } // global_lower_bound

  template<size_t BOX_ID, size_t DIM>
  auto local_lower_bound() {
    return 0;
  } // local_lower_bound

  auto local_lower_bound(size_t box_id, size_t dim) {
    return 0 * (box_id * dim);
  } // local_lower_bound

  /* Upper bounds */
  auto global_upper_bounds() {
    return upperbnds_;
  } // global_upper_bounds

  template<size_t BOX_ID>
  void global_upper_bounds(id_array_t & ubnds) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      ubnds[i] = upperbnds_[MESH_DIMENSION * BOX_ID + i];
  } // global_upper_bounds

  void global_upper_bounds(size_t box_id, id_array_t & ubnds) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      ubnds[i] = upperbnds_[MESH_DIMENSION * box_id + i];
  } // global_upper_bounds

  template<size_t BOX_ID, size_t DIM>
  auto global_upper_bound() {
    return upperbnds_[MESH_DIMENSION * BOX_ID + DIM];
  } // global_upper_bound

  auto global_upper_bound(size_t box_id, size_t dim) {
    return upperbnds_[MESH_DIMENSION * box_id + dim];
  } // global_upper_bound

  template<size_t BOX_ID, size_t DIM>
  auto local_upper_bound() {
    return (upperbnds_[MESH_DIMENSION * BOX_ID + DIM] -
            lowerbnds_[MESH_DIMENSION * BOX_ID + DIM]);
  } // local_upper_bound

  auto local_upper_bound(size_t box_id, size_t dim) {
    return (upperbnds_[MESH_DIMENSION * box_id + dim] -
            lowerbnds_[MESH_DIMENSION * box_id + dim]);
  } // local_upper_bound

  /* Strides */
  template<size_t BOX_ID, size_t DIM>
  auto global_stride() {
    return strides_[MESH_DIMENSION * BOX_ID + DIM];
  } // global_stride

  auto global_stride(size_t box_id, size_t dim) {
    return strides_[MESH_DIMENSION * box_id + dim];
  } // global_stride

  auto global_strides() {
    return strides_;
  } // global_strides

  template<size_t BOX_ID>
  void global_strides(id_array_t & str) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      str[i] = strides_[MESH_DIMENSION * BOX_ID + i];
  } // global_strides

  void global_strides(size_t box_id, id_array_t & str) {
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      str[i] = strides_[MESH_DIMENSION * box_id + i];
  } // global_strides

  template<size_t BOX_ID, size_t DIM>
  auto local_stride() {
    return local_upper_bound<BOX_ID, DIM>() + 1;
  } // local_stride

  auto local_stride(size_t box_id, size_t dim) {
    return local_upper_bound(box_id, dim) + 1;
  } // local_stride

  void local_strides(id_array_all_t & lstrides) {
    for(size_t i = 0; i < NUM_BOXES; ++i)
      for(size_t j = 0; j < MESH_DIMENSION; ++j)
        lstrides[MESH_DIMENSION * i + j] = local_stride(i, j);
  } // local_strides

  template<size_t BOX_ID>
  void local_strides(id_array_t & str) {
    id_array_all_t lstr;
    local_strides(lstr);
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      str[i] = lstr[MESH_DIMENSION * BOX_ID + i];
  } // local_strides

  void local_strides(size_t box_id, id_array_t & str) {
    id_array_all_t lstr;
    local_strides(lstr);
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      str[i] = lstr[MESH_DIMENSION * box_id + i];
  } // local_strides

  /* Ranges */
  template<size_t BOX_ID>
  auto local_range_low() {
    id_t low = 0;
    for(size_t i = 0; i < BOX_ID; i++)
      low += sizes_[i];

    return low;
  }

  template<size_t BOX_ID>
  auto local_range_up() {
    id_t up = sizes_[0];
    for(size_t i = 0; i < BOX_ID; i++)
      up += sizes_[i + 1];
    return up;
  }

  /* Bounds checking */
  template<size_t BOX_ID, size_t DIM>
  bool check_bounds_global_index(size_t index) {
    return (index >= lowerbnds_[MESH_DIMENSION * BOX_ID + DIM] &&
            index <= upperbnds_[MESH_DIMENSION * BOX_ID + DIM]);
  } // check_bounds_global_index

  bool check_bounds_global_index(size_t box_id, size_t dim, size_t index) {
    return ((index >= lowerbnds_[MESH_DIMENSION * box_id + dim]) &&
            (index <= upperbnds_[MESH_DIMENSION * box_id + dim]));
  } // check_bounds_global_index

  template<size_t BOX_ID>
  bool check_bounds_global_indices(id_array_t global_indices) {
    bool within_bnds = true;
    for(size_t i = 0; i < MESH_DIMENSION; i++) {
      within_bnds =
        within_bnds &&
        ((global_indices[i] >= lowerbnds_[MESH_DIMENSION * BOX_ID + i]) &&
          (global_indices[i] <= upperbnds_[MESH_DIMENSION * BOX_ID + i]));
    }
    return within_bnds;
  } // check_bounds_global_indices

  bool check_bounds_global_indices(size_t box_id, id_array_t global_indices) {
    bool within_bnds = true;
    for(size_t i = 0; i < MESH_DIMENSION; i++) {
      within_bnds =
        within_bnds &&
        ((global_indices[i] >= lowerbnds_[MESH_DIMENSION * box_id + i]) &&
          (global_indices[i] <= upperbnds_[MESH_DIMENSION * box_id + i]));
    }
    return within_bnds;
  } // check_bounds_global_indices

  template<size_t BOX_ID, size_t DIM>
  bool check_bounds_local_index(size_t index) {
    return ((index >= 0) && (index <= local_upper_bound<BOX_ID, DIM>()));
  } // check_bounds_local_index

  bool check_bounds_local_index(size_t box_id, size_t dim, size_t index) {
    return ((index >= 0) && (index <= local_upper_bound(box_id, dim)));
  } // check_bounds_local_index

  template<size_t BOX_ID>
  bool check_bounds_local_indices(id_array_t local_indices) {
    bool within_bnds = true;
    for(size_t i = 0; i < MESH_DIMENSION; i++) {
      within_bnds =
        within_bnds && ((local_indices[i] >= 0) &&
                         (local_indices[i] <= local_upper_bound(BOX_ID, i)));
    }
    return within_bnds;
  } // check_bounds_local_indices

  bool check_bounds_local_indices(size_t box_id, id_array_t local_indices) {
    bool within_bnds = true;
    for(size_t i = 0; i < MESH_DIMENSION; i++) {
      within_bnds =
        within_bnds && ((local_indices[i] >= 0) &&
                         (local_indices[i] <= local_upper_bound(box_id, i)));
    }
    return within_bnds;
  } // check_bounds_local_indices

  /* Sizes */
  template<size_t BOX_ID>
  auto local_size() {
    return sizes_[BOX_ID];
  } // size

  auto local_size(size_t box_id) {
    return sizes_[box_id];
  } // size

  auto local_size() {
    size_t count = 0;
    for(size_t i = 0; i < NUM_BOXES; i++)
      count += sizes_[i];
    return count;
  } // size

  template<size_t BOX_ID>
  auto global_size() {
    size_t count = 1;
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      count *= strides_[MESH_DIMENSION * BOX_ID + i];
    ;
    return count;
  } // global_size

  auto global_size(size_t box_id) {
    size_t count = 1;
    for(size_t i = 0; i < MESH_DIMENSION; i++)
      count *= strides_[MESH_DIMENSION * box_id + i];
    ;
    return count;
  } // global_size

  /* Tags */
  auto boundary_tags() {
    return bnd_tags_;
  }

  /*****************************************************************************
   * Basic query methods: indices <-> offset              		      *
   *****************************************************************************/
  //--------------------------------------------------------------------------//
  //! Global to global: indices <-> offset
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Return the global box offset given a global offset.
  //! @param global_offset Global offset of an entity
  //--------------------------------------------------------------------------//
  auto global_box_offset_from_global_offset(const id_t & global_offset) {
    id_t box_id = 0, box_offset = 0;
    global_box_offset_from_global_offset(global_offset, box_id, box_offset);

    return box_offset;

  } // global_box_offset_from_global_offset

  void global_box_offset_from_global_offset(const id_t & global_offset,
    id_t & box_id,
    id_t & box_offset) {
    box_id = 0;
    if(NUM_BOXES > 1)
      box_id = find_box_id_from_global_offset(global_offset);

    box_offset = global_offset;
    for(id_t i = 0; i < box_id; ++i)
      box_offset -= global_size(i);
  } // global_box_offset_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the global offset given a global box id and offset w.r.t to
  //! the box.
  //! @param global_box_id     id of the global box
  //! @param global_box_offset offset w.r.t the global box
  //--------------------------------------------------------------------------//
  auto global_offset_from_global_box_offset(const id_t & global_box_id,
    const id_t & global_box_offset) {
    size_t value = global_box_offset;
    for(id_t i = 0; i < global_box_id; ++i)
      value += global_size(i);

    return value;
  } // global_offset_from_global_box_offset

  //--------------------------------------------------------------------------//
  //! Return the global box indices given a global box id and offset w.r.t to
  //! the box.
  //! @param global_box_id     id of the global box
  //! @param global_box_offset offset w.r.t the global box
  //--------------------------------------------------------------------------//
  void global_box_indices_from_global_box_offset(const id_t & global_box_id,
    const id_t & global_box_offset,
    id_array_t & indices) {
    id_array_t str;
    global_strides(global_box_id, str);
    compute_indices_from_offset(str, global_box_offset, indices);
  } // global_box_indices_from_global_box_offset

  //--------------------------------------------------------------------------//
  //! Return the global box offset given a global box id and indices w.r.t to
  //! the box.
  //! @param global_box_id      id of the global box
  //! @param global_box_indices indices w.r.t the global box
  //--------------------------------------------------------------------------//
  auto global_box_offset_from_global_box_indices(const id_t & global_box_id,
    const id_array_t & global_box_indices) {
    id_array_t str;
    global_strides(global_box_id, str);
    return compute_offset_from_indices(str, global_box_indices);
  } // global_box_offset_from_global_box_indices

  //--------------------------------------------------------------------------//
  //! Return the global box indices given a global offset.
  //! @param global_offset Global offset of an entity
  //--------------------------------------------------------------------------//
  void global_box_indices_from_global_offset(const id_t & global_offset,
    id_array_t & indices) {
    id_t box_id = 0, offset;
    global_box_offset_from_global_offset(global_offset, box_id, offset);
    global_box_indices_from_global_box_offset(box_id, offset, indices);
  } // global_box_indices_from_global_offset

  void global_box_indices_from_global_offset(const id_t & global_offset,
    id_t & box_id,
    id_array_t & indices) {
    id_t offset;
    global_box_offset_from_global_offset(global_offset, box_id, offset);
    global_box_indices_from_global_box_offset(box_id, offset, indices);
  } // global_box_indices_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the global offset given a global box id and indices w.r.t to
  //! the box.
  //! @param global_box_id      id of the global box
  //! @param global_box_indices indices w.r.t the global box
  //--------------------------------------------------------------------------//
  auto global_offset_from_global_box_indices(const id_t & global_box_id,
    const id_array_t & global_box_indices) {
    id_t offset = global_box_offset_from_global_box_indices(
      global_box_id, global_box_indices);
    return global_offset_from_global_box_offset(global_box_id, offset);
  } // global_offset_from_global_box_indices

  //--------------------------------------------------------------------------//
  //! Local to local
  //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//
  //! Return the local box offset given a local offset
  //! @param local_offset  local offset of the entity
  //--------------------------------------------------------------------------//
  auto local_box_offset_from_local_offset(const id_t & local_offset) {
    id_t box_id = 0, box_offset = 0;
    local_box_offset_from_local_offset(local_offset, box_id, box_offset);

    return box_offset;

  } // local_box_offset_from_local_offset

  void local_box_offset_from_local_offset(const id_t & local_offset,
    id_t & box_id,
    id_t & box_offset) {
    box_id = 0;
    if(NUM_BOXES > 1)
      box_id = find_box_id_from_local_offset(local_offset);

    box_offset = local_offset;
    for(id_t i = 0; i < box_id; ++i)
      box_offset -= local_size(i);
  } // local_box_offset_from_local_offset

  //--------------------------------------------------------------------------//
  //! Return the local offset given a local box id and offset w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_offset offset w.r.t the local box
  //--------------------------------------------------------------------------//
  auto local_offset_from_local_box_offset(const id_t & local_box_id,
    const id_t & local_box_offset) {
    id_t value = local_box_offset;
    for(size_t i = 0; i < local_box_id; ++i)
      value += local_size(i);

    return value;
  } // local_offset_from_local_box_offset

  //--------------------------------------------------------------------------//
  //! Return the local box offset given a local box id and offset w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_offset offset w.r.t the local box
  //--------------------------------------------------------------------------//
  void local_box_indices_from_local_box_offset(const id_t local_box_id,
    const id_t & local_box_offset,
    id_array_t & indices) {
    id_array_t str;
    local_strides(local_box_id, str);
    compute_indices_from_offset(str, local_box_offset, indices);
  } // local_box_indices_from_local_box_offset

  //--------------------------------------------------------------------------//
  //! Return the local box offset given a local box id and indices w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_indices indices w.r.t the local box
  //--------------------------------------------------------------------------//
  auto local_box_offset_from_local_box_indices(const id_t & local_box_id,
    const id_array_t & local_box_indices) {
    id_array_t str;
    local_strides(local_box_id, str);
    return compute_offset_from_indices(str, local_box_indices);
  } // local_box_offset_from_local_box_indices

  //--------------------------------------------------------------------------//
  //! Return the local box indices given a local offset.
  //! @param local_offset local offset of an entity
  //--------------------------------------------------------------------------//
  void local_box_indices_from_local_offset(const id_t & local_offset,
    id_array_t & indices) {
    id_t box_id = 0, offset;
    local_box_offset_from_local_offset(local_offset, box_id, offset);
    local_box_indices_from_local_box_offset(box_id, offset, indices);
  } // local_box_indices_from_local_offset

  auto local_box_indices_from_local_offset(const id_t & local_offset,
    id_t & box_id,
    id_array_t & indices) {
    id_t offset;
    local_box_offset_from_local_offset(local_offset, box_id, offset);
    local_box_indices_from_local_box_offset(box_id, offset, indices);
  } // local_box_indices_from_local_offset

  //--------------------------------------------------------------------------//
  //! Return the local offset given a local box id and indices w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_indices indices w.r.t the local box
  //--------------------------------------------------------------------------//
  auto local_offset_from_local_box_indices(const id_t & local_box_id,
    const id_array_t & local_box_indices) {
    id_t offset =
      local_box_offset_from_local_box_indices(local_box_id, local_box_indices);
    return local_offset_from_local_box_offset(local_box_id, offset);
  } // local_offset_from_local_box_indices

  //--------------------------------------------------------------------------//
  //! Global to local and local to global
  //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//
  //! Return the global box indices given a local box id and indices w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_indices indices w.r.t the local box
  //--------------------------------------------------------------------------//
  void global_box_indices_from_local_box_indices(const id_t & local_box_id,
    const id_array_t & local_box_indices,
    id_array_t & indices) {
    // id_array_t id;
    for(size_t i = 0; i < MESH_DIMENSION; ++i)
      indices[i] =
        local_box_indices[i] + lowerbnds_[MESH_DIMENSION * local_box_id + i];
    // return id;

  } // global_box_indices_from_local_box_indices

  //--------------------------------------------------------------------------//
  //! Return the local box indices given a global box id and indices w.r.t to
  //! the box.
  //! @param global_box_id      id of the global box
  //! @param global_box_indices indices w.r.t the global box
  //--------------------------------------------------------------------------//
  void local_box_indices_from_global_box_indices(const id_t & global_box_id,
    const id_array_t & global_box_indices,
    id_array_t & indices) {
    // id_array_t id;
    for(size_t i = 0; i < MESH_DIMENSION; ++i)
      indices[i] =
        global_box_indices[i] - lowerbnds_[MESH_DIMENSION * global_box_id + i];
    // return id;
  } // local_box_indices_from_global_box_indices

  //--------------------------------------------------------------------------//
  //! Return the global box offset given a local box id and offset w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_offset offset w.r.t the local box
  //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_box_offset(const id_t & local_box_id,
    const id_t & local_box_offset) {
    id_array_t id;
    global_box_indices_from_local_box_offset(
      local_box_id, local_box_offset, id);
    return global_box_offset_from_global_box_indices(local_box_id, id);
  } // global_box_offset_from_local_box_offset

  //--------------------------------------------------------------------------//
  //! Return the local box offset given a global box id and offset w.r.t to
  //! the box.
  //! @param global_box_id     id of the global box
  //! @param global_box_offset offset w.r.t the global box
  //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_box_offset(const id_t & global_box_id,
    const id_t & global_box_offset) {
    id_array_t id;
    local_box_indices_from_global_box_offset(
      global_box_id, global_box_offset, id);
    return local_box_offset_from_local_box_indices(global_box_id, id);
  } // local_box_offset_from_global_box_offset

  //--------------------------------------------------------------------------//
  //! Return the global offset given a local offset
  //! @param local_offset  local offset of the entity
  //--------------------------------------------------------------------------//
  auto global_offset_from_local_offset(const id_t & local_offset) {
    id_t box_id;
    id_array_t id;
    local_box_indices_from_local_offset(local_offset, box_id, id);

    return global_offset_from_local_box_indices(box_id, id);

  } // global_offset_from_local_offset

  //--------------------------------------------------------------------------//
  //! From global_offset to local_offset, local_box_offset, local_box_indices,
  //! global_box_offset, global_box_indices
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Return the local offset given a global offset.
  //! @param global_offset Global offset of an entity
  //--------------------------------------------------------------------------//
  auto local_offset_from_global_offset(const id_t & global_offset) {
    id_t box_id;
    id_array_t id;
    global_box_indices_from_global_offset(global_offset, box_id, id);
    return local_offset_from_global_box_indices(box_id, id);
  } // local_offset_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the global box indices given a local box id and offset w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_offset offset w.r.t the local box
  //--------------------------------------------------------------------------//
  void global_box_indices_from_local_box_offset(const id_t & local_box_id,
    const id_t & local_box_offset,
    id_array_t & indices) {
    id_array_t id;
    local_box_indices_from_local_box_offset(local_box_id, local_box_offset, id);
    global_box_indices_from_local_box_indices(local_box_id, id, indices);
  } // global_box_indices_from_local_box_offset

  //--------------------------------------------------------------------------//
  //! Return the local box offset given a global box id and indices w.r.t to
  //! the box.
  //! @param global_box_id      id of the global box
  //! @param global_box_indices indices w.r.t the global box
  //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_box_indices(const id_t & global_box_id,
    const id_array_t & global_box_indices) {
    id_array_t id;
    local_box_indices_from_global_box_indices(
      global_box_id, global_box_indices, id);
    return local_box_offset_from_local_box_indices(global_box_id, id);
  } // local_box_offset_from_global_box_indices

  //--------------------------------------------------------------------------//
  //! Return the global box offset given a local box id and indices w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_indices indices w.r.t the local box
  //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_box_indices(const id_t & local_box_id,
    const id_array_t & local_box_indices) {
    id_array_t id;
    global_box_indices_from_local_box_indices(
      local_box_id, local_box_indices, id);
    return global_box_offset_from_global_box_indices(local_box_id, id);
  } // global_box_offset_from_local_box_indices

  //--------------------------------------------------------------------------//
  //! Return the local box indices given a global box id and offset w.r.t to
  //! the box.
  //! @param global_box_id     id of the global box
  //! @param global_box_offset offset w.r.t the global box
  //--------------------------------------------------------------------------//
  void local_box_indices_from_global_box_offset(const id_t & global_box_id,
    const id_t & global_box_offset,
    id_array_t & indices) {
    id_array_t id;
    global_box_indices_from_global_box_offset(
      global_box_id, global_box_offset, id);
    local_box_indices_from_global_box_indices(global_box_id, id, indices);
  } // local_box_indices_from_global_box_offset

  //--------------------------------------------------------------------------//
  //! Return the global box offset given a local offset
  //! @param local_offset  local offset of the entity
  //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_offset(const id_t & local_offset) {
    id_t box_id;
    id_array_t id;
    local_box_indices_from_local_offset(local_offset, box_id, id);
    return global_box_offset_from_local_box_indices(box_id, id);

  } // global_box_offset_from_local_offset

  //--------------------------------------------------------------------------//
  //! Return the local offset given a global box id and offset w.r.t to
  //! the box.
  //! @param global_box_id     id of the global box
  //! @param global_box_offset offset w.r.t the global box
  //--------------------------------------------------------------------------//
  auto local_offset_from_global_box_offset(const id_t & global_box_id,
    const id_t & global_box_offset) {
    id_array_t id;
    local_box_indices_from_global_box_offset(
      global_box_id, global_box_offset, id);
    return local_offset_from_local_box_indices(global_box_id, id);

  } // local_offset_from_global_box_offset

  //--------------------------------------------------------------------------//
  //! Return the global offset given a local box id and offset w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_offset offset w.r.t the local box
  //--------------------------------------------------------------------------//
  auto global_offset_from_local_box_offset(const id_t & local_box_id,
    const id_t & local_box_offset) {
    id_array_t id;
    global_box_indices_from_local_box_offset(
      local_box_id, local_box_offset, id);
    return global_offset_from_global_box_indices(local_box_id, id);

  } // global_offset_from_local_box_offset

  //--------------------------------------------------------------------------//
  //! Return the local box offset given a global offset.
  //! @param global_offset Global offset of an entity
  //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_offset(const id_t & global_offset) {
    id_t box_id;
    id_array_t id;
    global_box_indices_from_global_offset(global_offset, box_id, id);
    return local_box_offset_from_global_box_indices(box_id, id);
  } // local_box_offset_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the global box indices given a local offset
  //! @param local_offset  local offset of the entity
  //--------------------------------------------------------------------------//
  void global_box_indices_from_local_offset(const id_t & local_offset,
    id_array_t & indices) {
    id_t box_id;
    id_array_t id;
    local_box_indices_from_local_offset(local_offset, box_id, id);
    global_box_indices_from_local_box_indices(box_id, id, indices);
  } // global_box_indices_from_local_offset

  //! From local_box_offset to local_offset, local_box_indices, global_offset,
  //! global_box_offset, global_box_indices
  //

  //--------------------------------------------------------------------------//
  //! Return the local offset given a global box id and indices w.r.t to
  //! the box.
  //! @param global_box_id      id of the global box
  //! @param global_box_indices indices w.r.t the global box
  //--------------------------------------------------------------------------//
  auto local_offset_from_global_box_indices(const id_t & global_box_id,
    const id_array_t & global_box_indices) {
    id_array_t id;
    local_box_indices_from_global_box_indices(
      global_box_id, global_box_indices, id);
    return local_offset_from_local_box_indices(global_box_id, id);
  } // local_offset_from_global_box_indices

  //--------------------------------------------------------------------------//
  //! Return the global offset given a local box id and indices w.r.t to
  //! the box.
  //! @param local_box_id      id of the local box
  //! @param local_box_indices indices w.r.t the local box
  //--------------------------------------------------------------------------//
  auto global_offset_from_local_box_indices(const id_t & local_box_id,
    const id_array_t & local_box_indices) {
    id_array_t id;
    global_box_indices_from_local_box_indices(
      local_box_id, local_box_indices, id);

    return global_offset_from_global_box_indices(local_box_id, id);
  } // global_offset_from_local_box_indices

  //--------------------------------------------------------------------------//
  //! Return the local box indices given a global offset.
  //! @param global_offset Global offset of an entity
  //--------------------------------------------------------------------------//
  void local_box_indices_from_global_offset(const id_t & global_offset,
    id_array_t & indices) {
    id_t box_id;
    id_array_t id;
    global_box_indices_from_global_offset(global_offset, box_id, id);
    local_box_indices_from_global_box_indices(box_id, id, indices);
  } // local_box_indices_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the id of the global box which the query entity, of a specified
  //! topological dimension and domain, is part of.
  //!
  //! @param global_offset   global offset of the entity
  //--------------------------------------------------------------------------//
  auto find_box_id_from_global_offset(id_t global_offset) {
    id_array_t str_all = global_strides();
    size_t bid = 0, low = 0, up = 0;

    for(size_t i = 0; i < NUM_BOXES; ++i) {
      size_t count = 1;
      for(size_t j = 0; j < MESH_DIMENSION; ++j)
        count *= str_all[MESH_DIMENSION * i + j];
      up += count;

      if((global_offset >= low) && (global_offset < up)) {
        bid = i;
        break;
      }
      low = up;
    }

    return bid;
  } // find_box_id_from_global_offset

  //--------------------------------------------------------------------------//
  //! Return the id of the local box which the query entity, of a specified
  //! topological dimension and domain, is part of.
  //!
  //! @param local_offset   local offset of the entity
  //--------------------------------------------------------------------------//
  auto find_box_id_from_local_offset(id_t local_offset) {
    id_array_all_t str_all;
    local_strides(str_all);
    id_t bid = 0, low = 0, up = 0;

    for(size_t i = 0; i < NUM_BOXES; ++i) {
      size_t count = 1;
      for(size_t j = 0; j < MESH_DIMENSION; ++j)
        count *= str_all[MESH_DIMENSION * i + j];
      up += count;

      if((local_offset >= low) && (local_offset < up)) {
        bid = i;
        break;
      }
      low = up;
    }

    return bid;
  } // find_box_id_from_local_offset

  /************************************************************************
   * Iterators
   *************************************************************************/
  template<class ENTITY,
    size_t BOX_ID = NUM_BOXES + 1,
    bool BOUNDARY_TAG = BND_TAGGED>
  auto begin() {
    id_t start = (BOX_ID == NUM_BOXES + 1) ? 0 : local_range_low<BOX_ID>();

    return iterator_u<ENTITY,
      unit_box_u<MESH_DIMENSION, ENTITY::dimension, BND_TAGGED>,
      BOX_ID,
      BOUNDARY_TAG>(this, start, true);
  };

  template<class ENTITY,
    size_t BOX_ID = NUM_BOXES + 1,
    bool BOUNDARY_TAG = BND_TAGGED>
  auto end() {

    id_t end =
      (BOX_ID == NUM_BOXES + 1) ? local_size() : local_range_up<BOX_ID>();

    return iterator_u<ENTITY,
      unit_box_u<MESH_DIMENSION, ENTITY::dimension, BND_TAGGED>,
      BOX_ID,
      BOUNDARY_TAG>(this, end, false);
  };

  template<class ENTITY,
    class UB,
    size_t BOX_ID = NUM_BOXES + 1,
    bool BOUNDARY_TAG = BND_TAGGED>
  class iterator_u
  {
  public:
    iterator_u(UB * ub, id_t current, bool forward)
      : ubox_{ub}, current_{current}, forward_{forward} {
      // if (BOUNDARY_TAG)
      //  ubox_->get_valid_local_offset(BOX_ID, current_, forward_);

      entity_.set_id(current_);
      id_t val = ubox_->global_offset_from_local_offset(current_);
      entity_.set_global_id(val);
    };

    ~iterator_u(){};

    iterator_u & operator++() {
      ++current_;

      // if (BOUNDARY_TAG)
      //  ubox_->get_valid_local_offset(BOX_ID, current_, forward_);

      entity_.set_id(current_);
      entity_.set_global_id(ubox_->global_offset_from_local_offset(current_));
      return *this;
    };

    iterator_u & operator--() {
      --current_;

      // if (BOUNDARY_TAG)
      //  ubox_->get_valid_local_offset(BOX_ID, current_, forward_);

      entity_.set_id(current_);
      entity_.set_global_id(ubox_->global_offset_from_local_offset(current_));
    };

    bool operator!=(const iterator_u & rhs) {
      return (this->current_ != rhs.current());
    };

    bool operator==(const iterator_u & rhs) {
      return (this->current_ == rhs.current());
    }

    bool operator<(const iterator_u & rhs) {
      return (this->current_ < rhs.current());
    };

    bool operator>(const iterator_u & rhs) {
      return (this->current_ > rhs.current());
    };

    bool operator<=(const iterator_u & rhs) {
      return (this->current_ <= rhs.current());
    };

    bool operator>=(const iterator_u & rhs) {
      return (this->current_ >= rhs.current());
    };

    ENTITY & operator*() {
      return entity_;
    }

    ENTITY & entity() {
      return entity_;
    }

    id_t current() const {
      return current_;
    }

  private:
    UB * ubox_;
    id_t current_;
    ENTITY entity_;
    bool forward_;
  };

  // Iterator with out entity type
  template<size_t BOX_ID = NUM_BOXES + 1, bool BOUNDARY_TAG = BND_TAGGED>
  auto begin() {
    id_t start = (BOX_ID == NUM_BOXES + 1) ? 0 : local_range_low<BOX_ID>();

    return iterator_<unit_box_u<MESH_DIMENSION, ENTITY_DIMENSION, BND_TAGGED>,
      BOX_ID,
      BOUNDARY_TAG>(this, start, true);
  };

  template<size_t BOX_ID = NUM_BOXES + 1, bool BOUNDARY_TAG = BND_TAGGED>
  auto end() {

    id_t end =
      (BOX_ID == NUM_BOXES + 1) ? local_size() : local_range_up<BOX_ID>();

    return iterator_<unit_box_u<MESH_DIMENSION, ENTITY_DIMENSION, BND_TAGGED>,
      BOX_ID,
      BOUNDARY_TAG>(this, end, false);
  };

  template<class UB,
    size_t BOX_ID = NUM_BOXES + 1,
    bool BOUNDARY_TAG = BND_TAGGED>
  class iterator_
  {
  public:
    iterator_(UB * ub, id_t current, bool forward)
      : ubox_{ub}, current_local_{current}, forward_{forward} {
      // std::cout<<"Before: lid = "<<current_local_<<", gid =
      // "<<current_global_<<std::endl;
      if(BOUNDARY_TAG && (!ubox_->primary_))
        current_local_ = ubox_->find_valid_id(current_local_);

      current_global_ = ubox_->global_offset_from_local_offset(current_local_);
      // std::cout<<"After: lid = "<<current_local_<<", gid =
      // "<<current_global_<<std::endl;
    };

    ~iterator_(){};

    iterator_ & operator++() {
      std::cout << "Before: lid = " << current_local_
                << ", gid = " << current_global_ << std::endl;
      ++current_local_;

      if(BOUNDARY_TAG && (!ubox_->primary_))
        current_local_ = ubox_->find_valid_id(current_local_);

      current_global_ = ubox_->global_offset_from_local_offset(current_local_);
      std::cout << "After: lid = " << current_local_
                << ", gid = " << current_global_ << std::endl;

      return *this;
    };

    iterator_ & operator--() {
      --current_local_;

      if(BOUNDARY_TAG && (!ubox_->primary_))
        current_local_ = ubox_->find_valid_id(current_local_);

      current_global_ = ubox_->global_offset_from_local_offset(current_local_);
    };

    bool operator!=(const iterator_ & rhs) {
      return (this->current_local_ != rhs.current_local());
    };

    bool operator==(const iterator_ & rhs) {
      return (this->current_local_ == rhs.current_local());
    }

    bool operator<(const iterator_ & rhs) {
      return (this->current_local_ < rhs.current_local());
    };

    bool operator>(const iterator_ & rhs) {
      return (this->current_local_ > rhs.current_local());
    };

    bool operator<=(const iterator_ & rhs) {
      return (this->current_local_ <= rhs.current_local());
    };

    bool operator>=(const iterator_ & rhs) {
      return (this->current_local_ >= rhs.current_local());
    };

    id_t & operator*() {
      return current_global_;
    }

    id_t current_local() const {
      return current_local_;
    }

  private:
    UB * ubox_;
    id_t current_local_;
    id_t current_global_;
    bool forward_;
  };

  /************************************************************************
   * Stencil Queries
   *************************************************************************/
  //--------------------------------------------------------------------------//
  //! Return the id of the global box which the query entity, of a specified
  //! topological dimension and domain, is part of.
  //!
  //! @param e   Query entity
  //! @param stencil_offset This array specifies the local offsets along each
  //!                       direction assuming that the current entity is the
  //! 			   central offset with value 0.
  //--------------------------------------------------------------------------//
  template<class ENTITY>
  auto stencil(ENTITY * e, size_t & box_id, id_array_t & stencil_offset) {
    auto lid = e->template id();
    id_array_t lindices;
    local_box_indices_from_local_offset(lid, lindices);
    id_array_t new_indices = lindices + stencil_offset;

    if(check_bounds_local_indices(box_id, new_indices))
      return local_offset_from_local_box_indices(box_id, new_indices);
  } // stencil

  template<class ENTITY>
  auto stencils(ENTITY * e,
    size_t & box_id,
    std::vector<id_array_t> & stencil_offsets) {
    auto lid = e->template id();
    id_array_t lindices;
    local_box_indices_from_local_offset(lid, lindices);

    id_array_t new_indices;
    std::vector<id_t> ent_ids(stencil_offsets.size());

    for(size_t i = 0; i < stencil_offsets.size(); ++i) {
      new_indices = lindices + stencil_offsets[i];
      if(check_bounds_local_indices(box_id, new_indices))
        ent_ids[i] = local_offset_from_local_box_indices(box_id, new_indices);
      else
        ent_ids[i] = -1;
    }

    return ent_ids;
  } // stencil

  auto find_valid_id(id_t & local_id) {
    id_t valid_id = local_id;
    bool valid = false;
    id_t box_id;
    id_array_t indices;

    auto map = dim2bounds(MESH_DIMENSION, ENTITY_DIMENSION);

    while(!valid) {

      // get box_id and indices w.r.t the box
      local_box_indices_from_local_offset(valid_id, box_id, indices);
      std::cout << "indices = [" << indices[0] << ", " << indices[1] << "]"
                << std::endl;

      // get list of bids this box is incident on
      auto bid_list = map[box_id]; // the bid list starts from MESH_DIMENSION
      int bid = -1;

      for(size_t b = MESH_DIMENSION; b < bid_list.size(); ++b) {
        bool on_bid = true;

        for(size_t d = 0; d < MESH_DIMENSION; ++d) {
          auto bid2dir_map = bid2dir(MESH_DIMENSION, d);
          int lbnd = static_cast<int>(local_lower_bound(box_id, d));
          int ubnd = static_cast<int>(local_upper_bound(box_id, d));

          std::cout << " b = " << b << ": d = " << d << ": lbnd = " << lbnd
                    << ": ubnd = " << ubnd
                    << ": b2d = " << bid2dir_map[bid_list[b]] << std::endl;

          if(bid2dir_map[bid_list[b]] == 0) {
            on_bid = on_bid && (indices[d] == lbnd);
          }
          else if(bid2dir_map[bid_list[b]] == 1) {
            on_bid = on_bid && (indices[d] == ubnd);
          }
          else {
            on_bid = on_bid && (indices[d] >= lbnd) && (indices[d] <= ubnd);
            // on_bid = on_bid && (indices[d] > lbnd) && (indices[d] < ubnd);
          }
        } // for

        if(on_bid) {
          bid = bid_list[b];
          std::cout << "bid = " << bid << std::endl;
          break;
        }
      } // loop over bid list

      if(bid == -1)
        valid = true;
      else {
        if(!bnd_tags_[tagsz * box_id + bid])
          ++valid_id;
        else
          valid = true;
      }
    } // while

    return valid_id;
  } // find_valid_id

private:
  // Utility methods
  auto compute_offset_from_indices(id_array_t & strides, id_array_t & indices) {
    id_t value = 0;
    compute_offset_from_indices(strides, indices, value);
    return value;
  } // compute_offset_from_indices

  auto compute_offset_from_indices(const id_array_t & strides,
    const id_array_t & indices) {
    id_t value = 0;
    compute_offset_from_indices(strides, indices, value);
    return value;
  } // compute_offset_from_indices

  void compute_offset_from_indices(id_array_t & strides,
    id_array_t & indices,
    id_t & value) {
    size_t factor;
    for(size_t i = 0; i < MESH_DIMENSION; ++i) {
      factor = 1;
      for(size_t j = 0; j < MESH_DIMENSION - i - 1; ++j)
        factor *= strides[j];
      value += indices[MESH_DIMENSION - i - 1] * factor;
    }
  } // compute_offset_from_indices

  void compute_offset_from_indices(const id_array_t & strides,
    const id_array_t & indices,
    id_t & value) {
    size_t factor;
    for(size_t i = 0; i < MESH_DIMENSION; ++i) {
      factor = 1;
      for(size_t j = 0; j < MESH_DIMENSION - i - 1; ++j)
        factor *= strides[j];
      value += indices[MESH_DIMENSION - i - 1] * factor;
    }
  } // compute_offset_from_indices

  void compute_indices_from_offset(id_array_t & strides,
    id_t & offset,
    id_array_t & indices) {
    size_t factor, value;
    id_t rem = offset;
    for(size_t i = 0; i < MESH_DIMENSION; ++i) {
      factor = 1;
      for(size_t j = 0; j < MESH_DIMENSION - i - 1; ++j)
        factor *= strides[j];
      value = rem / factor;
      indices[MESH_DIMENSION - i - 1] = value;
      rem -= value * factor;
    }
  } // compute_indices_from_offset

  void compute_indices_from_offset(const id_array_t & strides,
    const id_t & offset,
    id_array_t & indices) {
    size_t factor, value;
    id_t rem = offset;
    for(size_t i = 0; i < MESH_DIMENSION; ++i) {
      factor = 1;
      for(size_t j = 0; j < MESH_DIMENSION - i - 1; ++j)
        factor *= strides[j];
      value = rem / factor;
      indices[MESH_DIMENSION - i - 1] = value;
      rem -= value * factor;
    }
  } // compute_indices_from_offset

  // state
  size_t lowerbnds_[MESH_DIMENSION * NUM_BOXES];
  size_t upperbnds_[MESH_DIMENSION * NUM_BOXES];
  size_t strides_[MESH_DIMENSION * NUM_BOXES];
  size_t sizes_[NUM_BOXES];

  static constexpr size_t tagsz = power(3, MESH_DIMENSION);
  bool is_bndtagged_ = false;
  bool bnd_tags_[tagsz * NUM_BOXES];

  bool primary_;
}; // unit_box_u

} // namespace structured_impl
} // namespace topology
} // namespace flecsi
#endif
