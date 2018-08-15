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

#include <flecsi/utils/logging.h>

#if !defined(ENABLE_OPENSSL)
#error ENABLE_OPENSSL not defined! This file depends on OpenSSL!
#endif

#include <openssl/evp.h>

namespace flecsi {
namespace utils {

struct checksum_t {
  unsigned char value[EVP_MAX_MD_SIZE];
  char strvalue[EVP_MAX_MD_SIZE * 2 + 1];
  unsigned int length;
}; // struct checksum_t

/*!
  Compute the checksum of an array.

  @param buffer The data buffer on which to compute the checksum.
  @param elements The size of the buffer.
  @param[out] sum The checksum data structure to fill.
  @param[in] digest The digest context.

  @ingroup utils
 */
template<typename T>
void
checksum(
    T * buffer,
    std::size_t elements,
    checksum_t & sum,
    const char * digest = "md5") {
  std::size_t bytes = elements * sizeof(T);

  EVP_MD_CTX * ctx = EVP_MD_CTX_create();

  /*!
    Add all digests to table
   */
  OpenSSL_add_all_digests();

  /*!
    Initialize context
   */
  EVP_MD_CTX_init(ctx);

  /*!
    Get digest
   */
  const EVP_MD * md = EVP_get_digestbyname(digest);
  clog_assert(md, "invalid digest");

  /*!
    Initialize digest
   */
  EVP_DigestInit_ex(ctx, md, NULL);

  /*!
    Update digest with buffer
   */
  EVP_DigestUpdate(ctx, reinterpret_cast<void *>(buffer), bytes);

  /*!
    Finalize
   */
  EVP_DigestFinal_ex(ctx, sum.value, &sum.length);

  /*!
    Free resources
   */
  EVP_MD_CTX_destroy(ctx);

  char tmp[256];
  strcpy(sum.strvalue, "");

  for (std::size_t i(0); i < sum.length; i++) {
    sprintf(tmp, "%02x", sum.value[i]);
    strcat(sum.strvalue, tmp);
  } // for

} // checksum

} // namespace utils
} // namespace flecsi
