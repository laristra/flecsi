/* -*- C++ -*- */

#ifndef flecstan_prep
#define flecstan_prep

#include "flecstan-macro.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/MacroArgs.h"

// -----------------------------------------------------------------------------
// Preprocessor
// -----------------------------------------------------------------------------

namespace flecstan {

class Preprocessor : public clang::PPCallbacks
{
  // FleCSI macros that we'll recognize
  static const std::set<std::string> macros;

  // CompilerInstance, Yaml
  clang::CompilerInstance & ci;
  Yaml & yaml;

  // FleCSI macro information
  // Map from {file,line,column} to a particular macro invocation
  std::map<std::tuple<std::string, std::string, std::string>,
    std::vector<MacroInvocation>>
    pos2macro;

public:
  // constructor, destructor
  Preprocessor(clang::CompilerInstance &, Yaml &);
  ~Preprocessor();

  // MacroExpands
  // override w.r.t. PPCallbacks
  void MacroExpands(const clang::Token &,
    const clang::MacroDefinition &,
    const clang::SourceRange,
    const clang::MacroArgs * const) override;

  // invocation
  const MacroInvocation * invocation(const clang::SourceLocation &) const;
};

} // namespace flecstan

#endif
