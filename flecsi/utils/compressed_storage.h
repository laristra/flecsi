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

// user includes
#include "flecsi/utils/array_ref.h"

// system includes
#include <algorithm>
#include <iomanip>
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
        
  constexpr const value_type * data() const { return data_.data(); }
  constexpr value_type * data() { return data_.data(); }
      
  //============================================================================
  // Iterators
  //============================================================================

  class iterator : 
    public std::iterator<
      std::random_access_iterator_tag,
      array_ref<value_type>,
      difference_type,
      array_ref<value_type>,
      array_ref<value_type>
    >
  {
  public:
    iterator(const iterator&) = default;
    iterator(iterator&&) = default;
    iterator( compressed_row_storage * ptr = nullptr, size_type row = 0 )
      : ptr_(ptr), row_(row) {}
    ~iterator(){}
    iterator& operator=(const iterator&) = default;

    operator bool() const
    { return (ptr_ != nullptr); }
    
    bool operator==(const iterator& it) const
    { return (ptr_ == it.ptr_ && row_ == it.row_); }
    bool operator!=(const iterator& it) const
    { return (ptr_ != it.ptr_ || row_ != it.row_); }
    
    iterator& operator+=(const difference_type& n)
    { row_ += n; return (*this); }
    iterator& operator-=(const difference_type& n)
    { row_ -= n; return (*this); }
    iterator& operator++() { ++row_;return (*this); }
    iterator& operator--() { --row_;return (*this); }
    iterator  operator++(int){ return {ptr_, ++row_}; }
    iterator  operator--(int){ return {ptr_, --row_}; }
    iterator  operator+(const difference_type& n) { return {ptr_, row_+n}; }
    iterator  operator-(const difference_type& n) { return {ptr_, row_-n}; }
    
    difference_type operator-(const iterator& it) { return row_-it.row_; }

    auto operator*() { return ptr_->row(row_); }
    auto operator*() const { return ptr_->row(row_); }
    auto operator->() { tmp_row_ = ptr_->row(row_); return &tmp_row_; }

  private:
    compressed_row_storage * ptr_ = nullptr;
    size_type row_ = 0;
    mutable array_ref<value_type> tmp_row_;
  };

  auto begin() { return iterator(this); }
  auto end() { return iterator(this, rows()); }

  auto row(size_type i) const
  { 
    return make_array_ref(
      data_.data() + row_pointer_[i],
      row_pointer_[i+1] - row_pointer_[i]
    );
  }

  //============================================================================
  // Capacity
  //============================================================================

  constexpr auto rows() const { return row_pointer_.size() - 1; }
  constexpr auto cols() const { return num_columns_; }
  
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
    row_pointer_.resize(1);
    row_pointer_[0] = 0;
    num_columns_ = 0;
  }  
  
  void push_back( const T& value )
  {
    data_.push_back( value );
    row_pointer_.emplace_back( row_pointer_.back() + 1 );
    num_columns_ = std::max( num_columns_, static_cast<size_type>(1) );
  }

  void push_back( T&& value )
  {
    data_.push_back( std::move( value ) );
    row_pointer_.emplace_back( row_pointer_.back() + 1 );
    num_columns_ = std::max( num_columns_, static_cast<size_type>(1) );
  }

  template< typename InputIt >
  void push_back( InputIt first, InputIt last )
  {
    size_type n = std::distance( first, last );
    data_.insert( data_.end(), first, last );
    row_pointer_.emplace_back( row_pointer_.back() + n );
    num_columns_ = std::max( num_columns_, n );
  }

  void push_back( std::initializer_list<T> ilist )
  {
    data_.insert( data_.end(), ilist.begin(), ilist.end() );
    row_pointer_.emplace_back( row_pointer_.back() + ilist.size() );
    num_columns_ = std::max( num_columns_, ilist.size() );
  }

  template< class... Args, std::size_t... I >
  void emplace_back_( Args&&... args, std::index_sequence<I...> )
  {
    data_.emplace_back( std::forward<Args>(args)... );
    row_pointer_.emplace_back( row_pointer_.back() + sizeof...(args) );
  }

  template< class... Args >
  void emplace_back( Args&&... args )
  {
    constexpr auto N = sizeof...(Args);
    emplace_back_( std::forward<Args>(args)..., std::make_index_sequence<N>{} );
    num_columns_ = std::max( num_columns_, N );
  }

  template< typename InputIt >
  void insert( iterator pos, InputIt first, InputIt last )
  {
    // no op
    if ( first==last ) return;

    // distance from beginning
    auto row = std::distance( begin(), pos );

    // iteratively add rows
    for ( auto it=first; it!=last; ++it ) {
      // size of the row
      auto n = it->size();
      // find the position to insert into the non-zero array
      auto data_pos = std::next( data_.begin(), row_pointer_[row]  );
      data_.insert( data_pos, it->begin(), it->end() );
      // update the row pointers
      auto ptr_pos = std::next( row_pointer_.begin(), row+1 );
      ptr_pos = row_pointer_.emplace( ptr_pos, row_pointer_[row] + n );
      std::for_each(
        std::next(ptr_pos), row_pointer_.end(), [=](auto & i) { i+=n; } 
      );
      // check max column size
      num_columns_ = std::max( num_columns_, n );
      // increment row
      ++row;
    }
  }

  void insert( iterator pos, iterator first, iterator last )
  {

    // no op
    if ( first==last ) return;

    // distance from beginning
    auto row = std::distance( begin(), pos );
    std::cout << "adding to row " << row << std::endl;
    
    std::cout << "initial row pointer  : ";
    for ( auto i : row_pointer_ ) std::cout << std::setw(2) << i << " "; std::cout << std::endl;
    std::cout << "initial data pointer : ";
    for ( auto i : data_ ) std::cout << std::setw(2) << i << " "; std::cout << std::endl;

    // dont need to iteratively add rows, we can splice all the data 
    auto data_first = first->begin();
    auto data_end = std::prev(last)->end();
    auto n = std::distance(data_first, data_end);
    auto data_pos = std::next( data_.begin(), row_pointer_[row]  );
    std::cout << "inserting in " << row_pointer_[row] << " spot" << std::endl;
    data_.insert( data_pos, data_first, data_end );
    // the row pointer is trickier
    auto ptr_pos = std::next( row_pointer_.begin(), row+1 );
    auto offset = *ptr_pos;
    auto ptr_first = row_pointer_.insert( 
      ptr_pos, row_pointer_.begin(), std::prev(row_pointer_.end())
    );
    std::cout << "new row pointer  : ";
    for ( auto i : row_pointer_ ) std::cout << std::setw(2) << i << " "; std::cout << std::endl;
    std::cout << "new data pointer : ";
    for ( auto i : data_ ) std::cout << std::setw(2) << i << " "; std::cout << std::endl;
    //auto ptr_end = std::next(ptr_pos, n);
    //std::for_each(
    //  ptr_first, ptr_end, [=](auto & i) { i+offset; } 
    //);
    //offset = 
    //std::for_each(
    //  ptr_end, row_pointer_.end(), [=](auto & i) { i+offset; } 
    //);

  }


private:

  //============================================================================
  // private data
  //============================================================================

  //! \brief the actual data is stored in an array
  vector<value_type> data_;
  //! \brief the column pointers
  vector<size_type> row_pointer_ = {0};
  //! \brief the null value
  value_type null_value_ = value_type();
  
  //! \brief keep track of the number of columns
  size_type num_columns_ = 0;
        
        
};

}      // End namespace
}      // End namespace

#endif // flecsi_utils_compressed_storage
