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

/*!
  @file

  This file contains implementations of field accessor types.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/reference.hh"
#include "flecsi/execution.hh"
#include "flecsi/topo/size.hh"
#include "flecsi/util/array_ref.hh"
#include <flecsi/data/field.hh>

#include <algorithm>
#include <memory>
#include <stack>

namespace flecsi {
namespace data {

namespace detail {
template<class A>
void
construct(const A & a) {
  const auto s = a.span();
  std::uninitialized_default_construct(s.begin(), s.end());
}
template<class T, layout L, std::size_t P>
void
destroy_task(typename field<T, L>::template accessor1<P> a) {
  const auto s = a.span();
  std::destroy(s.begin(), s.end());
}
template<std::size_t P,
  class T,
  layout L,
  class Topo,
  typename Topo::index_space S>
void
destroy(const field_reference<T, L, Topo, S> & r) {
  execute<destroy_task<T, L, privilege_repeat(rw, privilege_count(P))>>(r);
}
} // namespace detail

// All accessors are ultimately implemented in terms of those for the raw
// layout, minimizing the amount of backend-specific code required.

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<single, DATA_TYPE, PRIVILEGES> : bind_tag, send_tag {
  using value_type = DATA_TYPE;
  // We don't actually inherit from base_type; we don't want its interface.
  using base_type = accessor<dense, DATA_TYPE, PRIVILEGES>;
  using element_type = typename base_type::element_type;

  explicit accessor(std::size_t s) : base(s) {}
  accessor(const base_type & b) : base(b) {}

  element_type & get() const {
    return base(0);
  } // data
  operator element_type &() const {
    return get();
  } // value

  const accessor & operator=(const DATA_TYPE & value) const {
    return const_cast<accessor &>(*this) = value;
  } // operator=
  accessor & operator=(const DATA_TYPE & value) {
    get() = value;
    return *this;
  } // operator=

  element_type operator->() const {
    static_assert(
      std::is_pointer<value_type>::value, "-> called on non-pointer type");
    return get();
  } // operator->

  base_type & get_base() {
    return base;
  }
  const base_type & get_base() const {
    return base;
  }
  template<class F>
  void send(F && f) {
    f(get_base(), [](const auto & r) { return r.template cast<dense>(); });
  }

private:
  base_type base;
}; // struct accessor

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<raw, DATA_TYPE, PRIVILEGES> : reference_base {
  using value_type = DATA_TYPE;
  using element_type = std::
    conditional_t<privilege_write(PRIVILEGES), value_type, const value_type>;

  explicit accessor(std::size_t f) : reference_base(f) {}

  auto span() const {
    return s;
  }

  void bind(util::span<element_type> x) { // for bind_accessors
    s = x;
  }

private:
  util::span<element_type> s;
}; // struct accessor

template<class T, std::size_t P>
struct accessor<dense, T, P> : accessor<raw, T, P>, send_tag {
  using base_type = accessor<raw, T, P>;
  using base_type::base_type;

  accessor(const base_type & b) : base_type(b) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */
  typename accessor::element_type & operator()(std::size_t index) const {
    const auto s = this->span();
    flog_assert(index < s.size(), "index out of range");
    return s[index];
  } // operator()

  typename accessor::element_type & operator[](std::size_t index) const {
    return this->span()[index];
  }

  base_type & get_base() {
    return *this;
  }
  const base_type & get_base() const {
    return *this;
  }
  template<class F>
  void send(F && f) {
    f(get_base(), [](const auto & r) {
      // TODO: use just one task for all fields
      if constexpr(privilege_discard(P) && !std::is_trivially_destructible_v<T>)
        r.get_region().cleanup(r.fid(), [r] { detail::destroy<P>(r); });
      return r.template cast<raw>();
    });
    if constexpr(privilege_discard(P))
      detail::construct(*this); // no-op on caller side
  }
};

// The offsets privileges are separate because they are writable for mutators
// but read-only for even writable accessors.
template<class T, std::size_t P, std::size_t OP = P>
struct ragged_accessor
  : accessor<raw, T, P>,
    send_tag,
    util::with_index_iterator<const ragged_accessor<T, P, OP>> {
  using base_type = typename ragged_accessor::accessor;
  using typename base_type::element_type;
  using Offsets = accessor<dense, std::size_t, OP>;
  using row = util::span<element_type>;

  using base_type::base_type;
  ragged_accessor(const base_type & b) : base_type(b) {}

  row operator[](std::size_t i) const {
    // Without an extra element, we must store one endpoint implicitly.
    // Storing the end usefully ignores any overallocation.
    return this->span().first(off(i)).subspan(i ? off(i - 1) : 0);
  }
  std::size_t size() const noexcept { // not total!
    return off.span().size();
  }
  std::size_t total() const noexcept {
    const auto s = off.span();
    return s.empty() ? 0 : s.back();
  }

  util::span<element_type> span() const {
    return get_base().span().first(total());
  }

  base_type & get_base() {
    return *this;
  }
  const base_type & get_base() const {
    return *this;
  }

  Offsets & get_offsets() {
    return off;
  }
  const Offsets & get_offsets() const {
    return off;
  }
  template<class F>
  void send(F && f) {
    f(get_base(), [](const auto & r) {
      using R = std::remove_reference_t<decltype(r)>;
      const field_id_t i = r.fid();
      r.get_region().template ghost_copy<P>(r);
      auto & t = r.topology().ragged;
      r.get_region(t).cleanup(
        i,
        [=] {
          if constexpr(!std::is_trivially_destructible_v<T>)
            detail::destroy<P>(r);
        },
        privilege_discard(P));
      // We rely on the fact that field_reference uses only the field ID.
      return field_reference<T,
        raw,
        topo::ragged_topology<typename R::Topology>,
        R::space>({i, 0}, t);
    });
    f(get_offsets(), [](const auto & r) {
      // Disable normal ghost copy of offsets:
      r.get_region().template ghost<privilege_pack<wo, wo>>(r.fid());
      return r.template cast<dense, std::size_t>();
    });
    if constexpr(privilege_discard(P))
      detail::construct(*this); // no-op on caller side
  }

  template<class Topo, typename Topo::index_space S>
  static ragged_accessor parameter(
    const field_reference<T, data::ragged, Topo, S> & r) {
    return exec::replace_argument<base_type>(r.template cast<data::raw>());
  }

private:
  Offsets off{this->identifier()};
};

template<class T, std::size_t P>
struct accessor<ragged, T, P>
  : ragged_accessor<T, P, privilege_repeat(ro, privilege_count(P))> {
  using accessor::ragged_accessor::ragged_accessor;
};

template<class T, std::size_t P>
struct mutator<ragged, T, P>
  : bind_tag, send_tag, util::with_index_iterator<const mutator<ragged, T, P>> {
  static_assert(privilege_write(P) && !privilege_discard(P),
    "mutators require read/write permissions");
  using base_type = ragged_accessor<T, P>;
  struct Overflow {
    std::size_t del;
    std::vector<T> add;
  };
  using TaskBuffer = std::vector<Overflow>;

private:
  using base_row = typename base_type::row;
  struct raw_row {
    using size_type = typename base_row::size_type;

    base_row s;
    Overflow * o;

    T & operator[](size_type i) const {
      const auto b = brk();
      if(i < b)
        return s[i];
      i -= b;
      flog_assert(i < o->add.size(), "index out of range");
      return o->add[i];
    }

    size_type brk() const noexcept {
      return s.size() - o->del;
    }
    size_type size() const noexcept {
      return brk() + o->add.size();
    }

    void destroy(std::size_t skip = 0) const {
      std::destroy(s.begin() + skip, s.begin() + brk());
    }
  };

public:
  struct row : util::with_index_iterator<const row>, private raw_row {
    using value_type = T;
    using typename raw_row::size_type;
    using difference_type = typename base_row::difference_type;
    using typename row::with_index_iterator::iterator;

    row(const raw_row & r) : raw_row(r) {}

    using raw_row::operator[];
    T & front() const {
      return *this->begin();
    }
    T & back() const {
      return this->end()[-1];
    }

    using raw_row::size;
    bool empty() const noexcept {
      return !this->size();
    }
    size_type max_size() const noexcept {
      return o->add.max_size();
    }
    size_type capacity() const noexcept {
      // We can't count the span, since the client might remove all its
      // elements and then work back up to capacity().  The strange result is
      // that size() can be greater than capacity(), but that just means that
      // every operation might invalidate everything.
      return o->add.capacity();
    }
    void reserve(size_type n) const {
      o->add.reserve(n);
    }
    void shrink_to_fit() const { // repacks into span; only basic guarantee
      const size_type mv = std::min(o->add.size(), o->del);
      const auto b = o->add.begin(), e = b + mv;
      std::uninitialized_move(b, e, s.end());
      o->del -= mv;
      if(mv || o->add.size() < o->add.capacity())
        decltype(o->add)(
          std::move_iterator(e), std::move_iterator(o->add.end()))
          .swap(o->add);
    }

    void clear() const noexcept {
      o->add.clear();
      for(auto n = brk(); n--;)
        pop_span();
    }
    iterator insert(iterator i, const T & t) const {
      return put(i, t);
    }
    iterator insert(iterator i, T && t) const {
      return put(i, std::move(t));
    }
    iterator insert(iterator i, size_type n, const T & t) const {
      const auto b = brki();
      auto & a = o->add;
      if(i < b) {
        const size_type
          // used elements assigned from t:
          filla = std::min(n, b.index() - i.index()),
          fillx = n - filla, // other spaces to fill
          fillc = std::min(fillx, o->del), // slack spaces constructed from t
          fillo = fillx - fillc, // overflow copies of t
          // slack spaces move-constructed from used elements:
          mvc = std::min(o->del - fillc, filla);

        // Perform the moves and fills mostly in reverse order.
        T *const s0 = s.data(), *const ip = s0 + i.index(),
                 *const bp = s0 + b.index();
        // FIXME: Is there a way to do just one shift?
        a.insert(a.begin(), fillo, t); // first to reduce the volume of shifting
        a.insert(a.begin() + fillo, bp - (filla - mvc), bp);
        std::uninitialized_move_n(
          bp - filla, mvc, std::uninitialized_fill_n(bp, fillc, t));
        o->del -= fillc + mvc; // before copies that might throw
        std::fill(ip, std::move_backward(ip, bp - filla, bp), t);
      }
      else {
        if(i == b)
          for(; n && o->del; --n) // fill in the gap
            push_span(t);
        a.insert(a.begin(), n, t);
      }
      return i;
    }
    template<class I, class = std::enable_if_t<!std::is_integral_v<I>>>
    iterator insert(iterator i, I a, I b) const {
      // FIXME: use size when available
      for(iterator j = i; a != b; ++a, ++j)
        insert(j, *a);
      return i;
    }
    iterator insert(iterator i, std::initializer_list<T> l) {
      return insert(i, l.begin(), l.end());
    }
    template<class... AA>
    iterator emplace(iterator i, AA &&... aa) const {
      return put<true>(i, std::forward<AA>(aa)...);
    }
    iterator erase(iterator i) const noexcept {
      const auto b = brki();
      if(i < b) {
        std::move(i + 1, b, i);
        pop_span();
      }
      else
        o->add.erase(o->add.begin() + (i - b));
      return i;
    }
    iterator erase(iterator i, iterator j) const noexcept {
      const auto b = brki();
      if(i < b) {
        if(j == i)
          ;
        else if(j <= b) {
          std::move(j, b, i);
          for(auto n = j - i; n--;)
            pop_span();
        }
        else {
          erase(b, j);
          erase(i, b);
        }
      }
      else {
        const auto ab = o->add.begin();
        o->add.erase(ab + (i - b), ab + (j - b));
      }
      return i;
    }
    void push_back(const T & t) const {
      emplace_back(t);
    }
    void push_back(T && t) const {
      emplace_back(std::move(t));
    }
    template<class... AA>
    T & emplace_back(AA &&... aa) const {
      return !o->del || !o->add.empty()
               ? o->add.emplace_back(std::forward<AA>(aa)...)
               : push_span(std::forward<AA>(aa)...);
    }
    void pop_back() const noexcept {
      if(o->add.empty())
        pop_span();
      else
        o->add.pop_back();
    }
    void resize(size_type n) const {
      extend(n);
    }
    void resize(size_type n, const T & t) const {
      extend(n, t);
    }
    // No swap: it would swap the handles, not the contents

  private:
    using raw_row::brk;
    using raw_row::o;
    using raw_row::s;

    auto brki() const noexcept {
      return this->begin() + brk();
    }
    template<bool Destroy = false, class... AA>
    iterator put(iterator i, AA &&... aa) const {
      static_assert(Destroy || sizeof...(AA) == 1);
      const auto b = brki();
      auto & a = o->add;
      if(i < b) {
        auto & last = s[brk() - 1];
        if(o->del)
          push_span(std::move(last));
        else
          a.insert(a.begin(), std::move(last));
        std::move_backward(i, b - 1, b);
        if constexpr(Destroy) {
          auto * p = &*i;
          p->~T();
          new(p) T(std::forward<AA>(aa)...);
        }
        else
          *i = (std::forward<AA>(aa), ...);
      }
      else if(i == b && o->del)
        push_span(std::forward<AA>(aa)...);
      else {
        const auto j = a.begin() + (i - b);
        if constexpr(Destroy)
          a.emplace(j, std::forward<AA>(aa)...);
        else
          a.insert(j, std::forward<AA>(aa)...);
      }
      return i;
    }
    template<class... U> // {} or {const T&}
    void extend(size_type n, U &&... u) const {
      auto sz = this->size();
      if(n <= sz) {
        while(sz-- > n)
          pop_back();
      }
      else {
        reserve(n - (o->add.empty() ? s.size() : 0));

        struct cleanup {
          const row & r;
          size_type sz0;
          bool fail = true;
          ~cleanup() {
            if(fail)
              r.resize(sz0);
          }
        } guard = {*this, sz};

        while(sz++ < n)
          emplace_back(std::forward<U>(u)...);
        guard.fail = false;
      }
    }
    template<class... AA>
    T & push_span(AA &&... aa) const {
      auto & ret = *new(&s[brk()]) T(std::forward<AA>(aa)...);
      --o->del;
      return ret;
    }
    void pop_span() const noexcept {
      ++o->del;
      s[brk()].~T();
    }
  };

  mutator(const base_type & b, const topo::resize::policy & p)
    : acc(b), grow(p) {}

  row operator[](std::size_t i) const {
    return raw_get(i);
  }
  std::size_t size() const noexcept {
    return acc.size();
  }

  base_type & get_base() {
    return acc;
  }
  const base_type & get_base() const {
    return acc;
  }
  auto & get_size() {
    return sz;
  }
  const topo::resize::policy & get_grow() const {
    return grow;
  }
  void buffer(TaskBuffer & b) { // for unbind_accessors
    over = &b;
  }
  template<class F>
  void send(F && f) {
    f(get_base(), [](const auto & r) {
      r.get_partition(r.topology().ragged).resize();
      return r;
    });
    f(get_size(), [](const auto & r) {
      return r.get_partition(r.topology().ragged).sizes();
    });
    if(over)
      over->resize(acc.size()); // no-op on caller side
  }

  void commit() const {
    // To move each element before overwriting it, we propagate moves outward
    // from each place where the movement switches from rightward to leftward.
    const auto all = acc.get_base().span();
    const std::size_t n = size();
    // Read and write cursors.  It would be possible, if ugly, to run cursors
    // backwards for the rightward-moving portions and do without the stack.
    std::size_t is = 0, js = 0, id = 0, jd = 0;
    raw_row rs = raw_get(is), rd = raw_get(id);
    struct work {
      void operator()() const {
        if(assign)
          *w = std::move(*r);
        else
          new(w) T(std::move(*r));
      }
      T *r, *w;
      bool assign;
    };
    std::stack<work> stk;
    const auto back = [&stk] {
      for(; !stk.empty(); stk.pop())
        stk.top()();
    };
    for(;; ++js, ++jd) {
      while(js == rs.size()) { // js indexes the split row
        if(++is == n)
          break;
        rs = raw_get(is);
        js = 0;
      }
      if(is == n)
        break;
      while(jd == rd.s.size()) { // jd indexes the span (including slack space)
        if(++id == n)
          break; // we can write off the end of the last
        else
          rd = raw_get(id);
        jd = 0;
      }
      const auto r = &rs[js], w = rd.s.data() + jd;
      flog_assert(w != all.end(),
        "ragged entries for last " << n - is << " rows overrun allocation for "
                                   << all.size() << " entries");
      if(r != w)
        stk.push({r, w, jd < rd.brk()});
      // Perform this move, and any already queued, if it's not to the right.
      // Offset real elements by one position to come after insertions.
      if(rs.s.data() + std::min(js + 1, rs.brk()) > w)
        back();
    }
    back();
    if(jd < rd.brk())
      rd.destroy(jd);
    while(++id < n)
      raw_get(id).destroy();
    auto & off = acc.get_offsets();
    std::size_t delta = 0; // may be "negative"; aliasing is implausible
    for(is = 0; is < n; ++is) {
      auto & ov = (*over)[is];
      off(is) += delta += ov.add.size() - ov.del;
      ov.add.clear();
    }
    sz = data::partition::make_row(
      run::context::instance().color(), grow(acc.total(), all.size()));
  }

private:
  raw_row raw_get(std::size_t i) const {
    return {get_base()[i], &(*over)[i]};
  }

  base_type acc;
  topo::resize::accessor<wo> sz;
  topo::resize::policy grow;
  TaskBuffer * over = nullptr;
};

// Many compilers incorrectly require the 'template' for a base class.
template<class T, std::size_t P>
struct accessor<sparse, T, P>
  : field<T, sparse>::base_type::template accessor1<P>,
    util::with_index_iterator<const accessor<sparse, T, P>> {
  static_assert(!privilege_discard(P),
    "sparse accessor requires read permission");

private:
  using FieldBase = typename field<T, sparse>::base_type;

public:
  using value_type = typename FieldBase::value_type;
  using base_type = typename FieldBase::template accessor1<P>;
  using element_type =
    std::tuple_element_t<1, typename base_type::element_type>;

private:
  using base_row = typename base_type::row;

public:
  // Use the ragged interface to obtain the span directly.
  struct row {
    row(base_row s) : s(s) {}
    element_type & operator()(std::size_t c) const {
      return std::partition_point(
        s.begin(), s.end(), [c](const value_type & v) { return v.first < c; })
        ->second;
    }

  private:
    base_row s;
  };

  using base_type::base_type;
  accessor(const base_type & b) : base_type(b) {}

  row operator[](std::size_t i) const {
    return get_base()[i];
  }

  base_type & get_base() {
    return *this;
  }
  const base_type & get_base() const {
    return *this;
  }
  template<class F>
  void send(F && f) {
    f(get_base(),
      [](const auto & r) { return r.template cast<ragged, value_type>(); });
  }
};

template<class T, std::size_t P>
struct mutator<sparse, T, P>
  : bind_tag, send_tag, util::with_index_iterator<const mutator<sparse, T, P>> {
  using base_type = typename field<T, sparse>::base_type::template mutator1<P>;
  using TaskBuffer = typename base_type::TaskBuffer;

private:
  using base_row = typename base_type::row;
  using base_iterator = typename base_row::iterator;

public:
  struct row {
    using key_type = std::size_t;
    using value_type = typename base_row::value_type;
    using size_type = std::size_t;

    // NB: invalidated by insertions/deletions, unlike std::map::iterator.
    struct iterator {
    public:
      using value_type = std::pair<const key_type &, T &>;
      using difference_type = std::ptrdiff_t;
      using reference = value_type;
      using pointer = void;
      // We could easily implement random access, but std::map doesn't.
      using iterator_category = std::bidirectional_iterator_tag;

      iterator(base_iterator i = {}) : i(i) {}

      value_type operator*() const {
        return {i->first, i->second};
      }

      iterator & operator++() {
        ++i;
        return *this;
      }
      iterator operator++(int) {
        iterator ret = *this;
        ++*this;
        return ret;
      }
      iterator & operator--() {
        --i;
        return *this;
      }
      iterator operator--(int) {
        iterator ret = *this;
        --*this;
        return ret;
      }

      bool operator==(const iterator & o) const {
        return i == o.i;
      }
      bool operator!=(const iterator & o) const {
        return i != o.i;
      }

      base_iterator get_base() const {
        return i;
      }

    private:
      base_iterator i;
    };

    row(base_row r) : r(r) {}

    T & operator[](key_type c) const {
      return try_emplace(c).first->second;
    }

    iterator begin() const noexcept {
      return r.begin();
    }
    iterator end() const noexcept {
      return r.end();
    }

    bool empty() const noexcept {
      return r.empty();
    }
    size_type size() const noexcept {
      return r.size();
    }
    size_type max_size() const noexcept {
      return r.max_size();
    }

    void clear() const noexcept {
      r.clear();
    }
    std::pair<iterator, bool> insert(const value_type & p) const {
      auto [i, hit] = lookup(p.first);
      if(!hit)
        i = r.insert(i, p); // assignment is no-op
      return {i, !hit};
    }
    // TODO: insert(U&&), insert(value_type&&)
    template<class I>
    void insert(I a, I b) const {
      for(; a != b; ++a)
        insert(*a);
    }
    void insert(std::initializer_list<value_type> l) const {
      insert(l.begin(), l.end());
    }
    template<class U>
    std::pair<iterator, bool> insert_or_assign(key_type c, U && u) const {
      auto [i, hit] = lookup(c);
      if(hit)
        i->second = std::forward<U>(u);
      else
        i = r.insert(i, {c, std::forward<U>(u)}); // assignment is no-op
      return {i, !hit};
    }
    // We don't support emplace since we can't avoid moving the result.
    template<class... AA>
    std::pair<iterator, bool> try_emplace(key_type c, AA &&... aa) const {
      auto [i, hit] = lookup(c);
      if(!hit)
        i = r.insert(i,
          {std::piecewise_construct,
            std::make_tuple(c),
            std::forward_as_tuple(
              std::forward<AA>(aa)...)}); // assignment is no-op
      return {i, !hit};
    }

    iterator erase(iterator i) const {
      return r.erase(i.get_base());
    }
    iterator erase(iterator i, iterator j) const {
      return r.erase(i.get_base(), j.get_base());
    }
    size_type erase(key_type c) const {
      const auto [i, hit] = lookup(c);
      if(hit)
        r.erase(i);
      return hit;
    }
    // No swap: it would swap the handles, not the contents

    size_type count(key_type c) const {
      return lookup(c).second;
    }
    iterator find(key_type c) const {
      const auto [i, hit] = lookup(c);
      return hit ? i : end();
    }
    std::pair<iterator, iterator> equal_range(key_type c) const {
      const auto [i, hit] = lookup(c);
      return {i, i + hit};
    }
    iterator lower_bound(key_type c) const {
      return lower(c);
    }
    iterator upper_bound(key_type c) const {
      const auto [i, hit] = lookup(c);
      return i + hit;
    }

  private:
    base_iterator lower(key_type c) const {
      return std::partition_point(
        r.begin(), r.end(), [c](const value_type & v) { return v.first < c; });
    }
    std::pair<base_iterator, bool> lookup(key_type c) const {
      const auto i = lower(c);
      return {i, i != r.end() && i->first == c};
    }

    // We simply keep the (ragged) row sorted; this avoids the complexity of
    // two lookaside structures and is efficient for small numbers of inserted
    // elements and for in-order initialization.
    base_row r;
  };

  mutator(const base_type & b) : rag(b) {}

  row operator[](std::size_t i) const {
    return get_base()[i];
  }
  std::size_t size() const noexcept {
    return rag.size();
  }

  base_type & get_base() {
    return rag;
  }
  const base_type & get_base() const {
    return rag;
  }
  template<class F>
  void send(F && f) {
    f(get_base(), [](const auto & r) {
      return r.template cast<ragged, typename base_row::value_type>();
    });
  }
  void buffer(TaskBuffer & b) { // for unbind_accessors
    rag.buffer(b);
  }

  void commit() const {
    rag.commit();
  }

private:
  base_type rag;
};

} // namespace data

template<data::layout L, class T, std::size_t P>
struct exec::detail::task_param<data::accessor<L, T, P>> {
  template<class Topo, typename Topo::index_space S>
  static auto replace(const data::field_reference<T, L, Topo, S> & r) {
    return data::accessor<L, T, P>(r.fid());
  }
};
template<class T, std::size_t P>
struct exec::detail::task_param<data::mutator<data::ragged, T, P>> {
  using type = data::mutator<data::ragged, T, P>;
  template<class Topo, typename Topo::index_space S>
  static type replace(
    const data::field_reference<T, data::ragged, Topo, S> & r) {
    return {type::base_type::parameter(r),
      r.get_partition(r.topology().ragged).growth};
  }
};
template<class T, std::size_t P>
struct exec::detail::task_param<data::mutator<data::sparse, T, P>> {
  using type = data::mutator<data::sparse, T, P>;
  template<class Topo, typename Topo::index_space S>
  static type replace(
    const data::field_reference<T, data::sparse, Topo, S> & r) {
    return exec::replace_argument<typename type::base_type>(
      r.template cast<data::ragged,
        typename field<T, data::sparse>::base_type::value_type>());
  }
};

} // namespace flecsi
