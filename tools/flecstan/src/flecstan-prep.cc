/* -*- C++ -*- */

#include "flecstan-prep.h"



// -----------------------------------------------------------------------------
// Preprocessor::macros
// Initialization
// -----------------------------------------------------------------------------

namespace flecstan {

// macros
const std::set<std::string> Preprocessor::macros {
   #define flecstan_quote(name) #name,
   flecstan_expand(flecstan_quote,)
   #undef flecstan_quote
};

} // namespace flecstan



// -----------------------------------------------------------------------------
// Preprocessor
// Constructor and destructor
// -----------------------------------------------------------------------------

namespace flecstan {

Preprocessor::Preprocessor(clang::CompilerInstance &_ci, Yaml &_yaml)
 : ci(_ci), yaml(_yaml)
{
   debug("ctor: Preprocessor");
}


Preprocessor::~Preprocessor()
{
   debug("dtor: Preprocessor");
}

} // namespace flecstan



// -----------------------------------------------------------------------------
// Preprocessor
// MacroExpands
// -----------------------------------------------------------------------------

namespace flecstan {

void Preprocessor::MacroExpands(
   const clang::Token &token,
   const clang::MacroDefinition &def,
   const clang::SourceRange range,
   const clang::MacroArgs *const arguments
) {
   debug("Preprocessor::MacroExpands()");

   // Sema, SourceManager
   clang::Sema &sema = ci.getSema();
   clang::SourceManager &man = sema.getSourceManager();

   // Macro's IdentifierInfo
   const clang::IdentifierInfo *const ii = token.getIdentifierInfo();
   if (!ii) return;

   // Macro's name
   const std::string name = ii->getName().str();

   // If name isn't one we're looking for, we're done here
   if (macros.find(name) == macros.end())
      return;

   // OK, we've recognized one of our macros.
   // Create a MacroInvocation object for this particular macro call.
   MacroInvocation m(token,range,man,name);

   // Number of arguments to the macro
   const std::size_t narg = arguments->getNumMacroArguments();

   // For each macro argument
   for (std::size_t a = 0;  a < narg;  ++a) {
      // Pointer to first token of the argument (other tokens will follow)
      const clang::Token *const tokbegin = arguments->getUnexpArgument(a);

      // Initialize argument; then push tokens below...
      m.arguments.push_back(std::vector<clang::Token>{});

      // For each token of the current macro argument
      const clang::Token *tok; // outside for-loop in case we use it below
      for (tok = tokbegin;  tok->isNot(clang::tok::eof);  ++tok)
         m.arguments.back().push_back(*tok);

      /*
      // Here's a way we can get the original spelling (including white space!)
      // of the full, original argument (not the argument broken into tokens).
      // I'll include this here, in case we end up wanting it for anything.
      clang::CharSourceRange range;
      range.setBegin(tokbegin->getLocation());
      range.setEnd(tok->getLocation()); // the eof from the above for-loop
      llvm::StringRef ref = clang::Lexer::getSourceText(
         range, man, sema.getLangOpts());
      std::cout << "ref = \"" << ref.str() << "\"" << std::endl;
      */
   }

   // Enter the new MacroInvocation object into our "source map" structure,
   // essentially a map(FileID,map(Offset,MacroInvocation)).
   const std::pair<clang::FileID, unsigned> &pos =
      man.getDecomposedExpansionLoc(token.getLocation());
   const clang::FileID fileid = pos.first;
   const unsigned offset = pos.second;

   auto pair = sourceMap[fileid].insert(std::make_pair(offset,m));
   const bool direct = pair.second; // direct (true) or nested (false) macro?
   if (!direct) return;

   // 2019-01-16.
   // Remark to self. There's some apparent convolutedness here that deserves
   // to be cleaned up eventually. We're dealing with a MacroInvocation above,
   // and an InvocationInfo below. For our YAML output, each macro will have
   // two sections: an "invoked" section containing only the basics (file, line,
   // column) of any invocation of a FleCSI macro; and a "matched" section that,
   // for each of our macros, gives those same basics (file, line, column) via
   // the flecstan::macrobase class, plus additional macro-specific parameters.
   // Above, we're recording full information (basics and macro parameters),
   // in a MacroInvocation, into our "source map"; this info is recovered
   // later if and when a construct in the AST is matched with a macro. Below,
   // we're immediately recording the basic information (via an InvocationInfo)
   // into the "invoked" part of the YAML structure for the macro in question.
   // It makes sense to do as above: record all macro information (including
   // arguments) for potential later retrieval when as AST construct matches.
   // The below part, as written here, feels redundant and out-of-order; and,
   // in fact, InvocationInfo's last two parameters (name,macro) are there, at
   // the moment, just for printing purposes (see the contructor). Perhaps the
   // below code (basic) should go before the above code (full info), and the
   // classes InvocationInfo and macrobase should go together. (Not sure about
   // YAML mapping issues, though.) Think about all this.
   // Record, for the YAML document, the basics of this macro invocation

   // Record, for the YAML document, the basics of this macro invocation
   #define flecstan_invoked(mac) \
      if (name == #mac) \
         yaml.mac.invoked.push_back(InvocationInfo(sema,m))
   flecstan_expand(flecstan_invoked,;)
   #undef flecstan_invoked
}

} // namespace flecstan



// -----------------------------------------------------------------------------
// Preprocessor
// invocation
// -----------------------------------------------------------------------------

namespace flecstan {

const MacroInvocation *Preprocessor::invocation(
   const clang::SourceLocation &loc
) const {
   debug("Preprocessor::invocation()");

   // Sema, SourceManager
   clang::Sema &sema = ci.getSema();
   clang::SourceManager &man = sema.getSourceManager();

   // For the construct (declaration, expression, etc.) we're examining, get
   // its File ID and offset. We'll then see if it's associated with a macro.
   //
   // Remark: The getFileLoc() call (in contrast to just sending loc directly
   // to getDecomposedExpansionLoc()) was necessary in order to extract the
   // correct location in the event that one of our macros is invoked within
   // another macro. For example, consider this code:
   //     #define print(x) ...
   //     ...
   //     print(flecsi_macro(stuff))
   // Using getFileLoc(), the location of constructs in "stuff" (as sent via
   // the parameter "loc" to the present function) is associated with the 'f'
   // in "flecsi_macro" - just as we want it to be. Without getFileLoc(), we
   // were actually getting the location for the 'p' in print!
   const std::pair<clang::FileID,unsigned>
      pos = man.getDecomposedExpansionLoc(man.getFileLoc(loc));
   const clang::FileID fileid = pos.first;
   const unsigned offset = pos.second;

   // Look for File ID in our map of macro information
   auto itr = sourceMap.find(fileid);
   if (itr == sourceMap.end()) { // find failed
      debug("invocation(): failure 1");
      return nullptr;
   }

   // Submap (offset --> MacroInvocation) for the File ID.
   // Keys of this submap are offsets of the starts of macro invocations.
   const std::map<std::size_t, MacroInvocation> &sub = itr->second;

   // Look for the largest submap entry <= offset. That macro would
   // be responsible for the present construct, if *any* macro is.
   auto mitr = gleq(sub,offset);
   if (mitr == sub.end()) { // find failed; offset must be before any macro
      debug("invocation(): failure 2");
      return nullptr;
   }

   // MacroInvocation for the desired (FileID,offset)
   const MacroInvocation &m = mitr->second;
   const std::size_t begin = mitr->first;
   const std::size_t end   = m.end;
   if (begin <= offset && offset <= end)
      return &mitr->second;

   debug("invocation(): failure 3");
   return nullptr;
}

} // namespace flecstan
