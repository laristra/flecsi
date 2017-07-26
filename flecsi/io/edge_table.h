/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_io_edge_table_h
#define flecsi_io_edge_table_h

/// \file

// system includes
#include <algorithm>
#include <map>

namespace flecsi {
namespace io {


////////////////////////////////////////////////////////////////////////////////
/// \brief a data structure to make finding edges via point pairs
///        easier/faster.
////////////////////////////////////////////////////////////////////////////////
template< typename IndexType >
class edge_table__ {

  //! \breif the size type
  using size_type = std::size_t;

  //! \brief the indexing type
  using index_type = IndexType;

  //============================================================================
  //! \brief the key type for the map
  //! This is essential point pairs for each edge, with a comparison operator. 
  //! The point pairs are stored such that the first is less
  //============================================================================
  struct pair_type 
  {
    //! \brief the first and second point
    index_type first, second;
    //! \brief the comparison operator for sorting
    friend bool operator<(const pair_type & lhs, const pair_type & rhs)
    {
      if (lhs.first != rhs.first)
        return lhs.first < rhs.first;
      else
        return lhs.second < rhs.second;
    }
  };

  //! \brief the map type used for searching
  using map_type = std::map< pair_type, index_type >;
  //! \brief the list type used for iterating in order
  using list_type = std::vector< pair_type >;

  //! \brief the iterator for moving through edges in order
  using iterator = typename list_type::const_iterator;


  //============================================================================
  //! \brief a result type, similar to maps but with named paramers
  //============================================================================
  struct find_result_type 
  {
    //! \brief the iterator position the found edge
    iterator position;
    //! \brief the id of the found edge
    index_type id;
    //! \brief whether the point pair match is reversed
    bool is_reversed;
  };

  //============================================================================
  //! \brief a result type, similar to maps but with named paramers
  //============================================================================
  struct insert_result_type
  {
    //! \brief the iterator position the found edge
    iterator position;
    //! \brief the id of the found edge
    index_type id;
    //! \brief whether the point pair match is reversed
    bool is_reversed;
    //! \brief whether the point pair was inserted
    bool was_inserted;
  };

  //============================================================================
  // private data
  //============================================================================

  //! \brief the searchable map
  map_type map_;
  //! \brief the list of ordered edges
  list_type edges_;

public:

  //============================================================================
  // public accessors
  //============================================================================

  //! \brief return the size of the edge list
  size_type size() const noexcept
  { 
    return edges_.size(); 
  }

  //! \brief return an iterator to the beginning of the list
  iterator begin() const noexcept
  { 
    return edges_.begin();
  }

  //! \brief return an iterator to the end of the list
  iterator end() const noexcept
  { 
    return edges_.end();
  }

  //! \brief return the edge pair for a specified index
  //! \param [in] e the edge index
  //! \return a const reference to the edge pair
  //! \remark this version has no bounds checking
  iterator operator[]( size_type n ) const noexcept
  {
    return std::next( begin(), n );
  }

  //============================================================================
  //! \brief insert an edge, and return the iterator to that edge
  //! Uses a more sophisticated return value that also identifies if the edge
  //! were inserted
  //============================================================================
  insert_result_type insert( index_type p0, index_type p1 ) 
  {
    auto minmax = std::minmax(p0, p1);
    auto edge_id = static_cast<index_type>( edges_.size() );
    // search for the point, p0 always less than p1
    auto it = 
      map_.emplace( pair_type{ minmax.first, minmax.second }, edge_id );
    // check if actually inserted
    if (it.second) {
      edges_.emplace_back( pair_type{ p0, p1 } );
      return { operator[](edge_id), edge_id, false, true };
    }
    // not inserted, return an iterator to the value that prevented insertion
    edge_id = it.first->second;
    auto first_point = edges_[edge_id].first;
    return { operator[](edge_id), edge_id, p0!=first_point, false };
  }

  //============================================================================
  //! \brief find an edge, and return the iterator to that edge
  //! Uses a more sophisticated return value that also identifies if the points
  //! were located in reversed order
  //============================================================================
  find_result_type find( index_type p0, index_type p1 ) const
  {
    auto minmax = std::minmax(p0, p1);
    // search for the point, p0 always less than p1
    auto it = map_.find( {minmax.first, minmax.second} );
    // found it
    if ( it != map_.end() ) {
      auto edge_id = it->second;
      return { operator[](edge_id), p0!=edges_[edge_id].first, edge_id };
    }
    // return end if not found
    return { edges_.end(), -1, false, false };

    
  }
  
  
    
};

} // namespace
} // namespace

#endif // flecsi_io_edge_table_h
