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

#include "flecstan-prep.h"
#include <queue>

namespace flecstan {

// -----------------------------------------------------------------------------
// Preprocessor::macros
// Initialization
// -----------------------------------------------------------------------------

// macros
// A set of all the macro names we care about
const std::set<std::string> Preprocessor::macros{
#define flecstan_quote(macname) #macname,
  flecstan_expand(flecstan_quote, )
#undef flecstan_quote
};

// -----------------------------------------------------------------------------
// Preprocessor::ctor, dtor, map2yaml
// -----------------------------------------------------------------------------

// constructor
Preprocessor::Preprocessor(const std::string & u,
  clang::CompilerInstance & c,
  Yaml & y)
  : unit(u), ci(c), yaml(y) {
  debug("ctor: Preprocessor");
}

// destructor
Preprocessor::~Preprocessor() {
  debug("dtor: Preprocessor");
}

// map2yaml
//
// Here, we transfer each MacroCall value in this Preprocessor's pos2macros map
// into the appropriate vector<MacroCall> in Yaml &yaml, which references a
// persistent (not for this Preprocessor object only) construct. By doing this,
// we'll create (if flecstan is asked to emit YAML) a record of every MacroCall
// we found. The vector<MacroCall>s are used in the analysis stage as well, not
// just used for YAML output. (They were initially intended only for the latter
// purpose, but proved to be useful for the analysis stage as well; so we still
// collect the "YAML" output even if we're not actually performing YAML output.
//
// In earlier versions of flecstan, Preprocessor's MacroExpands() pushed to the
// YAML vectors simultaneously with pushing to pos2macros, essentially creating
// both at once instead of building pos2macros first and then transferring its
// information, as we're doing here now. That worked, nominally, but an issue
// was that the "ast" bool in MacroCall (updated during AST visitation in order
// to indicate if we found the AST constructs corresponding to the macro's call)
// ended up being updated only in the pos2macros MacroCalls, not in the YAML
// vectors, because pos2macros is where we search for AST matches and update
// the flag accordingly. (Note: we don't simple chuck the vectors and output
// pos2macros directly to the YAML, because pos2macros is per-processed-file,
// whereas the YAML vectors reflect the entire compilation, all files included.
// And, a vector is easier to deal with, in the analysis stage, than a map is.)
//
// We call the function below from just after our TraverseDecl() call in another
// file. One might wonder why we don't do the pos2macros --> vectors transfer
// in Preprocessor's destructor, above. Doing so *might* be fine, but we were
// a bit skeptical. Preprocessor objects are created in the Actions coming out
// of [Factory : public clang::tooling::FrontendActionFactory]. They're created
// dynamically, using new, but then given to Clang in the form of unique_ptr<>s!
// Clang thus takes ownership of the Preprocessor objects, and we can't be sure
// when and how Clang's internals will destruct them. We'll thus do the transfer
// on our own terms.

void
Preprocessor::map2yaml() {
  debug("Preprocessor::map2yaml()");

  for(auto pair : pos2macros) {
    const std::vector<MacroCall> & vec = pair.second;
    for(auto call : vec) {
#define flecstan_called(mac)                                                   \
  if(call.macname == #mac)                                                     \
  yaml.mac.called.push_back(call)
      flecstan_expand(flecstan_called, ;)
#undef flecstan_called
    }
  }
}

// -----------------------------------------------------------------------------
// Re: nested macros
// Eventually, combine this with Preprocessor::macros
// -----------------------------------------------------------------------------

#define nested_mac(outer, ...)                                                 \
  std::make_pair(std::string(outer), std::queue<std::string>{{__VA_ARGS__}})

static std::map<std::string, std::queue<std::string>> nested_macros{
  nested_mac("flecsi_register_mpi_task_simple", "flecsi_register_task_simple"),
  nested_mac("flecsi_register_mpi_task", "flecsi_register_task"),
  nested_mac("flecsi_execute_mpi_task_simple", "flecsi_execute_task_simple"),
  nested_mac("flecsi_execute_mpi_task", "flecsi_execute_task"),
  nested_mac("flecsi_get_global",
    "flecsi_get_handle",
    "flecsi_get_client_handle"),
  nested_mac("flecsi_get_color",
    "flecsi_get_handle",
    "flecsi_get_client_handle")};

#undef nested_mac

// -----------------------------------------------------------------------------
// Preprocessor::MacroExpands
// -----------------------------------------------------------------------------

void
Preprocessor::MacroExpands(const clang::Token & token,
  const clang::MacroDefinition &,
  const clang::SourceRange,
  const clang::MacroArgs * const macroargs) /* override */ {
  debug("Preprocessor::MacroExpands()");

  // ------------------------
  // Bookkeeping
  // ------------------------

  // Sema, SourceManager
  clang::Sema & sema = ci.getSema();
  clang::SourceManager & sman = sema.getSourceManager();

  // Macro: IdentifierInfo, name
  const clang::IdentifierInfo * const ii = token.getIdentifierInfo();
  if(!ii)
    return;
  const std::string macname = ii->getName().str();

  // If macro name isn't one we're looking for, we're done here
  if(macros.find(macname) == macros.end())
    return;

  // ------------------------
  // User-called or nested
  // ------------------------

  /*
  fixme
  Think through this in more detail.
     Consider plain macro A,
     and macro B that calls C
     and macro D that calls E that calls F
  Realize that even the nested macros *might* also be user-called directly.
  */

  // Deal with the fact that the current FleCSI macro may have been called
  // by the user, or may have appeared here simply because another FleCSI
  // macro called it.
  static std::string original;
  static std::queue<std::string> expect;

  if(expect.empty()) {
    // Either it's the first call to MacroExpands(); or else no nested FleCSI
    // macro is expected, based on the one that was seen on the previous call.
    // Insert and retrieve an empty queue of expected upcoming nested macros
    // for the current macro, or retrieve its existing empty or non-empty one
    // for use on the next MacroExpands() call.
    expect =
      nested_macros
        .insert(std::make_pair(original = macname, std::queue<std::string>{}))
        .first->second;
  }
  else {
    // Based on the previous MacroExpands() call, we expect to see a specific
    // macro. E.g. flecsi_register_task called from flecsi_register_mpi_task.
    if(macname == expect.front()) {
      expect.pop();
      return;
    }

    // Print an error, but then continue in this function. The proper error
    // recovery process is probably indeterminate under these circumstances.
    error(
      "Expected call to Flecsi macro:\n   " + original +
      "\n"
      "inside of FleCSI macro:\n   " +
      expect.front() +
      "\n"
      "Either this analyzer is broken, or someone changed FleCSI's internal\n"
      "macro implementations without informing us. "
      "Please report this error!\n"
      "Note: this problem may trigger additional errors or warnings....");
  }

  // ------------------------
  // MacroCall
  // ------------------------

  // OK, a user called a FleCSI macro.
  // Create a MacroCall object for this call.
  MacroCall mc(unit, token, sman, macname);

  // Number of arguments to the macro
  const std::size_t narg = macroargs->getNumMacroArguments();

  // For each macro argument
  for(std::size_t a = 0; a < narg; ++a) {
    // Pointer to first token of the argument (other tokens will follow)
    const clang::Token * const tokbegin = macroargs->getUnexpArgument(a);

    // Initialize argstok; then push tokens below...
    mc.argstok.push_back(std::vector<clang::Token>{});

    // For each token of the current argument
    const clang::Token * tok; // outside the for-loop; we use it below!
    for(tok = tokbegin; tok->isNot(clang::tok::eof); ++tok)
      mc.argstok.back().push_back(*tok); // push_back to inner vector

    // Argument, in raw form.
    // Here, we get the original spelling (including white space) of the
    // original argument - not the argument broken into tokens, as above.
    clang::CharSourceRange range;
    range.setBegin(tokbegin->getLocation());
    range.setEnd(tok->getLocation()); // the "eof" from the above for-loop
    llvm::StringRef ref =
      clang::Lexer::getSourceText(range, sman, sema.getLangOpts());
    mc.argsraw.push_back(ref.str());
  }

  // ------------------------
  // Save
  // ------------------------

  // Enter the new MacroCall object into our position-to-macro structure.
  mc.report(sema);
  pos2macros[std::make_tuple(
               mc.location.file, mc.location.line, mc.location.column)]
    .push_back(mc);
}

// -----------------------------------------------------------------------------
// Preprocessor::findcall
// -----------------------------------------------------------------------------

const MacroCall *
Preprocessor::findcall(const clang::SourceLocation & loc) const {
  debug("Preprocessor::findcall()");

  // Sema, SourceManager
  clang::Sema & sema = ci.getSema();
  clang::SourceManager & sman = sema.getSourceManager();

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
  // to use sman.getFileLoc(loc), below, rather that just loc.
  FileLineColumn flc;
  getFileLineColumn(&sman, sman.getFileLoc(loc), flc);

  // Does {file,line,column} match with a macro position we saved earlier...?
  const std::tuple<std::string, std::string, std::string> key(
    flc.file, flc.line, flc.column);
  auto iter = pos2macros.find(key);
  if(iter == pos2macros.end()) {
    // ...No, but that's OK; we just don't care about the AST construct
    debug("findcall(): failure 1");
    return nullptr;
  }

  // ...Yes, so look in the (possibly many) macros at this location
  const std::vector<MacroCall> & vec = iter->second;
  for(auto & mc : vec)
    if(!mc.ast) // then hasn't already been matched
      return &mc;

  // No, but that still can be OK; maybe it isn't an AST construct that
  // we'd be needing to match.
  debug("findcall(): failure 2");
  return nullptr;
}

} // namespace flecstan
