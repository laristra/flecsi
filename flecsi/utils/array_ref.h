/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */


#include <array>
#include <assert.h>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace flecsi {
namespace utils {

/*!
 \class array_ref<T> represents an immutable array of \c size()
 elements of type T.  The storage for the array is *not* owned by
 the \c array_ref object, and clients must arrange for the backing
 store to remain live while the \c array_ref object is in use.

 Implicit conversion operations are provided from types with
 contiguous iterators like \c std::vector, \c std::string, \c
 std::array, and primitive arrays.  \c array_ref objects are
 invalidated by any operation that invalidates their underlying
 pointers.

 One common use for \c array_ref is when passing arguments to a
 routine where you want to be able to accept a variety of array
 types.  The usual approach here is to have the client explicitly
 pass in a pointer and a length, as in:
 \snippet array_ref_run_test.cc MyOldRoutine

 \par
 Unfortunately, this leads to ugly and error-prone code at the
 call site:
 \snippet array_ref_run_test.cc MyOldRoutine calls

 \par
 Instead, you can use an \c array_ref as the argument to the
 routine:
 \snippet array_ref_run_test.cc MyNewRoutine

 \par
 This makes the call sites cleaner, for the most part:
 \snippet array_ref_run_test.cc MyNewRoutine calls

 \todo The existing \c array_ref classes make the view const. It
 may be useful to extend that to allow modifications of the
 referenced array elements, and use \c array_ref<const T> for
 immutable views.

 @ingroup legion-execution
 */

template<typename T>
class array_ref {

public:
  /// \name types
  /// @{
  typedef T value_type;
  /// \todo Should the pointer type be configurable as a template argument?
  typedef const T * pointer;
  typedef const T & reference;
  typedef const T & const_reference;
  /// \xmlonly <implementation-defined-type/> \endxmlonly
  /// random-access, contiguous iterator type
  typedef const T * const_iterator;
  /// Because array_ref controls a constant sequence, iterator and
  /// const_iterator are the same type.
  typedef const_iterator iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef const_reverse_iterator reverse_iterator;
  typedef std::size_t size_type;
  typedef ptrdiff_t difference_type;
  /// @}

  /// \name construct/copy
  /// @{

  /// \post <code>empty() == true</code>
  constexpr array_ref() : ptr_(nullptr), length_(0) {}
  constexpr array_ref(const array_ref &) = default;
  array_ref & operator=(const array_ref &) = default;

  constexpr array_ref(const T * array, const size_type length)
      : ptr_(array), length_(length) {}

  // Implicit conversion constructors

  /// \todo Arguably, this conversion should be a std::vector
  /// conversion operator.
  template<typename Allocator>
  array_ref(const std::vector<T, Allocator> & v)
      : ptr_(v.data()), length_(v.size()) {}

  template<typename traits, typename Allocator>
  array_ref(const std::basic_string<T, traits, Allocator> & s)
      : ptr_(s.data()), length_(s.size()) {}

  template<size_type N>
  constexpr array_ref(const T (&a)[N]) : ptr_(a), length_(N) {}

  /// \todo Arguably, this conversion should be a std::array
  /// conversion operator.
  template<size_type N>
  constexpr array_ref(const std::array<T, N> & a)
      : ptr_(a.data()), length_(N) {}

  /// \todo See \c basic_string_ref::substr for interface
  /// questions. We want something like this on \c array_ref, but
  /// probably not with this name.
  constexpr array_ref
  substr(const size_type pos, const size_type n = size_type(-1)) const {
    // Recursive implementation to satisfy constexpr.
    return (
        pos > size() ? substr(size(), n)
                     : n > size() - pos ? substr(pos, size() - pos)
                                        : array_ref(data() + pos, n));
  }
  /// @}

  /// \name iterators
  /// @{
  constexpr const_iterator begin() const {
    return ptr_;
  }
  constexpr const_iterator end() const {
    return ptr_ + length_;
  }
  constexpr const_iterator cbegin() const {
    return begin();
  }
  constexpr const_iterator cend() const {
    return end();
  }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const {
    return rbegin();
  }
  const_reverse_iterator crend() const {
    return rend();
  }
  /// @}

  /// \name capacity
  /// @{
  constexpr size_type size() const {
    return length_;
  }
  constexpr size_type max_size() const {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }
  constexpr bool empty() const {
    return length_ == 0;
  }
  /// @}

  /// \name element access
  /// @{
  constexpr const T & operator[](const size_type i) const {
    return ptr_[i];
  }
  constexpr const T & at(const size_type i) const {
    // This makes at() constexpr as long as the argument is within the
    // bounds of the array_ref.
    /*
    // However, possibly returning a throw, as in the following construction,
    // gives "warning: returning reference to temporary [-Wreturn-local-addr]"
    return i >= size() ? throw std::out_of_range("at() argument out of range")
      : ptr_[i];
    */
    if (i >= size())
      throw std::out_of_range("at() argument out of range");
    return ptr_[i];
  }

  constexpr const T & front() const {
    return ptr_[0];
  }
  constexpr const T & back() const {
    return ptr_[length_ - 1];
  }

  /// \returns A pointer such that [<code>data()</code>,<code>data() +
  /// size()</code>) is a valid range. For a non-empty array_ref,
  /// <code>data() == &front()</code>.
  constexpr const T * data() const {
    return ptr_;
  }
  /// @}

  /// \name Outgoing conversion operators
  ///
  /// These functions provide explicit conversions to selected other
  /// contiguous sequence types using those types' iterator-range
  /// constructors.  We provide both explicit conversion operators for
  /// use in variable initialization and short member functions for
  /// use in function calls.
  ///
  /// The operators are \c explicit to avoid accidental O(N)
  /// operations on type mismatches.
  ///
  /// @{

  /// \todo Arguably, this conversion should be a std::vector
  /// constructor.
  explicit operator std::vector<T>() const {
    return std::vector<T>(begin(), end());
  }
  std::vector<T> vec() const {
    return std::vector<T>(*this);
  }

  /// \todo Arguably, this conversion should be a std::basic_string
  /// constructor.
  template<typename traits, typename Allocator>
  explicit operator std::basic_string<T, traits, Allocator>() const {
    return std::basic_string<T, traits, Allocator>(begin(), end());
  }
  std::basic_string<T> str() const {
    return std::basic_string<T>(*this);
  }

  /// @}

  /// \name mutators
  /// @{

  /// \par Effects:
  /// Resets *this to its default-constructed state.
  void clear() {
    *this = array_ref();
  }

  /// \par Effects:
  /// Advances the start pointer of this array_ref past \p n elements
  /// without moving the end pointer.
  void remove_prefix(const size_type n) {
    assert(length_ >= n);
    ptr_ += n;
    length_ -= n;
  }

  /// \par Effects:
  /// Moves the end pointer of this array_ref earlier by \p n elements
  /// without moving the start pointer.
  void remove_suffix(const size_type n) {
    assert(length_ >= n);
    length_ -= n;
  }
  /// \par Effects:
  /// <code>remove_suffix(1)</code>
  void pop_back() {
    remove_suffix(1);
  }
  /// \par Effects:
  /// <code>remove_prefix(1)</code>
  void pop_front() {
    remove_prefix(1);
  }
  /// @}

private:
  const T * ptr_;
  size_type length_;
};

/// \name deducing constructor wrappers
/// \relates std::array_ref
/// \xmlonly <nonmember/> \endxmlonly
///
/// These functions do the same thing as the constructor with the same
/// signature. They just allow users to avoid writing the iterator
/// type.
/// @{

template<typename T>
constexpr array_ref<T>
make_array_ref(const T * array, const std::size_t length) {
  return array_ref<T>(array, length);
}

template<typename T, std::size_t N>
constexpr array_ref<T>
make_array_ref(const T (&a)[N]) {
  return array_ref<T>(a);
}

template<typename T>
array_ref<T>
make_array_ref(const std::vector<T> & v) {
  return array_ref<T>(v);
}

template<typename T, std::size_t N>
array_ref<T>
make_array_ref(const std::array<T, N> & a) {
  return array_ref<T>(a);
}

/// @}

} // namespace utils
} // namespace flecsi
