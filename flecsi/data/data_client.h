/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_client_h
#define flecsi_data_client_h

#include "flecsi/utils/common.h"
#include "flecsi/data/data_constants.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/utils/const_string.h"
 
///
/// \file
/// \date Initial file creation: Mar 23, 2016
///

namespace flecsi {
namespace data {

///
/// \class data_client_t data_client.h
/// \brief data_client_t provides...
///
class data_client_t
{
public:

  /// Default constructor
  data_client_t() : id_(utils::unique_id_t<size_t>::instance().next()) {}

  /// Copy constructor (disabled)
  data_client_t(const data_client_t &) = delete;

  /// Assignment operator (disabled)
  data_client_t & operator = (const data_client_t &) = delete;

  /// Allow move construction
  data_client_t(data_client_t && o);

  /// Allow move construction
  data_client_t & operator=(data_client_t && o);

  /// Destructor
  virtual ~data_client_t();

  ///
  /// Return a unique runtime identifier for namespace access to the
  /// data manager.
  ///
  uintptr_t
  runtime_id() const
  {
    return (reinterpret_cast<uintptr_t>(this) << 4) ^ id_;
  } // runtime_id

  // FIXME: This needs to be made pure virtual
  // The current virtual implementation is here to avoid
  // breaking the old data model.
  //virtual size_t indices(size_t index_space) = 0;
  virtual
  size_t
  indices(
    size_t index_space
  ) const
  {
    return 0;
  } // indices

//data instances
public:
  using index_partition_t = dmp::index_partition__<size_t>;
  
  //map of the all partitions used in the code
  //std::map <name of the partition<entiry, index partition for entity>
 
  std::unordered_map<utils::const_string_t,
    std::unordered_map<utils::const_string_t, index_partition_t,
      utils::const_string_hasher_t>, utils::const_string_hasher_t > partitions;

private:

  size_t id_;

}; // class data_client_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_client_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
