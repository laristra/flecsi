/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_index_partition_h
#define flecsi_dmp_index_partition_h

///
// \file index_partition.h
// \authors bergen
// \date Initial file creation: Aug 17, 2016
///

namespace flecsi {
namespace dmp {

///
// \class partition__ index_partition.h
// \brief partition__ provides...
///
template<typename T>
struct index_partition__
{
  using identifier_t = T;

  std::vector<identifier_t> independent;
  std::vector<identifier_t> shared;
  std::vector<identifier_t> ghost;
}; // class partition__

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_index_partition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
