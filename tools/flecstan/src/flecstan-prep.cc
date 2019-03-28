/* -*- C++ -*- */

#include "flecstan-prep.h"
#include <queue>

namespace flecstan {



// -----------------------------------------------------------------------------
// Preprocessor::macros
// Initialization
// -----------------------------------------------------------------------------

// macros
// A set of all the macro names we care about
const std::set<std::string> Preprocessor::macros {
   #define flecstan_quote(name) #name,
   flecstan_expand(flecstan_quote,)
   #undef flecstan_quote
};



// -----------------------------------------------------------------------------
// Preprocessor::ctor,dtor
// -----------------------------------------------------------------------------

Preprocessor::Preprocessor(clang::CompilerInstance &_ci, Yaml &_yaml)
 : ci(_ci), yaml(_yaml)
{
   debug("ctor: Preprocessor");
}


Preprocessor::~Preprocessor()
{
   debug("dtor: Preprocessor");
}



// -----------------------------------------------------------------------------
// Re: nested macros
// Eventually, combine this with Preprocessor::macros
// -----------------------------------------------------------------------------

#define nested_mac(outer,...) \
   std::make_pair(std::string(outer), std::queue<std::string>{{__VA_ARGS__}})

static std::map<std::string, std::queue<std::string>> nested_macros
{
   nested_mac(
      "flecsi_register_mpi_task_simple",
      "flecsi_register_task_simple"
   ),
   nested_mac(
      "flecsi_register_mpi_task",
      "flecsi_register_task"
   ),
   nested_mac(
      "flecsi_execute_mpi_task_simple",
      "flecsi_execute_task_simple"
   ),
   nested_mac(
      "flecsi_execute_mpi_task",
      "flecsi_execute_task"
   ),
   nested_mac(
      "flecsi_get_global",
      "flecsi_get_handle",
      "flecsi_get_client_handle"
   ),
   nested_mac(
      "flecsi_get_color",
      "flecsi_get_handle",
      "flecsi_get_client_handle"
   )
};

#undef nested_mac



// -----------------------------------------------------------------------------
// Preprocessor::MacroExpands
// -----------------------------------------------------------------------------

void Preprocessor::MacroExpands(
   const clang::Token &token,
   const clang::MacroDefinition &def,
   const clang::SourceRange range,
   const clang::MacroArgs *const arguments
) /* override */ {
   debug("Preprocessor::MacroExpands()");

   // Sema, SourceManager
   clang::Sema &sema = ci.getSema();
   clang::SourceManager &sman = sema.getSourceManager();

   // Macro: IdentifierInfo, name
   const clang::IdentifierInfo *const ii = token.getIdentifierInfo();
   if (!ii) return;
   const std::string name = ii->getName().str();

   // If macro name isn't one we're looking for, we're done here
   if (macros.find(name) == macros.end())
      return;

   // Deal with the fact that the current FleCSI macro may have been called
   // by the user, or may have appeared here simply because another FleCSI
   // macro called it.
   static std::queue<std::string> expect;
   if (expect.empty()) {
      // Either it's the first call to MacroExpands(); or else no nested FleCSI
      // macro is expected, based on the one that was seen on the previous call.
      // Insert and retrieve an empty queue of expected upcoming nested macros
      // for the current macro, or retrieve its existing empty or non-empty one
      // for use on the next MacroExpands() call.
      expect = nested_macros.insert(
         std::make_pair(name, std::queue<std::string>{})
      ).first->second;
   } else {
      // Based on the previous call to MacroExpands(), we expect to see some
      // particular macro name.
      assert(name == expect.front()); /// <== make into a real diagnostic
      expect.pop();
      return;
   }

   // OK, a user called a FleCSI macro.
   // Create a MacroInvocation object for this call.
   MacroInvocation mi(token,range,sman,name);

   // Number of arguments to the macro
   const std::size_t narg = arguments->getNumMacroArguments();

   // For each macro argument
   for (std::size_t a = 0;  a < narg;  ++a) {
      // Pointer to first token of the argument (other tokens will follow)
      const clang::Token *const tokbegin = arguments->getUnexpArgument(a);

      // Initialize argument; then push tokens below...
      mi.arguments.push_back(std::vector<clang::Token>{});

      // For each token of the current argument
      const clang::Token *tok; // outside the for-loop, in case we use it below
      for (tok = tokbegin;  tok->isNot(clang::tok::eof);  ++tok)
         mi.arguments.back().push_back(*tok);

      /*
      // Here's a way we can get the original spelling (including white space!)
      // of the full, original argument (not the argument broken into tokens).
      // I'll include this here, in case we end up wanting it for anything.

      clang::CharSourceRange range;
      range.setBegin(tokbegin->getLocation());
      range.setEnd(tok->getLocation()); // the eof from the above for-loop
      llvm::StringRef ref = clang::Lexer::getSourceText(
         range, sman, sema.getLangOpts());
      std::cout << "ref = \"" << ref.str() << "\"" << std::endl;
      */
   }

   // Enter the new MacroInvocation object into our position-to-macro structure.
   pos2macro[
      std::make_tuple(
         mi.location.file,
         mi.location.line,
         mi.location.column
      )
   ].push_back(mi);

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
   #define flecstan_invoked(mac) \
      if (name == #mac) \
         yaml.mac.invoked.push_back(InvocationInfo(sema,mi))
   flecstan_expand(flecstan_invoked,;)
   #undef flecstan_invoked
}



// -----------------------------------------------------------------------------
// Preprocessor::invocation
// -----------------------------------------------------------------------------

const MacroInvocation *Preprocessor::invocation(
   const clang::SourceLocation &loc
) const {
   debug("Preprocessor::invocation()");

   // Sema, SourceManager
   clang::Sema &sema = ci.getSema();
   clang::SourceManager &sman = sema.getSourceManager();

   // For the construct (declaration, expression, etc.) we're examining, get
   // its file, line, and column. We'll then see if it's associated with one
   // of our macros.
   //
   // Remark: Consider something like this:
   //     #define something(x) ...
   //     something(flecsi_macro(parameters))
   // When flecsi_macro expands into constructs in the abstract syntax tree,
   // we want to associate those constructs with the 'f' in "flecsi_macro",
   // not with the 's' in "something". In order for this to happen, we needed
   // to use sman.getFileLoc(loc), below, in place of loc.
   FileLineColumn flc;
   getFileLineColumn(&sman, sman.getFileLoc(loc), flc);

   // Does {file,line,column} match with a macro position we saved earlier...?
   const std::tuple<std::string,std::string,std::string> key(
      flc.file,
      flc.line,
      flc.column
   );
   auto iter = pos2macro.find(key);
   if (iter == pos2macro.end()) {
      // ...No, but that's OK; we just don't care about the AST construct
      debug("invocation(): failure 1");
      return nullptr;
   }

   // ...Yes, so look in the (possibly many) macros at this location
   const std::vector<MacroInvocation> &vec = iter->second;
   for (auto &mi : vec)
      if (!mi.ast) // then hasn't already been matched
         return &mi;

   // No, but that still can be OK; maybe it isn't an AST construct that
   // we'd be needing to match.
   debug("invocation(): failure 2");
   return nullptr;
}

} // namespace flecstan
