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

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <cassert>

namespace flecsi {
namespace utils {

/*!
 The bit_buffer__ type provides a bit-addressable buffer which can set
 and retrivie arbitrary bit collections which may potentially span multiple
 words of its underlying storage. Unlike std::bitset, it can address multiple
 bits efficiently, e.g. bits 0-3 of index 7 and its size does not have to
 be configured at compile-time.

 @ingroup utils
 */

  template<
    typename T,
    size_t BITS_PER_INDEX
  >
  class bit_buffer__
  {
  public:
    static constexpr size_t word_bits = sizeof(T) * 8;

    static_assert(BITS_PER_INDEX <= word_bits, "invalid bit buffer params");

    class range_proxy__{
    public:
      range_proxy__(
        bit_buffer__& b,
        size_t index,
        size_t bit_start,
        size_t bit_end
      )
      : b_(b),
      index_(index),
      bit_start_(bit_start),
      bit_end_(bit_end){}

      range_proxy__&
      operator=(
        const T& value
      )
      {
        b_.set_(index_, bit_start_, bit_end_, value);
        return *this;
      }

      operator T()
      {
        return b_.get_(index_, bit_start_, bit_end_);
      }

    private:
      bit_buffer__& b_;
      size_t index_;
      size_t bit_start_;
      size_t bit_end_;
    };

    class proxy__{
    public:
      proxy__(
        bit_buffer__& b,
        size_t index,
        size_t bit
      )
      : b_(b),
      index_(index),
      bit_(bit){}

      proxy__&
      operator=(
        const T& value
      )
      {
        b_.set_(index_, bit_, value);
        return *this;
      }

      operator T()
      {
        return b_.get_(index_, bit_);
      }

    private:
      bit_buffer__& b_;
      size_t index_;
      size_t bit_;
    };

    bit_buffer__(
      uint8_t* buffer
    )
    : buffer_(buffer){}

    proxy__
    operator()(
      size_t index,
    )
    {
      return proxy__(*this, index, 0);
    }

    proxy__
    operator()(
      size_t index,
      size_t bit
    )
    {
      return proxy__(*this, index, bit);
    }

    range_proxy__
    operator()(
      size_t index,
      size_t bit_start,
      size_t bit_end
    )
    {
      assert(bit_start <= bit_end && bit_start < BITS_PER_INDEX &&
             "invalid index");

      return range_proxy__(*this, index, bit_start, bit_end);
    }

    void
    set_(
      size_t index,
      size_t bit_start,
      size_t bit_end,
      T value
    )
    {
      size_t s = index * BITS_PER_INDEX;
      size_t i1 = (s + bit_start) / word_bits;
      size_t i2 = (s + bit_end) / word_bits;

      if(i1 == i2){
        size_t r = (s + bit_start) % word_bits;

        T* v = reinterpret_cast<T*>(buffer_ + i1);
        *v &= ~(((1 << (bit_end - bit_start + 1)) - 1) << r);
        *v |= (value << r);       
      }
      else{
        size_t r1 = (s + bit_start) % word_bits;
        size_t r2 = word_bits - r1 - 1;

        T* v1 = reinterpret_cast<T*>(buffer_ + i1);
        *v1 &= ~(((1 << word_bits - r1) - 1) << r1);
        *v1 |= (value << r1);

        T* v2 = reinterpret_cast<T*>(buffer_ + i2);
        *v2 &= ~((1 << r2) - 1);
        *v2 |= (value >> (word_bits - r1));     
      }
    }

    void
    set_(
      size_t index,
      size_t bit,
      T value
    )
    {
      size_t s = index * BITS_PER_INDEX + bit;
      size_t i = s / word_bits;
      size_t r = s % word_bits;

      T* v = reinterpret_cast<T*>(buffer_ + i);
      *v &= ~(((1 << (bit + 1)) - 1) << r);
      *v |= (value << r);       
    }

    T
    get_(
      size_t index,
      size_t bit_start,
      size_t bit_end
      )
    {
      size_t s = index * BITS_PER_INDEX;
      size_t i1 = (s + bit_start) / word_bits;
      size_t i2 = (s + bit_end) / word_bits;

      if(i1 == i2){
        size_t r = (s + bit_start) % word_bits;

        T* v = reinterpret_cast<T*>(buffer_ + i1);
        return (*v >> r) & ((1 << (bit_end - bit_start + 1)) - 1);
      }
      else{
        size_t r1 = (s + bit_start) % word_bits;
        size_t r2 = bit_end - bit_start - (word_bits - r1) + 1;

        T* v1 = reinterpret_cast<T*>(buffer_ + i1);
        T* v2 = reinterpret_cast<T*>(buffer_ + i2);

        return ((*v1 >> r1) & ((1 << word_bits - r1) - 1)) | 
          ((*v2 & ((1 << r2) - 1)) << (word_bits - r1));
      }
    }

    T
    get_(
      size_t index,
      size_t bit
      )
    {
      size_t s = index * BITS_PER_INDEX + bit;
      size_t i = s / word_bits;
      size_t r = s % word_bits;

      T* v = reinterpret_cast<T*>(buffer_ + i);
      return (*v >> r) & ((1 << (bit + 1)) - 1);
    }

    void
    dump(
      size_t size
    )
    {
      for(size_t i = 0; i < size; ++i){
        std::cout << i << ": " << std::bitset<word_bits>(buffer_[i]) << std::endl;
      }
    }

  private:
    T* buffer_;
  };

} // namespace utils
} // namespace flecsi
