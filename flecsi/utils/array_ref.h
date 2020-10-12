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

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <flecsi/utils/target.h>

namespace flecsi {
namespace utils {

/// An emulation of std::to_address from C++20.
template<class T>
constexpr T *
to_address(T * p) noexcept {
  static_assert(!std::is_function_v<T>, "not an object pointer type");
  return p;
}
template<class T>
auto
to_address(const T & p) noexcept {
  return to_address(p.operator->());
}

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
  template<class C,
    class = std::enable_if_t<std::is_convertible_v<
      std::remove_pointer_t<decltype(void(std::size(std::declval<C &>())),
        std::data(std::declval<C &>()))> (*)[],
      T (*)[]>>>
  constexpr span(C & c) : span(std::data(c), std::size(c)) {}

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
    return {begin() + i, n == -1 ? size() - i : n};
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

/// A non-owning minimal std::vector interface backed by a fixed-size array.
/// \warning The destructor does not destroy any elements; call \c clear first
///   or call \c size to discover how many to destroy later.
template<class T>
struct vector_ref {
private:
  using span = utils::span<T>;

public:
  using value_type = typename span::value_type;
  using size_type = typename span::size_type;
  using difference_type = typename span::difference_type;
  using reference = typename span::reference;
  using const_reference = typename span::const_reference;
  using pointer = typename span::pointer;
  using const_pointer = typename span::const_pointer;
  using iterator = typename span::iterator;
  using const_iterator = typename span::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /// Wrap an array.  Elements past the size are taken to be out of lifetime.
  /// \param s underlying array
  /// \param sz number of existing elements
  constexpr vector_ref(span s = {}, size_type sz = 0)
    : s(s), e(s.begin() + sz) {}

  constexpr void assign(size_type n, const value_type & v) {
    reserve(n);
    clear();
    while(n--)
      append(v);
  }
  template<class I, class = std::enable_if_t<!std::is_integral_v<I>>>
  constexpr void assign(I a, I b) {
    clear();
    for(; a != b; ++a)
      append(*a);
  }

  constexpr reference at(size_type i) const {
    if(i >= size())
      throw std::out_of_range("vector_ref::at");
    return (*this)[i];
  }
  constexpr reference operator[](size_type i) const {
    return s[i];
  }
  constexpr reference front() const {
    return *begin();
  }
  constexpr reference back() const {
    return end()[-1]; // not s.back()!
  }

  constexpr pointer data() const noexcept {
    return s.data();
  }

  constexpr iterator begin() const noexcept {
    return s.begin();
  }
  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }
  constexpr iterator end() const noexcept {
    return e;
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

  constexpr bool empty() const noexcept {
    return begin() == end();
  }
  // FIXME: Spurious overflow for inordinately large containers
  constexpr size_type size() const noexcept {
    return end() - begin();
  }
  constexpr size_type max_size() const noexcept {
    return capacity();
  }
  constexpr void reserve(size_type n) {
    if(n > capacity())
      throw std::length_error("vector_ref overfull");
  }
  constexpr size_type capacity() const noexcept {
    return s.size();
  }
  constexpr void shrink_to_fit() noexcept {}

  constexpr void clear() noexcept {
    while(!empty())
      pop_back();
  }

  constexpr iterator insert(const_iterator ci, const value_type & v) {
    return put(ci, v);
  }
  constexpr iterator insert(const_iterator ci, value_type && v) {
    return put(ci, std::move(v));
  }
  iterator insert(const_iterator ci, size_type n, const value_type & v) {
    reserve(size() + n);
    const iterator i = write(ci);
    const size_type mv = e - i, nf = std::min(n, mv);
    std::uninitialized_fill_n(e, n - nf, v); // fill the gap first
    slide(i, n);
    std::fill_n(i, nf, v);
    return i;
  }
  template<class I, class = std::enable_if_t<!std::is_integral_v<I>>>
  iterator insert(const_iterator ci, I a, I b) {
    const iterator i = write(ci);
    if constexpr(std::is_base_of_v<std::forward_iterator_tag,
                   typename std::iterator_traits<I>::iterator_category>) {
      const size_type n = std::distance(a, b);
      reserve(size() + n);
      const auto mid = std::next(a, std::min<size_type>(e - i, n));
      std::uninitialized_copy(mid, b, e); // fill the gap first
      slide(i, n);
      std::copy(a, mid, i);
    }
    else
      for(; a != b; ++a)
        insert(i, *a); // no size available
    return i;
  }
  constexpr void push_back(const value_type & v) {
    append(v);
  }
  constexpr void push_back(value_type && v) {
    append(std::move(v));
  }
  constexpr void pop_back() {
    (--e)->~value_type();
  }

  struct cleanup {
    vector_ref & v;
    size_type sz0;
    bool fail = true;
    ~cleanup() {
      if(fail)
        v.resize(sz0);
    }
  };

  constexpr void resize(size_type n) {
    reserve(n);
    auto sz = size();
    cleanup guard = {*this, sz};

    for(; sz > n; --sz)
      pop_back();
    // uninitialized_value_construct isn't constexpr:
    for(; sz < n; ++sz, ++e)
      new(e) value_type();
    guard.fail = false;
  }

  constexpr void swap(vector_ref & v) noexcept {
    std::swap(s, v.s);
    std::swap(e, v.e);
  }

private:
  template<class U>
  constexpr void append(U && u) {
    if(e == s.end())
      throw std::length_error("vector_ref full");
    new(e) value_type(std::forward<U>(u));
    ++e; // on success only
  }
  template<class U>
  constexpr iterator put(const_iterator ci, U && u) {
    const iterator i = write(ci);
    if(i == end())
      append(std::forward<U>(u));
    else {
      reserve(size() + 1);
      slide(i, 1);
      *i = std::forward<U>(u);
    }
    return i;
  }
  // Move elements to make space for n elements starting from i.  [end(),i+n)
  // must already have been populated for exception safety; end() is adjusted
  // here.  Note that std::uninitialized_move is not constexpr.
  void slide(iterator i, size_type n) {
    if(!n)
      return; // std::move_backward doesn't allow no-op usage
    const size_type mv = e - i, nun = std::min(n, mv);
    // Move some elements into the uninitialized space:
    const iterator um = e - nun;
    e += n - nun; // the caller already filled the gap
    std::uninitialized_move_n(um, nun, um + n);
    e += nun; // even in case of subsequent failure
    std::move_backward(i, um, um + n); // never writes past the original e
  }
  constexpr static iterator write(const_iterator i) {
    return const_cast<iterator>(i);
  }

  span s;
  iterator e; // past the last existing element
};

/// A very simple emulation of std::ranges::transform_view from C++20.
template<class I, class F>
struct transform_view {
  struct iterator {
  private:
    using traits = std::iterator_traits<I>;

  public:
    using difference_type = typename traits::difference_type;
    // TODO: notice a reference return from F and upgrade iterator_category
    using reference = decltype(
      std::declval<const F &>()(std::declval<typename traits::reference>()));
    using value_type = std::decay_t<reference>;
    using pointer = void;
    // We provide all the operators, but we don't assume a real reference:
    using iterator_category = std::input_iterator_tag;

    constexpr iterator() noexcept
      : iterator({}, nullptr) {} // null F won't be used
    constexpr iterator(I p, const F * f) noexcept : p(p), f(f) {}

    constexpr iterator & operator++() {
      ++p;
      return *this;
    }
    constexpr iterator operator++(int) {
      const iterator ret = *this;
      ++*this;
      return ret;
    }
    constexpr iterator & operator--() {
      --p;
      return *this;
    }
    constexpr iterator operator--(int) {
      const iterator ret = *this;
      --*this;
      return ret;
    }
    constexpr iterator & operator+=(difference_type n) {
      p += n;
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
      p -= n;
      return *this;
    }
    constexpr iterator operator-(difference_type n) const {
      iterator ret = *this;
      ret -= n;
      return ret;
    }
    constexpr difference_type operator-(const iterator & i) const {
      return p - i.p;
    }

    constexpr bool operator==(const iterator & i) const noexcept {
      return p == i.p;
    }
    constexpr bool operator!=(const iterator & i) const noexcept {
      return !(*this == i);
    }
    constexpr bool operator<(const iterator & i) const noexcept {
      return p < i.p;
    }
    constexpr bool operator>(const iterator & i) const noexcept {
      return i < *this;
    }
    constexpr bool operator<=(const iterator & i) const noexcept {
      return !(*this > i);
    }
    constexpr bool operator>=(const iterator & i) const noexcept {
      return !(*this < i);
    }

    constexpr reference operator*() const {
      return (*f)(*p);
    }
    // operator-> makes sense only for a true 'reference'
    constexpr reference operator[](difference_type n) const {
      return *(*this + n);
    }

  private:
    I p;
    const F * f;
  };

  /// Wrap an iterator pair.
  constexpr transform_view(I b, I e, F f = {})
    : b(std::move(b)), e(std::move(e)), f(std::move(f)) {}
  /// Wrap a container.
  /// \warning Destroying \a C invalidates this object if it owns its
  ///   iterators or elements.  This implementation does not copy \a C if it
  ///   is a view.
  template<class C,
    class = std::enable_if_t<
      std::is_convertible_v<decltype(std::begin(std::declval<C &>())), I>>>
  constexpr transform_view(C && c, F f = {})
    : transform_view(std::begin(c), std::end(c), std::move(f)) {}

  constexpr iterator begin() const noexcept {
    return {b, &f};
  }
  constexpr iterator end() const noexcept {
    return {e, &f};
  }

  constexpr bool empty() const {
    return b == e;
  }
  constexpr explicit operator bool() const {
    return !empty();
  }

  constexpr auto size() const {
    return std::distance(b, e);
  }

  constexpr decltype(auto) front() const {
    return *begin();
  }
  constexpr decltype(auto) back() const {
    return *--end();
  }
  constexpr decltype(auto) operator[](
    typename std::iterator_traits<I>::difference_type i) const {
    return begin()[i];
  }

private:
  I b, e;
  F f;
};

template<class C, class F>
FLECSI_TARGET transform_view(C &&, F)
  ->transform_view<typename std::remove_reference_t<C>::iterator, F>;
template<class C, class F>
FLECSI_TARGET transform_view(const C &, F)
  ->transform_view<typename C::const_iterator, F>;

/// A very simple emulation of std::ranges::filter_view from C++20.
template<class I, class F>
struct filter_view {
  struct iterator {
  private:
    using traits = std::iterator_traits<I>;

  public:
    using difference_type = typename traits::difference_type;
    using value_type = typename traits::value_type;
    using pointer = typename traits::pointer;
    using reference = typename traits::reference;
    // TODO: bidirectional
    using iterator_category =
      std::common_type_t<typename traits::iterator_category,
        std::forward_iterator_tag>;

    constexpr iterator() noexcept
      : iterator({}, nullptr) {} // null F won't be used
    constexpr iterator(I p, const filter_view * f) noexcept : p(p), f(f) {}

    constexpr iterator & operator++() {
      f->skip(++p);
      return *this;
    }
    constexpr iterator operator++(int) {
      const iterator ret = *this;
      ++*this;
      return ret;
    }

    constexpr bool operator==(const iterator & i) const noexcept {
      return p == i.p;
    }
    constexpr bool operator!=(const iterator & i) const noexcept {
      return !(*this == i);
    }
    constexpr bool operator<(const iterator & i) const noexcept {
      return p < i.p;
    }
    constexpr bool operator>(const iterator & i) const noexcept {
      return i < *this;
    }
    constexpr bool operator<=(const iterator & i) const noexcept {
      return !(*this > i);
    }
    constexpr bool operator>=(const iterator & i) const noexcept {
      return !(*this < i);
    }

    constexpr reference operator*() const {
      return *p;
    }
    constexpr pointer operator->() const {
      return to_address(p);
    }

  private:
    I p;
    const filter_view * f;
  };

  /// Wrap an iterator pair.
  constexpr filter_view(I b, I e, F f = {})
    : b(std::move(b)), e(std::move(e)), f(std::move(f)) {
    skip(this->b);
  }
  /// Wrap a container.
  /// \warning Destroying \a C invalidates this object if it owns its
  ///   iterators or elements.  This implementation does not copy \a C if it
  ///   is a view.
  template<class C,
    class = std::enable_if_t<
      std::is_convertible_v<decltype(std::begin(std::declval<C &>())), I>>>
  constexpr filter_view(C && c, F f = {})
    : filter_view(std::begin(c), std::end(c), std::move(f)) {}

  constexpr iterator begin() const noexcept {
    return {b, this};
  }
  constexpr iterator end() const noexcept {
    return {e, this};
  }

  constexpr auto size() const {
    return std::distance(b, e);
  }

private:
  constexpr void skip(I & i) const {
    while(i != e && !f(*i))
      ++i;
  }

  I b, e;
  F f;
};

template<class C, class F>
filter_view(C &&, F)
  ->filter_view<typename std::remove_reference_t<C>::iterator, F>;
template<class C, class F>
filter_view(const C &, F)->filter_view<typename C::const_iterator, F>;

} // namespace utils
} // namespace flecsi
