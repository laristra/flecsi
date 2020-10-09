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

// initialize with an explicit loop to circumvent integral conversion warnings
#define define_as(member)                                                      \
  template<typename T>                                                         \
  std::vector<T> member##_as() const {                                         \
    std::vector<T> asvec;                                                      \
    asvec.reserve(member.size());                                              \
    for(auto v : member)                                                       \
      asvec.push_back(static_cast<T>(v));                                      \
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

    std::vector<size_t> new_offsets(offsets.size() - num_remove);
    std::vector<size_t> new_indices;
    new_indices.reserve(indices.size());

    new_offsets[0] = 0;
    auto delete_it = ids.begin();

    for(size_t iold = 0, inew = 0; iold < num_offsets; ++iold) {

      // skip deleted items
      if(delete_it != ids.end()) {
        if(*delete_it == iold) {
          delete_it++;
          continue;
        }
      }

      // keep otherwise
      auto start = offsets[iold];
      auto end = offsets[iold + 1];
      for(auto j = start; j < end; ++j) {
        new_indices.push_back(indices[j]);
      }
      new_offsets[inew + 1] = new_indices.size();
      inew++;
    }

    assert(new_offsets.size() == offsets.size() - num_remove);

    // resize arrays
    std::swap(offsets, new_offsets);
    std::swap(indices, new_indices);
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
      return utils::span(&data_.indices[i], n);
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
      return utils::span(&data_.indices[i], n);
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

  template<typename U,
    typename... Args,
    template<typename, typename...>
    class Vector>
  void push_back(const Vector<U, Args...> & init) {
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
 This type is a container for distributed compressed-storage of sparse data.

 @var distribution The index ranges for each color.

 @ingroup coloring
 */

struct dcrs_t : public crs_t {
  std::vector<size_t> distribution;

  define_as(distribution);

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
