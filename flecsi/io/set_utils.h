/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_set_utils_h
#define flecsi_io_set_utils_h

#include <vector>

#include "flecsi/io/mesh_definition.h"

///
// \file set_utils.h
// \authors bergen
// \date Initial file creation: Nov 21, 2016
///

namespace flecsi {
namespace io {

///
//
///
inline
std::set<size_t>
set_intersection(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::vector<size_t> ivec;

  std::set_intersection(s1.begin(), s1.end(),
    s2.begin(), s2.end(), std::back_inserter(ivec));

  std::set<size_t> intersection(ivec.begin(), ivec.end());

  return intersection;
} // set_intersection

///
//
///
inline
std::set<size_t>
set_union(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::vector<size_t> uvec;

  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::back_inserter(uvec));

  std::set<size_t> sunion(uvec.begin(), uvec.end());

  return sunion;
} // set_union

///
//
///
inline
std::set<size_t>
set_difference(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::vector<size_t> vdiff;

  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(vdiff, vdiff.begin()));

  std::set<size_t> difference(vdiff.begin(), vdiff.end());

  return difference;
} // set_difference

///
// Return the neighboring cells for the given cell id.
//
// \param md The mesh_definition_t object to query for mesh information.
// \param id The id of the cell.
// \param intersections The number of intersections that constitute a
//                      neighboring cell.
///
std::set<size_t>
cell_neighbors(
  mesh_definition_t & md,
  size_t id,
  size_t intersections
)
{
  // Get the vertex of the input cell
  auto vvec = md.vertices(id);

  // Intersection needs sorted inputs
  std::sort(vvec.begin(), vvec.end());

  std::vector<size_t> nvec;

  // Iterate over cells computing vertex intersections
  // to create neighbor list
  for(size_t cell(0); cell<md.num_cells(); ++cell) {

    // Skip this cell
    if(cell == id) {
      continue;
    } // if

    // Get the vertex ids of current cell
    auto cvec = md.vertices(cell);

    // Intersection needs sorted inputs
    std::sort(cvec.begin(), cvec.end());

    std::vector<size_t> vint;
    std::set_intersection(vvec.begin(), vvec.end(),
      cvec.begin(), cvec.end(), std::back_inserter(vint));

    // Add this cell id if the intersection shares at least
    // intersections vertices
    if(vint.size() >= intersections) {
      nvec.push_back(cell);
    } // if
  } // for
  
  // Put the results into set form
  std::set<size_t> neighbors(nvec.begin(), nvec.end());

  return neighbors;
} // cell_neighbors

///
// Return the dependency closure of the given set.
//
// \param md The mesh_definition_t object to query for mesh information.
// \param indices The cell indeces of the initial set.
// \param intersections The number of intersections that constitute a
//                      neighboring cell.
///
std::set<size_t>
cell_closure(
  mesh_definition_t & md,
  std::set<size_t> indices,
  size_t intersections
)
{
  std::vector<size_t> cvec;

  // Closure should include the initial set
  std::set<size_t> closure = indices;

  // Iterate over the cell indices and add all neighbors
  for(auto i: indices) {
    auto ncurr = cell_neighbors(md, i, intersections);

    closure = flecsi::io::set_union(ncurr, closure);
  } // for

  return closure;
} // cell_closure

///
// Return the cells that reference the given vertex id.
//
// \param md The mesh_definition_t object to query for mesh information.
// \param id The id of the vertex.
///
std::set<size_t>
vertex_referencers(
  mesh_definition_t & md,
  size_t id
)
{
  std::set<size_t> referencers;

  // Iterate over cells adding any cells that contain
  // the vertex id to the set.
  for(size_t cell(0); cell<md.num_cells(); ++cell) {

    // Get the vertex ids of current cell
    auto cvec = md.vertices(cell);
    std::set<size_t> cset(cvec.begin(), cvec.end());

    // If the cell references this vertex add it
    if(cset.find(id) != cset.end()) {
      referencers.insert(cell);
    } // if
  } // for

  return referencers;
} // vertex_referencers

///
// Return the set of vertices that are referenced by the cells in indices.
//
// \param md The mesh_definition_t object to query for mesh information.
// \param indices The cell indeces.
///
std::set<size_t>
vertex_closure(
  mesh_definition_t & md,
  std::set<size_t> indices
)
{
  std::set<size_t> closure;

  // Iterate over the cells in indices and add any vertices that are
  // referenced by one of the cell indices
  for(auto i: indices) {
    auto vvec = md.vertices(i);
    std::set<size_t> vset(vvec.begin(), vvec.end());

    closure = flecsi::io::set_union(closure, vset);
  } // for

  return closure;
} // vertex_closure

} // namespace io
} // namespace flecsi

#endif // flecsi_io_set_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
