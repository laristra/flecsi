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

#include <flecsi-config.h>

#include "flecsi/flog.hh"

#include <array>
#include <deque>
#include <limits>

namespace flecsi {
namespace util {

namespace cache_impl {

template<std::size_t Bytes>
struct block {
  uint64_t tag = std::numeric_limits<uint64_t>::max();
  std::array<std::byte, Bytes> data;
}; // struct block

template<std::size_t Capacity, std::size_t Blocks>
struct data {

  static_assert((Capacity & (Capacity - 1)) == 0,
    "Capacity must be a power of 2");
  static_assert(Capacity % Blocks == 0,
    "Capacity must be evenly divisible by Blocks");

protected:
  static constexpr std::size_t blocksize = Capacity / Blocks;
  std::array<block<blocksize>, Blocks> data_;
}; // struct data

struct entry {
  std::size_t tag;
  std::size_t index;
}; // struct entry

} // namespace cache_impl

template<typename Type,
  typename Device,
  std::size_t Capacity,
  std::size_t Blocks>
struct cache : cache_impl::data<Capacity, Blocks> {
  using Base = cache_impl::data<Capacity, Blocks>;

  cache(Device const & device) : device_(device), entries_(Blocks) {}

  Type const & operator()(std::size_t const & offset) {
    flog_assert(offset < device_.template entries<Type>(),
      "invalid offset(" << offset << ") into entries("
                        << device_.template entries<Type>() << ")");

    ++queries_;

    std::size_t tag =
      (offset * sizeof(Type) / Base::blocksize) * Base::blocksize;
    auto hit = std::find(entries_.begin(), entries_.end(), tag);

    if(hit != entries_.end()) {
      ++hits_;
      entries_.erase(hit);
      entries_.push_front(*hit);
      // return reinterpret_cast<Type>(this->data_[hit->index][]);
      return {};
    }
    else {
      entries_.pop_back();
      // add
      return operator()(offset);
    }
  }

private:
  std::size_t queries_{0};
  std::size_t hits_{0};

  Device const & device_;
  std::deque<cache_impl::entry> entries_;
}; // struct cache

#if 0
template<typename Type,
  typename Device,
  std::size_t Capacity,
  std::size_t Blocks,
  std::size_t Associativity,
  template<typename> Policy,
  typename >
struct cache : Policy<Capacity, Blocks> {

  cache(Device const & device, std::size_t entries)
    : device_(device), entries_(entries) {}

  Type const & operator() (std::size_t const & offset) {
    (void)t;
    return {};
  }

private:
  std::size_t queries_;
  std::size_t hits_;

  std::size_t entries_;
  Device const & device_;
};
#endif

} // namespace util
} // namespace flecsi
