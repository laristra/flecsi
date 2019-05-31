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

#include "flecstan-macro.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/MacroArgs.h"
#include <map>

// -----------------------------------------------------------------------------
// Preprocessor
// -----------------------------------------------------------------------------

namespace flecstan {

class Preprocessor : public clang::PPCallbacks
{
  // File we're currently dealing with. Called "unit" as in translation unit,
  // even though that isn't technically the same concept as file. We use the
  // term "file" elsewhere, to mean the file in which a macro call actually
  // appeared. That might not be unit, but a file that's #included from unit,
  // which is why we handle the two separately.
  const std::string unit;

  // FleCSI macros that we'll recognize
  static const std::set<std::string> macros;

  // CompilerInstance, Yaml
  clang::CompilerInstance & ci;
  Yaml & yaml;

  // FleCSI macro information
  // Map from {file,line,column} to macro calls that appear at that location.
  // That's "calls," plural, because many macro calls can end up associating
  // with one location. For example, consider a user macro that emits calls
  // to several FleCSI macros (I saw such things in FleCSALE.) The "location"
  // as encoded in {file,line,column} is the location of the user macro that
  // emits the FleCSI macros, not the locations of the FleCSI macros. This is
  // why we have a vector<MacroCall> for the value part of the map.
  std::map<std::tuple<std::string, std::string, std::string>,
    std::vector<MacroCall>>
    pos2macros;

public:
  // constructor, destructor
  Preprocessor(const std::string &, clang::CompilerInstance &, Yaml &);
  ~Preprocessor();

  // MacroExpands
  // override w.r.t. PPCallbacks
  void MacroExpands(const clang::Token &,
    const clang::MacroDefinition &,
    const clang::SourceRange,
    const clang::MacroArgs * const) override;

  // call
  const MacroCall * findcall(const clang::SourceLocation &) const;

  // map2yaml
  void map2yaml();
};

} // namespace flecstan
