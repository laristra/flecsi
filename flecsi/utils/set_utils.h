/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_set_utils_h
#define flecsi_utils_set_utils_h

#include <algorithm>
#include <set>

///
/// \file set_utils.h
/// \authors bergen
/// \date Initial file creation: Nov 21, 2016
///

namespace flecsi {
namespace utils {

///
/// Convenience function wrapper around std::set_intersection function. This
/// version computes the intersection of two sets and returns the result
/// as a set.
///
/// \param s1 The first set of the intersection.
/// \param s2 The second set of the intersection.
///
/// \return A set containing the intersection of s1 with s2.
///
inline
std::set<size_t>
set_intersection(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::set<size_t> intersection;

  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(intersection, intersection.begin()));

  return intersection;
} // set_intersection

///
/// Convenience function wrapper around std::set_union function. This
/// version computes the union of two sets and returns the result
/// as a set.
///
/// \param s1 The first set of the union.
/// \param s2 The second set of the union.
///
/// \return A set containing the union of s1 with s2.
///
inline
std::set<size_t>
set_union(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::set<size_t> sunion;

  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(sunion, sunion.begin()));

  return sunion;
} // set_union

///
/// Convenience function wrapper around std::set_difference function. This
/// version computes the difference of two sets and returns the result
/// as a set.
///
/// \param s1 The first set of the difference.
/// \param s2 The second set of the difference.
///
/// \return A set containing the difference of s1 with s2.
///
inline
std::set<size_t>
set_difference(
  std::set<size_t> s1,
  std::set<size_t> s2
)
{
  std::set<size_t> difference;

  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(difference, difference.begin()));

  return difference;
} // set_difference

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_set_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
