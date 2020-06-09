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

#if !defined(__clang__)
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <new>

namespace flecsi {
namespace util {
///////////////////////////////////////////////////////////////////////////
class uint128;

inline bool operator<(const uint128 & a, const uint128 & b) noexcept;
inline bool operator==(const uint128 & a, const uint128 & b) noexcept;
inline bool operator||(const uint128 & a, const uint128 & b) noexcept;
inline bool operator&&(const uint128 & a, const uint128 & b) noexcept;

// GLOBAL OPERATOR INLINES

inline uint128 operator+(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator-(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator*(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator/(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator%(const uint128 & a, const uint128 & b) noexcept;

inline uint128 operator>>(const uint128 & a, unsigned int n) noexcept;
inline uint128 operator<<(const uint128 & a, unsigned int n) noexcept;

inline uint128 operator&(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator|(const uint128 & a, const uint128 & b) noexcept;
inline uint128 operator^(const uint128 & a, const uint128 & b) noexcept;

inline bool operator>(const uint128 & a, const uint128 & b) noexcept;
inline bool operator<=(const uint128 & a, const uint128 & b) noexcept;
inline bool operator>=(const uint128 & a, const uint128 & b) noexcept;
inline bool operator!=(const uint128 & a, const uint128 & b) noexcept;

///////////////////////////////////////////////////////////////////////////
class uint128
{
private:
  // Binary correct representation of unsigned 128bit integer
  std::uint64_t lo;
  std::uint64_t hi;

protected:
  // Some global operator functions must be friends
  friend bool operator<(const uint128 &, const uint128 &) noexcept;
  friend bool operator==(const uint128 &, const uint128 &) noexcept;
  friend bool operator||(const uint128 &, const uint128 &) noexcept;
  friend bool operator&&(const uint128 &, const uint128 &) noexcept;

public:
  // Constructors
  inline uint128() noexcept : lo(0ull), hi(0ull) {}

  inline uint128(int a) noexcept : lo(a), hi(0ull) {}
  inline uint128(unsigned int a) noexcept : lo(a), hi(0ull) {}
  inline uint128(std::uint64_t a) noexcept : lo(a), hi(0ull) {}

  uint128(float a) noexcept
    : lo((std::uint64_t)fmodf(a, 18446744073709551616.0f)),
      hi((std::uint64_t)(a / 18446744073709551616.0f)) {}
  uint128(double a) noexcept
    : lo((std::uint64_t)fmod(a, 18446744073709551616.0)),
      hi((std::uint64_t)(a / 18446744073709551616.0)) {}
  uint128(long double a) noexcept
    : lo((std::uint64_t)fmodl(a, 18446744073709551616.0l)),
      hi((std::uint64_t)(a / 18446744073709551616.0l)) {}

  uint128(const char * sz) noexcept : lo(0u), hi(0u) {
    if(!sz)
      return;
    if(!sz[0])
      return;

    unsigned int radix = 10;
    std::size_t i = 0;
    bool minus = false;

    if(sz[i] == '-') {
      ++i;
      minus = true;
    };

    if(sz[i] == '0') {
      radix = 8;
      ++i;
      if(sz[i] == 'x') {
        radix = 16;
        ++i;
      };
    };

    std::size_t len = strlen(sz);
    for(; i < len; ++i) {
      unsigned int n = 0;
      if(sz[i] >= '0' && sz[i] <= (std::min)(('0' + (int)radix), (int)'9'))
        n = sz[i] - '0';
      else if(sz[i] >= 'a' && sz[i] <= 'a' + (int)radix - 10)
        n = sz[i] - 'a' + 10;
      else if(sz[i] >= 'A' && sz[i] <= 'A' + (int)radix - 10)
        n = sz[i] - 'A' + 10;
      else
        break;

      (*this) *= radix;
      (*this) += n;
    };

    if(minus)
      *this = 0u - *this;
  }

  // TODO: Consider creation of operator= to eliminate
  //       the need of intermediate objects during assignments.

private:
  // Special internal constructors
  uint128(const std::uint64_t & a, const std::uint64_t & b) noexcept
    : lo(a), hi(b) {}

public:
  // Operators
  bool operator!() const noexcept {
    return !(this->hi || this->lo);
  }

  uint128 operator-() const noexcept {
    if(!this->hi && !this->lo)
      // number is 0, just return 0
      return *this;

#pragma warning(push)
#pragma warning(disable : 4146)
    // non 0 number
    return uint128(-this->lo, ~this->hi);
#pragma warning(pop)
  }
  uint128 operator~() const noexcept {
    return uint128(~this->lo, ~this->hi);
  }

  uint128 & operator++() {
    ++this->lo;
    if(!this->lo)
      ++this->hi;

    return *this;
  }
  uint128 & operator--() {
    if(!this->lo)
      --this->hi;
    --this->lo;

    return *this;
  }
  uint128 operator++(int) {
    uint128 b = *this;
    ++*this;

    return b;
  }
  uint128 operator--(int) {
    uint128 b = *this;
    --*this;

    return b;
  }

  uint128 & operator+=(const uint128 & b) noexcept {
    std::uint64_t old_lo = this->lo;

    this->lo += b.lo;
    this->hi += b.hi + (this->lo < old_lo);

    return *this;
  }
  uint128 & operator*=(const uint128 & b) noexcept {
    if(!b)
      return *this = 0u;
    if(b == 1u)
      return *this;

    uint128 a = *this;
    uint128 t = b;

    this->lo = 0ull;
    this->hi = 0ull;

    for(unsigned int i = 0; i < 128; ++i) {
      if(t.lo & 1)
        *this += a << i;

      t >>= 1;
    };

    return *this;
  }

  uint128 & operator>>=(unsigned int n) noexcept {
    n &= 0x7F;

    if(n > 63) {
      n -= 64;
      this->lo = this->hi;
      this->hi = 0ull;
    };

    if(n) {
      // shift low qword
      this->lo >>= n;

      // get lower N bits of high qword
      std::uint64_t mask = 0ull;
      for(unsigned int i = 0; i < n; ++i)
        mask |= (1ll << i);

      // and add them to low qword
      this->lo |= (this->hi & mask) << (64 - n);

      // and finally shift also high qword
      this->hi >>= n;
    };

    return *this;
  }
  uint128 & operator<<=(unsigned int n) noexcept {
    n &= 0x7F;

    if(n > 63) {
      n -= 64;
      this->hi = this->lo;
      this->lo = 0ull;
    };

    if(n) {
      // shift high qword
      this->hi <<= n;

      // get higher N bits of low qword
      std::uint64_t mask = 0ull;
      for(unsigned int i = 0; i < n; ++i)
        mask |= (1ll << (63 - i));

      // and add them to high qword
      this->hi |= (this->lo & mask) >> (64 - n);

      // and finally shift also low qword
      this->lo <<= n;
    };

    return *this;
  }

  uint128 & operator|=(const uint128 & b) noexcept {
    this->hi |= b.hi;
    this->lo |= b.lo;

    return *this;
  }
  uint128 & operator&=(const uint128 & b) noexcept {
    this->hi &= b.hi;
    this->lo &= b.lo;

    return *this;
  }
  uint128 & operator^=(const uint128 & b) noexcept {
    this->hi ^= b.hi;
    this->lo ^= b.lo;

    return *this;
  }

  explicit operator bool() const noexcept {
    return lo || hi;
  }

  // Inline simple operators
  inline const uint128 & operator+() const noexcept {
    return *this;
  };

  // Rest of inline operators
  inline uint128 & operator-=(const uint128 & b) noexcept {
    return *this += (-b);
  };
  inline uint128 & operator/=(const uint128 & b) noexcept {
    uint128 dummy;
    *this = this->div(b, dummy);
    return *this;
  };
  inline uint128 & operator%=(const uint128 & b) noexcept {
    this->div(b, *this);
    return *this;
  };

  explicit operator std::uint64_t() const noexcept {
    return lo;
  }

  // Common methods
  unsigned int toUint() const noexcept {
    return (unsigned int)this->lo;
  };
  std::uint64_t toUint64() const noexcept {
    return (std::uint64_t)this->lo;
  };
  const char * toString(unsigned int radix = 10) const noexcept {
    if(!*this)
      return "0";
    if(radix < 2 || radix > 37)
      return "(invalid radix)";

    static char sz[256];
    std::memset(sz, 0, 256);

    uint128 r;
    uint128 ii = *this;
    int i = 255;

    while(!!ii && i) {
      ii = ii.div(radix, r);
      sz[--i] = r.toUint() + ((r.toUint() > 9) ? 'A' - 10 : '0');
    };

    return &sz[i];
  }
  float toFloat() const noexcept {
    return (float)this->hi * 18446744073709551616.0f + (float)this->lo;
  }
  double toDouble() const noexcept {
    return (double)this->hi * 18446744073709551616.0 + (double)this->lo;
  }
  long double toLongDouble() const noexcept {
    return (long double)this->hi * 18446744073709551616.0l +
           (long double)this->lo;
  }

  // Arithmetic methods
  uint128 div(const uint128 & ds, uint128 & remainder) const noexcept {
    if(!ds)
      return 1u / (unsigned int)ds.lo;

    uint128 dd = *this;

    // only remainder
    if(ds > dd) {
      remainder = *this;
      return std::uint64_t(0ull);
    };

    uint128 r = std::uint64_t(0ull);
    uint128 q = std::uint64_t(0ull);
    //    while (dd >= ds) { dd -= ds; q += 1; }; // extreme slow version

    unsigned int b = 127;
    while(r < ds) {
      r <<= 1;
      if(dd.bit(b--))
        r.lo |= 1;
    };
    ++b;

    while(true)
      if(r < ds) {
        if(!(b--))
          break;

        r <<= 1;
        if(dd.bit(b))
          r.lo |= 1;
      }
      else {
        r -= ds;
        q.bit(b, true);
      };

    remainder = r;
    return q;
  }

  // Bit operations
  bool bit(unsigned int n) const noexcept {
    n &= 0x7F;

    if(n < 64)
      return (this->lo & (1ull << n)) ? true : false;

    return (this->hi & (1ull << (n - 64))) ? true : false;
  }
  void bit(unsigned int n, bool val) noexcept {
    n &= 0x7F;

    if(val) {
      if(n < 64)
        this->lo |= (1ull << n);
      else
        this->hi |= (1ull << (n - 64));
    }
    else {
      if(n < 64)
        this->lo &= ~(1ull << n);
      else
        this->hi &= ~(1ull << (n - 64));
    };
  }
}
#ifdef __GNUC__
__attribute__((__aligned__(16), __packed__))
#endif
;

// GLOBAL OPERATORS

inline bool
operator<(const uint128 & a, const uint128 & b) noexcept {
  return (a.hi == b.hi) ? (a.lo < b.lo) : (a.hi < b.hi);
}
inline bool
operator==(const uint128 & a, const uint128 & b) noexcept {
  return a.hi == b.hi && a.lo == b.lo;
}
inline bool
operator||(const uint128 & a, const uint128 & b) noexcept {
  return (a.hi || a.lo) && (b.hi || b.lo);
}
inline bool
operator&&(const uint128 & a, const uint128 & b) noexcept {
  return (a.hi || a.lo) || (b.hi || b.lo);
}

// GLOBAL OPERATOR INLINES

inline uint128
operator+(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) += b;
};
inline uint128
operator-(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) -= b;
};
inline uint128 operator*(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) *= b;
};
inline uint128
operator/(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) /= b;
};
inline uint128
operator%(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) %= b;
};

inline uint128
operator>>(const uint128 & a, unsigned int n) noexcept {
  return uint128(a) >>= n;
};
inline uint128
operator<<(const uint128 & a, unsigned int n) noexcept {
  return uint128(a) <<= n;
};

inline uint128 operator&(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) &= b;
};
inline uint128
operator|(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) |= b;
};
inline uint128
operator^(const uint128 & a, const uint128 & b) noexcept {
  return uint128(a) ^= b;
};

inline bool
operator>(const uint128 & a, const uint128 & b) noexcept {
  return b < a;
};
inline bool
operator<=(const uint128 & a, const uint128 & b) noexcept {
  return !(b < a);
};
inline bool
operator>=(const uint128 & a, const uint128 & b) noexcept {
  return !(a < b);
};
inline bool
operator!=(const uint128 & a, const uint128 & b) noexcept {
  return !(a == b);
};
} // namespace util
} // namespace flecsi

typedef flecsi::util::uint128 __uint128_t;
#endif
