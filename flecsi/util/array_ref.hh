/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace flecsi {
namespace util {

/// A workalike for std::span from C++20 (only dynamic-extent, without ranges
/// support).
template<class T>
struct span {
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;

  // These two are implementation-defined:
  using iterator = pointer;
  using const_iterator = const_pointer;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr span() noexcept : span(nullptr, nullptr) {}
  constexpr span(pointer p, size_type sz) : span(p, p + sz) {}
  constexpr span(pointer p, pointer q) : p(p), q(q) {}
  template<std::size_t N>
  constexpr span(element_type (&a)[N]) : span(a, N) {}
  /// \warning Destroying \a C leaves this object dangling if it owns its
  ///   elements.  This implementation does not check for "borrowing".
  template<class C,
    class = std::enable_if_t<std::is_convertible_v<
      std::remove_pointer_t<decltype(void(std::size(std::declval<C &&>())),
        std::data(std::declval<C &&>()))> (*)[],
      T (*)[]>>>
  constexpr span(C && c) : span(std::data(c), std::size(c)) {}

  constexpr iterator begin() const noexcept {
    return p;
  }
  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }
  constexpr iterator end() const noexcept {
    return q;
  }
  constexpr const_iterator cend() const noexcept {
    return end();
  }

  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }
  constexpr const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }
  constexpr const_reverse_iterator crend() const noexcept {
    return rend();
  }

  constexpr reference front() const {
    return *begin();
  }
  constexpr reference back() const {
    return end()[-1];
  }

  constexpr reference operator[](size_type i) const {
    return begin()[i];
  }

  constexpr pointer data() const noexcept {
    return begin();
  }

  // FIXME: Spurious overflow for extremely large ranges
  constexpr size_type size() const noexcept {
    return end() - begin();
  }
  constexpr size_type size_bytes() const noexcept {
    return sizeof(element_type) * size();
  }

  constexpr bool empty() const noexcept {
    return begin() == end();
  }

  constexpr span first(size_type n) const {
    return {begin(), n};
  }
  constexpr span last(size_type n) const {
    return {end() - n, n};
  }
  constexpr span subspan(size_type i, size_type n = -1) const {
    return {begin() + i, n == size_type(-1) ? size() - i : n};
  }

private:
  pointer p, q;
};

template<class C>
span(C &)->span<typename C::value_type>;
template<class C>
span(const C &)->span<const typename C::value_type>;

/// Copy a span into a std::vector.
template<class T>
auto
to_vector(span<T> s) {
  // Work around GCC<8 having no deduction guide for vector:
  return std::vector<typename span<T>::value_type>(s.begin(), s.end());
}

/// A very simple emulation of std::ranges::iota_view from C++20.
template<class I>
struct iota_view {
  struct iterator {
    using value_type = I;
    using reference = I;
    using pointer = void;
    using difference_type = I;
    using iterator_category = std::input_iterator_tag;

    constexpr iterator(I i = I()) : i(i) {}

    constexpr I operator*() const {
      return i;
    }
    constexpr I operator[](difference_type n) {
      return i + n;
    }

    constexpr iterator & operator++() {
      ++i;
      return *this;
    }
    constexpr iterator operator++(int) {
      const iterator ret = *this;
      ++*this;
      return ret;
    }
    constexpr iterator & operator--() {
      --i;
      return *this;
    }
    constexpr iterator operator--(int) {
      const iterator ret = *this;
      --*this;
      return ret;
    }
    constexpr iterator & operator+=(difference_type n) {
      i += n;
      return *this;
    }
    friend constexpr iterator operator+(difference_type n, iterator i) {
      i += n;
      return i;
    }
    constexpr iterator operator+(difference_type n) const {
      return n + *this;
    }
    constexpr iterator & operator-=(difference_type n) {
      i -= n;
      return *this;
    }
    constexpr iterator operator-(difference_type n) const {
      iterator ret = *this;
      ret -= n;
      return ret;
    }
    constexpr difference_type operator-(const iterator & r) const {
      return i - r.i;
    }

    constexpr bool operator==(const iterator & r) const noexcept {
      return i == r.i;
    }
    constexpr bool operator!=(const iterator & r) const noexcept {
      return !(*this == r);
    }
    constexpr bool operator<(const iterator & r) const noexcept {
      return i < r.i;
    }
    constexpr bool operator>(const iterator & r) const noexcept {
      return r < *this;
    }
    constexpr bool operator<=(const iterator & r) const noexcept {
      return !(*this > r);
    }
    constexpr bool operator>=(const iterator & r) const noexcept {
      return !(*this < r);
    }

  private:
    I i;
  };

  iota_view() = default;
  constexpr iota_view(I b, I e) : b(b), e(e) {}

  constexpr iterator begin() const noexcept {
    return b;
  }
  constexpr iterator end() const noexcept {
    return e;
  }

  constexpr bool empty() const {
    return b == e;
  }
  constexpr explicit operator bool() const {
    return !empty();
  }

  constexpr auto size() const {
    return e - b;
  }

  constexpr decltype(auto) front() const {
    return *begin();
  }
  constexpr decltype(auto) back() const {
    return *--end();
  }
  constexpr decltype(auto) operator[](I i) const {
    return begin()[i];
  }

private:
  iterator b, e;
};

} // namespace util
} // namespace flecsi
