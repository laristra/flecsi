/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2019, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

//! @file

namespace flecsi {
namespace utils {

//=============================================================================
//! An implementation of std::vector/std::list that allows
//! implicit conversion from one type to another.
//!
//! \tparam T  The value type of the list
//=============================================================================
template< typename T >
class list
{
  //! the underlying data
  std::vector<T> data_;

public:

  //---------------------------------------------------------------------------
  // public typedefs
  //---------------------------------------------------------------------------
  using value_type =	T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::vector<T>::pointer;
  using const_pointer = typename std::vector<T>::const_pointer;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using reverse_iterator = typename std::vector<T>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

  //---------------------------------------------------------------------------
  // Constructors
  //---------------------------------------------------------------------------
  
  //! Implicit conversion constructor
  //! \tparam U  the type of the other list
  //! \param other  the other list
  template<
    typename U,
    typename = std::enable_if_t< !std::is_same_v< list<U>, list<T> > >
  >
  list( list<U> && other ) : data_( std::forward<list<U>>(other).begin(), std::forward<list<U>>(other).end() )
  {}
  
  //! Implicit conversion constructor
  //! \tparam U  the value type
  //! \param count  the number of values
  //! \param value  the value to set
  template<typename U>
  list(size_type count, const U& value ) : data_(count, value) {}

  //! constructor with number of entries
  //! \param count  the number of values
  explicit list( size_type count ) : data_(count) {}

  //! constructor with iterators
  //! \tparam InputIt  the input iterator type
  //! \param first  the beginning iterator
  //! \param last   the ending iterator
  template< class InputIt >
  list( InputIt first, InputIt last ) : data_(first,last) {}

  //! default copy constructor
  list( const list& ) = default;
  //! default move constructor
  list( list&& ) = default;

  //! constructor with initializer list
  //! \tparam U  the list type
  //! \param init  the initializer list
  template<typename U>
  list( std::initializer_list<U> init ) : data_(init) {}
  
  //! default copy operator
  list& operator=( const list& ) = default;
  //! default move operator
  list& operator=( list&& ) = default;

  //! assigment operator with initializer list
  //! \tparam U  the type of the list
  //! \param ilist  the initializer list
  template<typename U>
  list& operator=( std::initializer_list<U> ilist )
  { data_ = ilist; }

  //---------------------------------------------------------------------------
  // Member functions
  //---------------------------------------------------------------------------

  //! assign values
  //! \param count  the number of values to assign
  //! \param value  the value to assign to entries
  void assign( size_type count, const T& value )
  { data_.assign(count, value); }

  //! assign values using iterators
  //! \tparam InputIt  the input iterator type
  //! \param first  the beginning iterator
  //! \param last  the last iterator
  template< class InputIt >
  void assign( InputIt first, InputIt last )
  { data_.assign(first, last); }

  //! assign values using initializer list
  //! \tparam U  The type of initializer list
  //! \param ilist  the initializer list
  template<typename U>
  void assign( std::initializer_list<U> ilist )
  { data_.assign(ilist); }

  //---------------------------------------------------------------------------
  // element access
  //---------------------------------------------------------------------------

  //! access individual element
  //! \param pos  the position of the element to access
  //! \return the value of the element
  //! \{
  const auto & at( size_type pos ) const
  { return data_.at(pos); }
  auto & at( size_type pos )
  { return data_.at(pos); }
  //! \}

  //! access individual element
  //! \param pos  the position of the element to access
  //! \return the value of the element
  //! \{
  const auto & operator[]( size_type pos ) const
  { return data_[pos]; }
  auto & operator[]( size_type pos )
  { return data_[pos]; }
  //! \}

  //! get a reference to the first element
  //! \return a reference to the first element
  //! \{
  const auto & front() const
  { return data_.front(); }
  auto & front()
  { return data_.front(); }
  //! \}

  //! get a reference to the last element
  //! \return a reference to the last element
  //! \{
  const auto & back() const
  { return data_.back(); }
  auto & back()
  { return data_.back(); }
  //! \}

  //! get a pointer to the raw data
  //! \return a pointer to the underlying data
  //! \{
  auto data() const noexcept
  { return data_.data(); }
  auto data() noexcept
  { return data_.data(); }
  //! \}


  //---------------------------------------------------------------------------
  // iterators
  //---------------------------------------------------------------------------

  //! get the begining iterator
  //! \return an iterator to the first element
  //! \{
  auto begin() const noexcept { return data_.begin(); }
  auto begin() noexcept { return data_.begin(); }
  auto cbegin() const noexcept { return data_.cbegin(); }
  //! \}
  
  //! get the begining reverse_iterator
  //! \return an iterator to the first element
  //! \{
  auto rbegin() const noexcept { return data_.rbegin(); }
  auto rbegin() noexcept { return data_.rbegin(); }
  auto crbegin() const noexcept { return data_.crbegin(); }
  //! \}

  //! get the ending iterator
  //! \return an iterator past the last element
  //! \{
  auto end() const noexcept { return data_.end(); }
  auto end() noexcept { return data_.end(); }
  auto cend() const noexcept { return data_.cend(); }
  //! \}

  //! get the ending reverse_iterator
  //! \return an iterator past the last element
  //! \{
  auto rend() const noexcept { return data_.rend(); }
  auto rend() noexcept { return data_.rend(); }
  auto crend() const noexcept { return data_.crend(); }
  //! \}

  //---------------------------------------------------------------------------
  // capacity
  //---------------------------------------------------------------------------

  //! return true if empty, false if populated
  auto empty() const noexcept { return data_.empty(); }
  
  //! return current size
  auto size() const noexcept { return data_.size(); }
  
  //! return max size
  auto max_size() const noexcept { return data_.max_size(); }
  
  //! reserve storage
  //! \param new_cap  the new capacity
  void reserve( size_type new_cap ) { data_.reserve(new_cap); }
  
  //! return the capacity
  auto capacity() const noexcept { return data_.capacity(); }
  
  //! shrink storage to fit current size
  void shrink_to_fit() { data_.shrink_to_fit(); }

  //---------------------------------------------------------------------------
  //modifiers
  //---------------------------------------------------------------------------

  //! reset the list
  void clear() noexcept { data_.clear(); }
 
  //! insert values into the list
  //! \param pos  the position to insert at
  //! \param value  the value to insert
  //! \return an iterator to the inserted value
  auto insert( const_iterator pos, const T& value )
  { return data_insert(pos, value); }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param value  the value to insert
  //! \return an iterator to the inserted value
  template<typename U>
  auto insert( const_iterator pos, U&& value )
  { return data_.insert(pos, std::forward<U>(value)); }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param count  the number of values to insert
  //! \param value  the value to insert
  //! \return an iterator to the first value inserted
  template<typename U>
  auto insert( const_iterator pos, size_type count, const U& value )
  { return data_.insert(pos, count, value); }

  //! insert values into the list
  //! \tparam InputIt  the input iterator type
  //! \param pos  the position to insert at
  //! \param first  the beginning iterator
  //! \param last  the ending iterator
  //! \return an iterator pointing to the first inserted value
  template< class InputIt >
  auto insert( const_iterator pos, InputIt first, InputIt last )
  { return data_.insert(pos, first, last); }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param ilist  the initializer list to insert
  //! \return an iterator pointing to the first inserted value
  template< typename U >
  auto insert( const_iterator pos, std::initializer_list<U> ilist )
  { return data_.insert(pos, ilist); }

  //! insert values into the list
  //! \tparam Args  the types of the arguments
  //! \param pos  the position to insert at
  //! \param args  the arguments passed to the new elements constructor
  //! \return an iterator pointing the inserted value
  template< class... Args >
  auto emplace( const_iterator pos, Args&&... args )
  { return data_.emplace(pos, std::forward<Args>(args)...); }

  //! erase an element
  //! \param pos  the position to erase
  //! \return Iterator following the last removed element.
  auto erase( const_iterator pos )
  { return data_.erase(pos); }

  //! erase a range of elements
  //! \param first  the first value to erase
  //! \param last an iterator to the end of the list to be deleted
  auto erase( const_iterator first, const_iterator last )
  { return data_.erase(first, last); }

  //! push a value onto the array
  //! \param value  the value to add
  void push_back( const T& value )
  { data_.push_back(value); }

  //! push a value onto the end of the list
  //! \tparam U  the type of the value to insert
  //! \param value  The value to add to the list
  template<typename U>
  void push_back( U&& value )
  { data_.push_back(std::forward<U>(value)); }

  //! push a value onto the end of the list
  //! \tparam Args  the types of the arguments
  //! \param args  the arguments passed to the new elements constructor
  template< class... Args >
  void emplace_back( Args&&... args )
  { data_.emplace_back(std::forward<Args>(args)...); }

  //! pop the last value off the array
  void pop_back()
  { data_.pop_back(); }

  //! resize the list
  //! \param count  the number of elements to increase by
  void resize( size_type count )
  { data_.resize(count); }

  //! resize the list
  //! \param count  the number of elements to increase by
  //! \param value  the value to insert for each element
  void resize( size_type count, const value_type& value )
  { data_.resize(count, value); }

  //! swap data with another list
  //! \param other  the other list to swap with
  void swap( list& other )
  { data_.swap(other); }

};

} // namespace
} // namespace
