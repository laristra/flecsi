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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/exec/fold.hh"
#include "flecsi/run/backend.hh"
#include "flecsi/util/demangle.hh"
#include <flecsi/flog.hh>

#include <legion.h>

#include <atomic>
#include <complex>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <type_traits>

namespace flecsi {

inline log::devel_tag reduction_wrapper_tag("reduction_wrapper");

namespace exec {

namespace detail {
template<class R, class = void>
struct identity_traits {
  // GCC rejects this if it's a reference (#97340):
  template<class T>
  static inline const T value{R::template identity<T>};
};
template<class R>
struct identity_traits<R, decltype(void(&R::identity))> {
  template<class T>
  static inline const T & value{R::identity};
};

struct atomic_base {
  // Expecting that concurrent atomic operations are more likely on a single
  // type, we share one large lock array:
  static constexpr std::uintptr_t locks = 255;
  static inline std::mutex lock[locks];

  static auto lower(std::memory_order o) {
    switch(o) {
      case std::memory_order_relaxed:
        return __ATOMIC_RELAXED;
      case std::memory_order_consume:
        return __ATOMIC_CONSUME;
      case std::memory_order_acquire:
        return __ATOMIC_ACQUIRE;
      case std::memory_order_release:
        return __ATOMIC_RELEASE;
      case std::memory_order_acq_rel:
        return __ATOMIC_ACQ_REL;
      default:
        return __ATOMIC_SEQ_CST;
    }
  }
};
// A simple, abridged version of std::atomic_ref from C++20.
template<class T, class = void>
struct atomic_ref : private atomic_base {
  explicit atomic_ref(T & t) : p(&t) {}
  bool compare_exchange_strong(T & expected,
    T desired,
    std::memory_order = {}) const noexcept {
    const std::unique_lock guard(
      lock[reinterpret_cast<std::uintptr_t>(p) % locks]);
    const bool fail = std::memcmp(p, &expected, sizeof(T));
    std::memcpy(fail ? &expected : p, fail ? p : &desired, sizeof(T));
    return !fail;
  }

private:
  T * p;
};
// The real implementation for certain built-in types:
template<class T>
struct atomic_ref<T,
  std::enable_if_t<std::is_pointer_v<T> || std::is_integral_v<T>>>
  : private atomic_base {
  explicit atomic_ref(T & t) : p(&t) {}

  bool compare_exchange_strong(T & expected,
    T desired,
    std::memory_order o = std::memory_order_seq_cst) const noexcept {
    return __atomic_compare_exchange_n(
      p, &expected, desired, false, lower(o), lower([o]() {
        switch(o) {
          case std::memory_order_acq_rel:
            return std::memory_order_acquire;
          case std::memory_order_release:
            return std::memory_order_relaxed;
          default:
            return o;
        }
      }()));
  }

private:
  T * p;
};
} // namespace detail

namespace fold {
inline Legion::ReductionOpID reduction_id;

// Adapts our interface to Legion's.
template<class R, class T, class = void>
struct wrap {
  typedef T LHS, RHS;

  template<bool E>
  static void apply(LHS & a, RHS b) {
    if constexpr(E)
      a = R::combine(a, b);
    else {
      LHS rd{};
      detail::atomic_ref<LHS> r(a);
      while(!r.compare_exchange_strong(
        rd, R::combine(rd, b), std::memory_order_relaxed))
        ;
    }
  }
  // Legion actually requires this additional interface:
  static constexpr const T & identity =
    detail::identity_traits<R>::template value<T>;
  template<bool E>
  static void fold(RHS & a, RHS b) {
    apply<E>(a, b);
  }

private:
  static void init() {
    {
      log::devel_guard guard(reduction_wrapper_tag);
      flog_devel(info) << "registering reduction operation " << util::type<R>()
                       << " for " << util::type<T>() << std::endl;
    }

    // Register the operation with the Legion runtime
    Legion::Runtime::register_reduction_op<wrap>(id);
  }

public:
  // NB: 0 is reserved by Legion.
  static inline const Legion::ReductionOpID id =
    (run::context::instance().register_init(init), ++reduction_id);
};

template<class>
constexpr legion_redop_kind_t redop() = delete;
template<>
constexpr legion_redop_kind_t
redop<min>() {
  return LEGION_REDOP_KIND_MIN;
}
template<>
constexpr legion_redop_kind_t
redop<max>() {
  return LEGION_REDOP_KIND_MAX;
}
template<>
constexpr legion_redop_kind_t
redop<sum>() {
  return LEGION_REDOP_KIND_SUM;
}
template<>
constexpr legion_redop_kind_t
redop<product>() {
  return LEGION_REDOP_KIND_PROD;
}

template<class>
constexpr legion_type_id_t redtype() = delete;
template<>
constexpr legion_type_id_t
redtype<bool>() {
  return LEGION_TYPE_BOOL;
}
template<>
constexpr legion_type_id_t
redtype<std::int8_t>() {
  return LEGION_TYPE_INT8;
}
template<>
constexpr legion_type_id_t
redtype<std::int16_t>() {
  return LEGION_TYPE_INT16;
}
template<>
constexpr legion_type_id_t
redtype<std::int32_t>() {
  return LEGION_TYPE_INT32;
}
template<>
constexpr legion_type_id_t
redtype<std::int64_t>() {
  return LEGION_TYPE_INT64;
}
template<>
constexpr legion_type_id_t
redtype<std::uint8_t>() {
  return LEGION_TYPE_UINT8;
}
template<>
constexpr legion_type_id_t
redtype<std::uint16_t>() {
  return LEGION_TYPE_UINT16;
}
template<>
constexpr legion_type_id_t
redtype<std::uint32_t>() {
  return LEGION_TYPE_UINT32;
}
template<>
constexpr legion_type_id_t
redtype<std::uint64_t>() {
  return LEGION_TYPE_UINT64;
}
template<>
constexpr legion_type_id_t
redtype<float>() {
  return LEGION_TYPE_FLOAT32;
}
template<>
constexpr legion_type_id_t
redtype<double>() {
  return LEGION_TYPE_FLOAT64;
}
template<>
constexpr legion_type_id_t
redtype<std::complex<float>>() {
  return LEGION_TYPE_COMPLEX64;
}
template<>
constexpr legion_type_id_t
redtype<std::complex<double>>() {
  return LEGION_TYPE_COMPLEX128;
}

template<class R, class T>
struct wrap<R, T, decltype(void((redop<R>(), redtype<T>())))> {
  static constexpr Legion::ReductionOpID id =
    LEGION_REDOP_BASE + redop<R>() * LEGION_TYPE_TOTAL + redtype<T>();
};

} // namespace fold
} // namespace exec
} // namespace flecsi
