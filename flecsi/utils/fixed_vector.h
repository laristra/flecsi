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

#include <flecsi/utils/type_traits.h>

#include <algorithm>
#include <array>
#include <cassert>

namespace flecsi {
namespace utils {

//=============================================================================
//! \brief An combination of std::vector/std::array that allows
//! implicit conversion from one type to another.
//!
//! This class serves two purposes:
//! - To implicitly convert between a list of one type to a list of another.
//!   This is useful for converting lists of handles to lists of accessors.
//! - Define a statically-sized list, like std::array, that behavies similarly
//!   to a dynamic list, like std::vector.  The maximum capacity is set at
//!   compile time, and users may push values onto the list.  As a result,
//!   size() /= capacity().
//!
//! \tparam T  The value type of the list
//! \tparam MAX_ENTRIES  The maximum capacity
//=============================================================================
template<typename T, std::size_t MAX_ENTRIES>
class fixed_vector
{

public:
  //---------------------------------------------------------------------------
  // public typedefs
  //---------------------------------------------------------------------------

  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  //---------------------------------------------------------------------------
  // Constructors
  //---------------------------------------------------------------------------

  //! default constructor
  constexpr fixed_vector() {}

  //! Implicit conversion constructor
  //! \tparam U  the type of the other list
  //! \param other  the other list
  template<typename U,
    std::size_t N,
    typename = std::enable_if_t<N<MAX_ENTRIES + 1>> fixed_vector(
      const fixed_vector<U, N> & other) : length_(other.size()) {
    std::copy_n(other.begin(), length_, begin());
  }

  //! Implicit conversion constructor
  //! \tparam U  the value type
  //! \param count  the number of values
  //! \param value  the value to set
  template<typename U>
  fixed_vector(size_type count, const U & value) : length_(count) {
    assign(count, value);
  }

  //! constructor with number of entries
  //! \param count  the number of values
  explicit fixed_vector(size_type count) : length_(count) {
    assert(size() <= capacity());
  }

  //! constructor with iterators
  //! \tparam InputIt  the input iterator type
  //! \param first  the beginning iterator
  //! \param last   the ending iterator
  template<class InputIt,
    typename = typename std::enable_if_t<is_iterator_v<InputIt>>>
  fixed_vector(InputIt first, InputIt last) {
    assign(first, last);
  }

  //! constructor with initializer list
  //! \tparam U  the list type
  //! \param init  the initializer list
  template<typename U>
  fixed_vector(std::initializer_list<U> init) {
    assign(init);
  }

  //! assigment operator with initializer list
  //! \tparam U  the type of the list
  //! \param init  the initializer list
  template<typename U>
  fixed_vector & operator=(std::initializer_list<U> init) {
    assign(init);
  }

  //---------------------------------------------------------------------------
  // Member functions
  //---------------------------------------------------------------------------

  //! assign values
  //! \param count  the number of values to assign
  //! \param value  the value to assign to entries
  void assign(size_type count, const value_type & value) {
    length_ = count;
    assert(size() <= capacity());
    std::fill_n(begin(), count, value);
  }

  //! assign values using iterators
  //! \tparam InputIt  the input iterator type
  //! \param first  the beginning iterator
  //! \param last  the last iterator
  template<class InputIt>
  void assign(InputIt first, InputIt last) {
    length_ = std::distance(first, last);
    assert(size() <= capacity());
    std::copy(first, last, begin());
  }

  //! assign values using initializer list
  //! \tparam U  The type of initializer list
  //! \param ilist  the initializer list
  template<typename U>
  void assign(std::initializer_list<U> ilist) {
    length_ = ilist.size();
    assert(size() <= capacity());
    std::copy(ilist.begin(), ilist.end(), begin());
  }

  //---------------------------------------------------------------------------
  // element access
  //---------------------------------------------------------------------------

  //! access individual element
  //! \param pos  the position of the element to access
  //! \return the value of the element
  //! \{
  const auto & at(size_type pos) const {
    return data_.at(pos);
  }
  auto & at(size_type pos) {
    return data_.at(pos);
  }
  //! \}

  //! access individual element
  //! \param pos  the position of the element to access
  //! \return the value of the element
  //! \{
  const auto & operator[](size_type pos) const {
    return data_[pos];
  }
  auto & operator[](size_type pos) {
    return data_[pos];
  }
  //! \}

  //! get a reference to the first element
  //! \return a reference to the first element
  //! \{
  const auto & front() const {
    return data_[0];
  }
  auto & front() {
    return data_[0];
  }
  //! \}

  //! get a reference to the last element
  //! \return a reference to the last element
  //! \{
  const auto & back() const {
    return data_[size() - 1];
  }
  auto & back() {
    return data_[size() - 1];
  }
  //! \}

  //! get a pointer to the raw data
  //! \return a pointer to the underlying data
  //! \{
  auto data() const noexcept {
    return data_.data();
  }
  auto data() noexcept {
    return data_.data();
  }
  //! \}

  //---------------------------------------------------------------------------
  // iterators
  //---------------------------------------------------------------------------

  //! get the begining iterator
  //! \return an iterator to the first element
  //! \{
  auto begin() const noexcept {
    return data_.begin();
  }
  auto begin() noexcept {
    return data_.begin();
  }
  auto cbegin() const noexcept {
    return data_.cbegin();
  }
  //! \}

  //! get the begining reverse_iterator
  //! \return an iterator to the first element
  //! \{
  auto rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  auto rbegin() noexcept {
    return reverse_iterator(end());
  }
  auto crbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  //! \}

  //! get the ending iterator
  //! \return an iterator past the last element
  //! \{
  auto end() const noexcept {
    return std::next(data_.begin(), length_);
  }
  auto end() noexcept {
    return std::next(data_.begin(), length_);
  }
  auto cend() const noexcept {
    return std::next(data_.cbegin(), length_);
  }
  //! \}

  //! get the ending reverse_iterator
  //! \return an iterator past the last element
  //! \{
  auto rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  auto rend() noexcept {
    return reverse_iterator(begin());
  }
  auto crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }
  //! \}

  //---------------------------------------------------------------------------
  // capacity
  //---------------------------------------------------------------------------

  //! return true if empty, false if populated
  auto empty() const noexcept {
    return length_ == 0;
  }

  //! return current size
  auto size() const noexcept {
    return length_;
  }

  //! return max size
  auto max_size() const noexcept {
    return MAX_ENTRIES;
  }

  //! return the capacity
  auto capacity() const noexcept {
    return MAX_ENTRIES;
  }

  //---------------------------------------------------------------------------
  // modifiers
  //---------------------------------------------------------------------------

  //! reset the list
  void clear() noexcept {
    length_ = 0;
  }

  //! insert values into the list
  //! \param pos  the position to insert at
  //! \param value  the value to insert
  //! \return an iterator to the inserted value
  auto insert(const_iterator pos, const value_type & value) {
    assert(size() < capacity());
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // shift data to the right
    auto d_last = std::next(d_first);
    std::copy(d_first, end(), d_last);
    // assign value
    *d_first = value;
    // increment counter
    length_++;
    // return an iterator to the inserted value
    return d_first;
  }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param value  the value to insert
  //! \return an iterator to the inserted value
  template<typename U>
  auto insert(const_iterator pos, U && value) {
    assert(size() < capacity());
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // shift data to the right
    auto d_last = std::next(d_first);
    std::move(d_first, end(), d_last);
    // assign value
    *d_first = std::move(value);
    // increment counter
    length_++;
    // return an iterator to the inserted value
    return d_first;
  }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param count  the number of values to insert
  //! \param value  the value to insert
  //! \return an iterator to the first value inserted
  template<typename U>
  auto insert(const_iterator pos, size_type count, const U & value) {
    assert(size() + count <= capacity());
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // if zero size is inserted, return pos
    if(count == 0)
      return d_first;
    // otherwise, shift data to the right
    auto d_last = std::next(d_first, count);
    std::copy(d_first, end(), d_last);
    // assign value
    std::fill_n(d_first, count, value);
    // increment counter
    length_ += count;
    // return an iterator to the inserted value
    return d_first;
  }

  //! insert values into the list
  //! \tparam InputIt  the input iterator type
  //! \param pos  the position to insert at
  //! \param first  the beginning iterator
  //! \param last  the ending iterator
  //! \return an iterator pointing to the first inserted value
  template<class InputIt,
    typename = typename std::enable_if_t<is_iterator_v<InputIt>>>
  auto insert(const_iterator pos, InputIt first, InputIt last) {
    // figure out the added size
    auto count = std::distance(first, last);
    assert(size() + count <= capacity());
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // if zero size is inserted, return pos
    if(count == 0)
      return d_first;
    // otherwise, shift data to the right
    auto d_last = std::next(d_first, count);
    std::move(d_first, end(), d_last);
    // assign values
    std::copy(first, last, d_first);
    // increment counter
    length_ += count;
    // return an iterator to the inserted value
    return d_first;
  }

  //! insert values into the list
  //! \tparam U  the type of the value to insert
  //! \param pos  the position to insert at
  //! \param ilist  the initializer list to insert
  //! \return an iterator pointing to the first inserted value
  template<typename U>
  auto insert(const_iterator pos, std::initializer_list<U> ilist) {
    // figure out the added size
    auto count = ilist.size();
    assert(size() + count <= capacity());
    // convert to a non-const iterator
    auto start = std::next(begin(), std::distance(cbegin(), pos));
    // if zero size is inserted, return pos
    if(count == 0)
      return start;
    // otherwise, shift data to the right
    auto next = std::next(start, count);
    std::move(start, end(), next);
    // assign values
    std::copy(ilist.begin(), ilist.end(), start);
    // increment counter
    length_ += count;
    // return an iterator to the inserted value
    return start;
  }

  //! insert values into the list
  //! \tparam Args  the types of the arguments
  //! \param pos  the position to insert at
  //! \param args  the arguments passed to the new elements constructor
  //! \return an iterator pointing the inserted value
  template<class... Args>
  auto emplace(const_iterator pos, Args &&... args) {
    assert(size() < capacity());
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // shift data to the right
    auto d_last = std::next(d_first);
    std::move(d_first, end(), d_last);
    // assign value
    *d_first = std::move(value_type(std::forward<Args>(args)...));
    // increment counter
    length_++;
    // return an iterator to the inserted value
    return d_first;
  }

  //! erase an element
  //! \param pos  the position to erase
  //! \return Iterator following the last removed element.
  auto erase(const_iterator pos) {
    // empty, nothing to do
    if(empty())
      return end();
    // convert to a non-const iterator
    auto d_first = _validate_iterator(pos);
    // get next position
    auto d_last = std::next(d_first);
    // if this is not last element, shift the values left
    if(d_last != end())
      std::move(d_last, end(), d_first);
    // decrement counter
    length_--;
    // return the iterator following the last removed element
    return d_first;
  }

  //! erase a range of elements
  //! \param first  the first value to erase
  //! \param last an iterator to the end of the list to be deleted
  auto erase(const_iterator first, const_iterator last) {
    // if empty, nothing to do
    if(empty())
      return end();
    // if first == last, nothring to do
    if(first == last)
      return begin();
    // get the number of deleted elements
    auto count = std::distance(first, last);
    // convert to a non-const iterator
    auto d_first = _validate_iterator(first);
    auto d_last = _validate_iterator(last);
    // if we are deleting from the middle, shift values left
    if(last != end())
      std::move(d_last, end(), d_first);
    // the new length
    length_ -= count;
    // return the iterator following the last removed element
    return d_first;
  }

  //! push a value onto the array
  //! \param value  the value to add
  void push_back(const value_type & value) {
    assert(size() < capacity());
    data_[length_++] = value;
  }

  //! push a value onto the end of the list
  //! \tparam U  the type of the value to insert
  //! \param value  The value to add to the list
  template<typename U>
  void push_back(U && value) {
    assert(size() < capacity());
    data_[length_++] = std::move(value);
  }

  //! push a value onto the end of the list
  //! \tparam Args  the types of the arguments
  //! \param args  the arguments passed to the new elements constructor
  template<class... Args>
  void emplace_back(Args &&... args) {
    assert(size() < capacity());
    data_[length_++] = value_type(std::forward<Args>(args)...);
  }

  //! pop the last value off the array
  void pop_back() {
    if(empty())
      return;
    length_--;
  }

  //! resize the list
  //! \param count  the number of elements to increase by
  void resize(size_type count) {
    length_ = count;
    assert(size() <= capacity());
  }

  //! resize the list
  //! \param count  the number of elements to increase by
  //! \param value  the value to insert for each element
  void resize(size_type count, const value_type & value) {
    assert(count <= capacity());
    for(size_type i = length_; i < count; i++)
      data_[i] = value;
    length_ = count;
  }

  //! swap data with another list
  //! \param other  the other list to swap with
  void swap(fixed_vector & other) {
    std::swap(length_, other.length_);
    std::swap(data_, other.data_);
  }

private:
  //---------------------------------------------------------------------------
  // private members
  //---------------------------------------------------------------------------

  iterator _validate_iterator(const_iterator pos) {
    auto n = std::distance(cbegin(), pos);
    assert(n >= 0 && n <= length_);
    return std::next(begin(), n);
  }

  //---------------------------------------------------------------------------
  // private data
  //---------------------------------------------------------------------------

  //! the underlying data
  std::array<T, MAX_ENTRIES> data_;

  //! the actual size used
  size_type length_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
//! \brief non member functions
////////////////////////////////////////////////////////////////////////////////
template<class T, std::size_t N>
bool
operator==(const fixed_vector<T, N> & lhs, const fixed_vector<T, N> & rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

} // namespace utils
} // namespace flecsi
