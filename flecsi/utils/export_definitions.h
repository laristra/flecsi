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


#if !defined(FLECSI_EXPORT_DEFINITIONS)
#define FLECSI_EXPORT_DEFINITIONS

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define FLECSI_SYMBOL_EXPORT __declspec(dllexport)
#define FLECSI_SYMBOL_IMPORT __declspec(dllimport)
#define FLECSI_SYMBOL_INTERNAL /* empty */
#define FLECSI_APISYMBOL_EXPORT __declspec(dllexport)
#define FLECSI_APISYMBOL_IMPORT __declspec(dllimport)
#elif defined(__NVCC__) || defined(__CUDACC__)
#define FLECSI_SYMBOL_EXPORT /* empty */
#define FLECSI_SYMBOL_IMPORT /* empty */
#define FLECSI_SYMBOL_INTERNAL /* empty */
#define FLECSI_APISYMBOL_EXPORT /* empty */
#define FLECSI_APISYMBOL_IMPORT /* empty */
#elif defined(FLECSI_HAVE_ELF_HIDDEN_VISIBILITY)
#define FLECSI_SYMBOL_EXPORT __attribute__((visibility("default")))
#define FLECSI_SYMBOL_IMPORT __attribute__((visibility("default")))
#define FLECSI_SYMBOL_INTERNAL __attribute__((visibility("hidden")))
#define FLECSI_APISYMBOL_EXPORT __attribute__((visibility("default")))
#define FLECSI_APISYMBOL_IMPORT __attribute__((visibility("default")))
#endif

// make sure we have reasonable defaults
#if !defined(FLECSI_SYMBOL_EXPORT)
#define FLECSI_SYMBOL_EXPORT /* empty */
#endif
#if !defined(FLECSI_SYMBOL_IMPORT)
#define FLECSI_SYMBOL_IMPORT /* empty */
#endif
#if !defined(FLECSI_SYMBOL_INTERNAL)
#define FLECSI_SYMBOL_INTERNAL /* empty */
#endif
#if !defined(FLECSI_APISYMBOL_EXPORT)
#define FLECSI_APISYMBOL_EXPORT /* empty */
#endif
#if !defined(FLECSI_APISYMBOL_IMPORT)
#define FLECSI_APISYMBOL_IMPORT /* empty */
#endif

///////////////////////////////////////////////////////////////////////////////
// define the export/import helper macros used by the runtime module
#if defined(FLECSI_EXPORTS) || defined(FleCSI_EXPORTS)
#define FLECSI_EXPORT FLECSI_SYMBOL_EXPORT
#define FLECSI_EXCEPTION_EXPORT FLECSI_SYMBOL_EXPORT
#define FLECSI_API_EXPORT FLECSI_APISYMBOL_EXPORT
#else
#define FLECSI_EXPORT FLECSI_SYMBOL_IMPORT
#define FLECSI_EXCEPTION_EXPORT FLECSI_SYMBOL_IMPORT
#define FLECSI_API_EXPORT FLECSI_APISYMBOL_IMPORT
#endif

#endif
