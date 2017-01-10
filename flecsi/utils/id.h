/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_id_h
#define flecsi_utils_id_h

#include <cassert>
#include <cstdint>
#include <iostream>

/*!
 * \file
 * \date Initial file creation: Feb 12, 2016
 */

namespace flecsi {
namespace utils {

  using local_id_t = __uint128_t;

  template<size_t PBITS, size_t EBITS, size_t FBITS, size_t GBITS>
  class id_
  {
  public:
    static constexpr size_t FLAGS_UNMASK = 
      ~(((size_t(1) << FBITS) - size_t(1)) << 59); 

    static_assert(PBITS + EBITS + FBITS + GBITS + 4 == 128, 
      "invalid id bit configuration");

    id_() = default;
    id_(id_&&) = default;

    id_(const id_& id)
    : dimension_(id.dimension_),
    domain_(id.domain_),
    partition_(id.partition_),
    entity_(id.entity_),
    flags_(id.flags_),
    global_(id.global_) { }

    explicit id_(size_t local_id)
    : dimension_(0),
    domain_(0),
    partition_(0),
    entity_(local_id),
    flags_(0),
    global_(0) { }

    template<size_t D, size_t M>
    static id_ make(size_t local_id,
                    size_t partition_id = 0,
                    size_t flags = 0,
                    size_t global = 0)
    {
      id_ global_id;
      global_id.dimension_ = D;
      global_id.domain_ = M;
      global_id.partition_ = partition_id;
      global_id.entity_ = local_id;
      global_id.global_ = global;
      global_id.flags_ = flags;

      return global_id;
    }

    template<size_t M>
    static id_ make(size_t dim,
                    size_t local_id,
                    size_t partition_id = 0,
                    size_t flags = 0,
                    size_t global = 0)
    {
      id_ global_id;
      global_id.dimension_ = dim;
      global_id.domain_ = M;
      global_id.partition_ = partition_id;
      global_id.entity_ = local_id;
      global_id.global_ = global;
      global_id.flags_ = flags;

      return global_id;
    }

    local_id_t local_id() const
    {
      local_id_t r = dimension_;
      r |= local_id_t(domain_) << 2; 
      r |= local_id_t(partition_) << 4; 
      r |= local_id_t(entity_) << (4 + PBITS);
      return r;
    }

    size_t global_id() const
    {
      constexpr size_t unmask = ~((size_t(1) << EBITS) - 1);
      return (local_id() & unmask) | global_;
    }

    void set_global(size_t global) const
    {
      global_ = global;
    }

    size_t global() const
    {
      return global_;
    }

    void set_partition(size_t partition) const
    {
      partition_ = partition;
    }

    id_& operator=(id_ &&) = default;

    id_& operator=(const id_ &id)
    {
      dimension_ = id.dimension_;
      domain_ = id.domain_;
      partition_ = id.partition_;
      entity_ = id.entity_;
      global_ = id.global_;
      flags_ = id.flags_;

      return *this;
    }

    size_t dimension() const{
      return dimension_;
    }

    size_t domain() const{
      return domain_;
    }

    size_t partition() const{
      return partition_;
    }

    size_t entity() const{
      return entity_;
    }

    size_t index_space_index() const{
      return entity_;
    }

    size_t flags() const{
      return flags_;
    }

    size_t set_flags(size_t flags) {
      assert(flags < 1 << FBITS && "flag bits exceeded");
      flags_ = flags;
    }

    bool operator<(const id_ & id) const{
      return local_id() < id.local_id();
    }

    bool operator==(const id_ & id) const{
      return (local_id() & FLAGS_UNMASK) == (id.local_id() & FLAGS_UNMASK);
    }

    bool operator!=(const id_ & id) const{
      return !(local_id() == id.local_id());
    }

  private:

    size_t dimension_ : 2;
    size_t domain_ : 2;
    size_t partition_ : PBITS;
    size_t entity_ : EBITS;
    size_t flags_ : FBITS;
    size_t global_ : GBITS;
  }; // id_

  inline std::ostream& operator<<(std::ostream& ostr, local_id_t x){
    ostr << uint64_t(x >> 64) << ":" << uint64_t(x);
    return ostr;
  }

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_id_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
