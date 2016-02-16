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

#ifndef flecsi_id_h
#define flecsi_id_h

#include <cstdint>

/*!
 * \file id.h
 * \authors nickm, bergen
 * \date Initial file creation: Feb 12, 2016
 */

namespace flecsi
{

  template<size_t PBITS, size_t EBITS>
  class id_
  {
  public:

    static_assert(PBITS + EBITS == sizeof(size_t) * 8 - 4, 
                  "invalid id bit configuration");

    id_() { }

    id_(const id_& id)
    : dimension_(id.dimension_),
    domain_(id.domain_),
    partition_(id.partition_),
    entity_(id.entity_) { }

    id_(size_t local_id)
    : dimension_(0),
    domain_(0),
    partition_(0),
    entity_(local_id) { }

    template<size_t D, size_t M>
    static id_ make(size_t local_id, size_t partition_id = 0)
    {
      id_ global_id;
      global_id.dimension_ = D;
      global_id.domain_ = M;
      global_id.partition_ = partition_id;
      global_id.entity_ = local_id;

      return global_id;
    }

    template<size_t M>
    static id_ make(size_t dim, size_t local_id, size_t partition_id = 0)
    {
      id_ global_id;
      global_id.dimension_ = dim;
      global_id.domain_ = M;
      global_id.partition_ = partition_id;
      global_id.entity_ = local_id;

      return global_id;
    }

    size_t global_id() const
    {
      return (size_t(dimension_) << 62) | (size_t(domain_) << 60) | 
        (size_t(partition_) << EBITS) | size_t(entity_); 
    }

    operator size_t() const
    {
      return global_id();
    }

    id_& operator=(const id_ &id)
    {
      dimension_ = id.dimension_;
      domain_ = id.domain_;
      partition_ = id.partition_;
      entity_ = id.entity_;

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

  private:
    size_t dimension_ : 2;
    size_t domain_ : 2;
    size_t partition_ : PBITS;
    size_t entity_ : EBITS;
  };

} // namespace flecsi

#endif // flecsi_id_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
