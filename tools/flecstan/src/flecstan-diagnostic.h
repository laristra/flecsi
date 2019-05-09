/* -*- C++ -*- */

/* -----------------------------------------------------------------------------
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2019, Triad National Security, LLC
   All rights reserved.
----------------------------------------------------------------------------- */

#pragma once

#include "flecstan-misc.h"
#include "clang/Tooling/Tooling.h"



// -----------------------------------------------------------------------------
// Diagnostic
// -----------------------------------------------------------------------------

namespace flecstan {

class Diagnostic : public clang::DiagnosticConsumer
{
   exit_status_t &status;

public:

   Diagnostic(exit_status_t &s) : status(s)
      { debug("ctor: Diagnostic"); }
  ~Diagnostic()
      { debug("dtor: Diagnostic"); }

   // override w.r.t. DiagnosticConsumer
   void HandleDiagnostic(
      clang::DiagnosticsEngine::Level,
      const clang::Diagnostic &
   ) override;
};

} // namespace flecstan
