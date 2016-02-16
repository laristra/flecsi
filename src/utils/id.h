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
  struct id_
  {
    static_assert(PBITS + EBITS == sizeof(size_t) * 8 - 4, 
                  "invalid id bit configuration");

    id_() { }

    id_(id_& id)
    : dimension(id.dimension),
    domain(id.domain),
    partition(id.partition),
    entity(id.entity) { }

    id_(size_t local_id)
    : dimension(0),
    domain(0),
    partition(0),
    entity(local_id) { }

    size_t dimension : 2;
    size_t domain : 2;
    size_t partition : PBITS;
    size_t entity : EBITS;

    template<size_t D, size_t M>
    static id_ make(size_t local_id, size_t partition_id = 0)
    {
      id_ global_id;
      global_id.dimension = D;
      global_id.domain = M;
      global_id.partition_id = partition_id;
      global_id.entity = local_id;

      return global_id;
    }

    template<size_t M>
    static id_ make(size_t dim, size_t local_id, size_t partition_id = 0)
    {
      id_ global_id;
      global_id.dimension = dim;
      global_id.domain = M;
      global_id.partition_id = partition_id;
      global_id.entity = local_id;

      return global_id;
    }

    size_t global_id() const
    {
      return (dimension << 62) | (domain << 60) | (partition << EBITS) | entity; 
    }

    operator size_t() const
    {
      return global_id();
    }

    id_& operator=(const id_ &id)
    {
      dimension = id.dimension;
      domain = id.domain;
      partition = id.partition;
      entity = id.entity;

      return *this;
    }
  };

} // namespace flecsi

#endif // flecsi_id_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
