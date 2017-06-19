/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// 
/// \brief A sparse matrix container
///
/// It's similar to a map, but it uses more efficient lookups.
////////////////////////////////////////////////////////////////////////////////
#ifndef flecsi_utils_compressed_storage
#define flecsi_utils_compressed_storage

// system includes
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace flecsi {
namespace utils {
    
////////////////////////////////////////////////////////////////////////////////
//! \brief the sparse matrix class
//!
//! Uses compressed row storage
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class compressed_row_storage {

public:

  //============================================================================
  // public typedefs
  //============================================================================
 
  //! An alias to the vector type
  template< typename U >
  using vector = typename std::vector<U>;

  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = typename vector<value_type>::iterator;
  using const_iterator = typename vector<value_type>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        
  //============================================================================
  // Construct / copy
  //============================================================================
  constexpr compressed_row_storage() = default;

  constexpr compressed_row_storage(const compressed_row_storage& o) = default; 

  constexpr compressed_row_storage(compressed_row_storage &&) = default;

  compressed_row_storage& operator=(const compressed_row_storage&) = default;
  compressed_row_storage& operator=(compressed_row_storage&&) = default;
        
  explicit compressed_row_storage( size_type rows, const T& null_value = T() ) :
    row_pointer_(rows+1,0),  null_value_(null_value)
  {}

  //============================================================================
  // element access
  //============================================================================

  constexpr const_reference operator[](size_type i) const noexcept
  { return data_[i]; }

  constexpr reference operator[](size_type i) noexcept
  { return data_[i]; }

  constexpr const_reference at(size_type i) const 
  {
    // This makes at() constexpr as long as the argument is within the bounds.
    return i >= size() ? throw std::out_of_range("at() argument out of range")
      : this->operator[](i);
  }
  constexpr reference at(size_type i)
  {
    // This makes at() constexpr as long as the argument is within the bounds.
    return i >= size() ? throw std::out_of_range("at() argument out of range")
      : this->operator[](i);
  }
  
  constexpr auto row(size_type i) const 
  {
    // This makes at() constexpr as long as the argument is within the bounds.
    if ( i >= rows() ) 
      throw std::out_of_range("row() argument out of range");
    vector<T> vals( row_begin(i), row_end(i) );
    return vals;
  }

#if 0
  constexpr const_reference operator()(size_type i, size_type j) const noexcept;

  constexpr reference operator()(size_type i, size_type j) noexcept;

  constexpr const_reference at(size_type i, size_type j) const 
  {
    // This makes at() constexpr as long as the argument is within the bounds.
    return i >= size() ? throw std::out_of_range("at() argument out of range")
      : this->operator()(i,j);
  }
  constexpr reference at(size_type i, size_type j)
  {
    // This makes at() constexpr as long as the argument is within the bounds.
    return i >= size() ? throw std::out_of_range("at() argument out of range")
      : this->operator()(i,j);
  }
#endif
      
  //============================================================================
  // Iterators
  //============================================================================

  constexpr iterator row_begin(size_type i) 
  { return std::next(data_.begin(), row_pointer_[i] ); }

  constexpr iterator row_end(size_type i) 
  { return std::next(data_.begin(), row_pointer_[i+1] ); }

  constexpr const_iterator row_begin(size_type i) const
  { return std::next(data_.begin(), row_pointer_[i] ); }

  constexpr const_iterator row_end(size_type i) const
  { return std::next(data_.begin(), row_pointer_[i+1] ); }

  constexpr const_iterator row_cbegin(size_type i) const 
  { return row_begin(i); }

  constexpr const_iterator row_cend(size_type i) const
  { return row_end(i); }

  reverse_iterator row_rbegin(size_type i) 
  { return row_end(i); }

  reverse_iterator row_rend(size_type i) 
  { return row_begin(i); }

  constexpr const_reverse_iterator row_crbegin(size_type i) const
  { return row_end(i); }

  constexpr const_reverse_iterator row_crend(size_type i) const
  { return row_begin(i); }

  // constexpr const_iterator begin() const { return data_.begin(); }
  // constexpr const_iterator end() const { return std::next(data_.begin(), length_); }
  // constexpr const_iterator cbegin() const { return begin(); }
  // constexpr const_iterator cend() const { return end(); }
  // reverse_iterator rbegin() const {
  //   return const_reverse_iterator(end());
  // }
  // reverse_iterator rend() const {
  //   return const_reverse_iterator(begin());
  // }
  // const_reverse_iterator crbegin() const { return rbegin(); }
  // const_reverse_iterator crend() const { return rend(); }

        

  //============================================================================
  // Capacity
  //============================================================================

  constexpr auto rows() const { return row_pointer_.size() - 1; }
  
  constexpr auto size() const { return data_.size(); }
  constexpr auto max_size() const { return data_.max_size(); }
  constexpr auto capacity() const { return data_.capacity(); }
  constexpr bool empty() const { return data_.empty(); }
        
  //============================================================================
  // Modifiers
  //============================================================================

  void clear() 
  { 
    data_.clear();
    column_index_.clear();
    row_pointer_.resize(1);
    row_pointer_[0] = 0;
  }  
  
  void push_back( const T& value )
  {
    data_.push_back( value );
    column_index_.emplace_back( 0 );
    row_pointer_.emplace_back( row_pointer_.back() + 1 );
  }

  void push_back( T&& value )
  {
    data_.push_back( std::move( value ) );
    column_index_.emplace_back( 0 );
    row_pointer_.emplace_back( row_pointer_.back() + 1 );
  }

  template< typename InputIt >
  void push_back( InputIt first, InputIt last )
  {
    size_type col(0);
    for ( auto it=first; it!=last; ++it ) {
      data_.push_back( *it );
      column_index_.emplace_back( col++ );
    }
    row_pointer_.emplace_back( row_pointer_.back() + col );
  }

  template< class... Args, std::size_t... I >
  void emplace_back_( Args&&... args, std::index_sequence<I...> )
  {
    data_.emplace_back( std::forward<Args>(args)... );
    column_index_.emplace_back( I... );
    row_pointer_.emplace_back( row_pointer_.back() + sizeof...(args) );
  }

  template< class... Args >
  void emplace_back( Args&&... args )
  {
    constexpr auto N = sizeof...(Args);
    emplace_back_( std::forward<Args>(args)..., std::make_index_sequence<N>{} );
  }


private:

  //============================================================================
  // private data
  //============================================================================

  //! \brief the actual data is stored in an array
  vector<value_type> data_;
  //! \brief the row indices
  vector<size_type> column_index_;
  //! \brief the column pointers
  vector<size_type> row_pointer_ = {0};
  //! \brief the null value
  value_type null_value_ = value_type();
        
};

}      // End namespace
}      // End namespace

#endif // flecsi_utils_compressed_storage
