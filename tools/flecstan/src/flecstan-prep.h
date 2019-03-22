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
   clang::CompilerInstance &ci;
   Yaml &yaml;

   // FleCSI macro information
   // Idea: For some file, at some location, we'll store
   // information about the invocation of a FleCSI macro
   std::map<
      clang::FileID,
      std::map<
         std::size_t,
         MacroInvocation
      >
   > sourceMap;

public:

   // constructor, destructor
   Preprocessor(clang::CompilerInstance &, Yaml &);
  ~Preprocessor();

   // MacroExpands
   // override w.r.t. PPCallbacks
   void MacroExpands(
      const clang::Token &,
      const clang::MacroDefinition &,
      const clang::SourceRange,
      const clang::MacroArgs *const
   ) override;

   // accessors
   const MacroInvocation *invocation(
      const clang::SourceLocation &
   ) const;
};

} // namespace flecstan



// -----------------------------------------------------------------------------
// Helper: gleq
// Greatest less than or equal to
// -----------------------------------------------------------------------------

namespace flecstan {

// const
template<class MAP>
inline typename MAP::const_iterator gleq(
   const MAP &m,
   const typename MAP::key_type &k
) {
    typename MAP::const_iterator it = m.upper_bound(k);
    return it == m.begin() ? m.end() : --it;
}

// non-const
template<class MAP>
inline typename MAP::iterator gleq(
   MAP &m,
   const typename MAP::key_type &k
) {
    typename MAP::iterator it = m.upper_bound(k);
    return it == m.begin() ? m.end() : --it;
}

} // namespace flecstan

#endif
