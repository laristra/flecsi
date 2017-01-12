/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_checksum_h
#define flecsi_utils_checksum_h

///
/// \file
/// \date Initial file creation: Jan 03, 2017
///

#include "flecsi/utils/logging.h"

#if !defined(ENABLE_OPENSSL)
  #error ENABLE_OPENSSL not defined! This file depends on OpenSSL!
#endif

#include <openssl/evp.h>

namespace flecsi {
namespace utils {

struct checksum_t {
  unsigned char value[EVP_MAX_MD_SIZE];
  char strvalue[EVP_MAX_MD_SIZE*2+1];
  unsigned int length;
}; // struct checksum_t

///
/// Compute the checksum of an array.
///
/// \param buffer The data buffer on which to compute the checksum.
/// \param elements The size of the buffer.
/// \param[out] sum The checksum data structure to fill.
/// \param[in] digest The digest context.
///
template<
  typename T
>
void
checksum(
  T * buffer,
  size_t elements,
  checksum_t & sum,
  const char * digest = "md5"
)
{
  size_t bytes = elements*sizeof(T);

  EVP_MD_CTX ctx;

  // Add all digests to table
  OpenSSL_add_all_digests();

  // Initialize context
  EVP_MD_CTX_init(&ctx);

  // Get digest
  const EVP_MD * md = EVP_get_digestbyname(digest);
  clog_assert(md, "invalid digest");

  // Initialize digest
  EVP_DigestInit_ex(&ctx, md, NULL);

  // Update digest with buffer
  EVP_DigestUpdate(&ctx, reinterpret_cast<void *>(buffer), bytes);

  // Finalize
  EVP_DigestFinal_ex(&ctx, sum.value, &sum.length);

  // Free resources
  EVP_MD_CTX_cleanup(&ctx);

  char tmp[256];
  strcpy(sum.strvalue, "");

  for(size_t i(0); i<sum.length; i++) {
    sprintf(tmp, "%02x", sum.value[i]);
    strcat(sum.strvalue, tmp);
  } // for

} // checksum

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_checksum_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
