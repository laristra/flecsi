/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_communicator_h
#define flecsi_dmp_communicator_h

///
// \file communicator.h
// \authors bergen
// \date Initial file creation: Dec 06, 2016
///

namespace flecsi {
namespace dmp {

struct entry_info_t {
  size_t id;
  size_t rank;
  size_t offset;
  std::set<size_t> shared;
  
  entry_info_t(
    size_t id_ = 0,
    size_t rank_ = 0,
    size_t offset_ = 0,
    std::set<size_t> shared_ = {}
  )
    : id(id_), rank(rank_), offset(offset_), shared(shared_) {}

  // Sort cell info by id.
  bool
  operator < (
    const entry_info_t & c
  ) const
  {
    return id < c.id;
  } // operator <

}; // struct entry_info_t

inline
std::ostream &
operator << (
  std::ostream & stream,
  const entry_info_t & e
)
{
  stream << e.id << " " << e.rank << " " << e.offset << " [ ";
  for(auto i: e.shared) {
    stream << i << " "; 
  } // for
  stream << "]";
} // operator <<

///
// \class communicator_t communicator.h
// \brief communicator_t provides...
///
class communicator_t
{
public:

  /// Default constructor
  communicator_t() {}

  /// Copy constructor (disabled)
  communicator_t(const communicator_t &) = delete;

  /// Assignment operator (disabled)
  communicator_t & operator = (const communicator_t &) = delete;

  /// Destructor
  virtual ~communicator_t() {}

  // I don't know where this belongs yet, but I want to work on the
  // interface so I'm putting it here for now. It probably doesn't really
  // belong in this interface definition. For one thing, the specialization
  // should have a shot at defining how this type of operation happens. We
  // will also most likely use Legion to arbitrate this type of communication
  // as soon as possible.
  //
  // The point of this method is to get cell ownership information
  // from adjacent ranks.
  virtual
  std::pair<std::vector<std::set<size_t>>, std::set<entry_info_t>>
  get_cell_info(
    const std::set<size_t> & primary,
    const std::set<size_t> & request_indices
  ) = 0;

  // Same admonishment as for get_cell_info...
  //
  // The point of this method is to get vertex offsets from the
  // owning ranks.
  virtual
  std::vector<std::set<size_t>>
  get_vertex_info(
    const std::set<entry_info_t> & vertex_info,
    const std::vector<std::set<size_t>> & request_indices
  ) = 0;

private:

}; // class communicator_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_communicator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
