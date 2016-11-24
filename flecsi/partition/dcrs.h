/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_dcrs_h
#define flecsi_dmp_dcrs_h

#include <vector>

///
// \file dcrs.h
// \authors bergen
// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
// Convenience macro to avoid having to reimplement this for each member.
///
#define define_dcrs_as(member)                                                 \
  template<                                                                    \
    typename T                                                                 \
  >                                                                            \
  std::vector<T>                                                               \
  member ## _as()                                                              \
  {                                                                            \
    std::vector<T> tmp(member.begin(), member.end());                  \
  } // member ## _as

struct dcrs_t
{
  define_dcrs_as(offsets)
  define_dcrs_as(indices)
  define_dcrs_as(distribution)

  std::vector<size_t> offsets;
  std::vector<size_t> indices;
  std::vector<size_t> distribution;
}; // struct dcrs_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_dcrs_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
