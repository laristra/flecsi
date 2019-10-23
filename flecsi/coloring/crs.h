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

#include <flecsi/utils/array_ref.h>

namespace flecsi {
namespace coloring {

//----------------------------------------------------------------------------//
// Convenience macro to avoid having to reimplement this for each member.
//----------------------------------------------------------------------------//

#define define_as(member)                                                      \
  template<typename T>                                                         \
  std::vector<T> member##_as() const {                                         \
    std::vector<T> asvec(member.begin(), member.end());                        \
    return asvec;                                                              \
  }

/*
  This type is a container for compressed-storage of sparse data.

  @var offsets The offset at which each index begins and ends in the
               list of indices.
  @var indices The indices of the sparse structure of the data resolved
               by this storage member.

  @ingroup coloring
*/

struct crs_t {

  using value_type = size_t;

  std::vector<size_t> offsets;
  std::vector<size_t> indices;

  define_as(offsets) define_as(indices)

    size_t size() const {
    if(offsets.empty())
      return 0;
    else
      return offsets.size() - 1;
  } // size

  /// \brief erase a bunch of ids
  void erase(const std::vector<size_t> & ids) {

    if(ids.empty())
      return;

    // assume sorted
    assert(std::is_sorted(ids.begin(), ids.end()) &&
           "entries to delete are not sorted");

    auto num_remove = ids.size();
    auto num_offsets = size();

    // shift data to the left
    for(auto i = num_remove; i-- > 0;) {
      // get local id to remove
      auto local_id = ids[i];
      // get offset positions
      auto start = offsets[local_id];
      auto end = offsets[local_id + 1];
      auto n = end - start;
      // shift all indices
      for(size_t j = end, k = 0; j < offsets[num_offsets]; ++j, ++k)
        indices[start + k] = indices[j];
      // shift all offsets
      for(auto j = local_id; j < num_offsets; ++j)
        offsets[j] = offsets[j + 1] - n;
      // remove one entry
      num_offsets--;
    }

    // resize arrays
    offsets.resize(num_offsets + 1);
    indices.resize(offsets[num_offsets]);
  }

  /// \brief clears the current storage
  void clear() {
    offsets.clear();
    indices.clear();
  }

  class iterator
  {
    crs_t & data_;
    size_t pos_;

  public:
    iterator(crs_t & data, size_t pos = 0) : data_(data), pos_(pos) {}
    auto operator*() {
      auto i = data_.offsets[pos_];
      auto n = data_.offsets[pos_ + 1] - i;
      return utils::make_array_ref(&data_.indices[i], n);
    }
    auto & operator++() {
      pos_++;
      return *this;
    } // prefix
    auto operator++(int) {
      auto i = *this;
      pos_++;
      return i;
    } // postfix
    bool operator!=(const iterator & it) const {
      return (pos_ != it.pos_ || &data_ != &it.data_);
    }
  };

  class const_iterator
  {
    const crs_t & data_;
    size_t pos_;

  public:
    const_iterator(const crs_t & data, size_t pos = 0)
      : data_(data), pos_(pos) {}
    auto operator*() {
      auto i = data_.offsets[pos_];
      auto n = data_.offsets[pos_ + 1] - i;
      return utils::make_array_ref(&data_.indices[i], n);
    }
    auto & operator++() {
      pos_++;
      return *this;
    } // prefix
    auto operator++(int) {
      auto i = *this;
      pos_++;
      return i;
    } // postfix
    bool operator!=(const const_iterator & it) const {
      return (pos_ != it.pos_ || &data_ != &it.data_);
    }
  };

  auto begin() {
    return iterator(*this);
  }
  auto end() {
    return iterator(*this, size());
  }

  auto begin() const {
    return const_iterator(*this);
  }
  auto end() const {
    return const_iterator(*this, size());
  }

  auto at(size_t i) {
    assert(i < size() && "index out of range");
    return *iterator(*this, i);
  }
  auto at(size_t i) const {
    assert(i < size() && "index out of range");
    return *const_iterator(*this, i);
  }
  auto operator[](size_t i) {
    return *iterator(*this, i);
  }
  auto operator[](size_t i) const {
    return *const_iterator(*this, i);
  }

  template<typename InputIt>
  void append(InputIt first, InputIt last) {
    if(first == last)
      return;
    if(offsets.empty())
      offsets.emplace_back(0);
    offsets.emplace_back(offsets.back() + std::distance(first, last));
    indices.insert(indices.end(), first, last);
  }

  template<typename U>
  void append(const U & value) {
    auto ptr = &value;
    append(ptr, ptr + 1);
  }

  template<typename U>
  void push_back(std::initializer_list<U> init) {
    append(init.begin(), init.end());
  }

  template<typename U, template<typename> class Vector>
  void push_back(const Vector<U> & init) {
    append(init.begin(), init.end());
  }

}; // struct crs_t

/*!
  Helper function to print a crs_t instance.
 */

inline std::ostream &
operator<<(std::ostream & stream, const crs_t & crs) {
  stream << "offsets: ";
  for(auto i : crs.offsets) {
    stream << i << " ";
  } // for
  stream << std::endl;

  stream << "indices: ";
  for(auto i : crs.indices) {
    stream << i << " ";
  } // for

  return stream;
} // operator <<

/*!
 This type is a container for distrinuted compressed-storage of sparse data.

 @var distribution The index ranges for each color.

 @ingroup coloring
 */

struct dcrs_t : public crs_t {
  std::vector<size_t> distribution;

  define_as(distribution)

    /// \brief clears the current storage
    void clear() {
    crs_t::clear();
    distribution.clear();
  }

}; // struct dcrs_t

/*!
 Helper function to print a crs_t instance.
 */

inline std::ostream &
operator<<(std::ostream & stream, const dcrs_t & dcrs) {
  stream << static_cast<const crs_t &>(dcrs) << std::endl;

  stream << "distribution: ";
  for(auto i : dcrs.distribution) {
    stream << i << " ";
  } // for

  return stream;
} // operator <<

#undef define_as

} // namespace coloring
} // namespace flecsi
