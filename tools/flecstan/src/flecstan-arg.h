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
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/Path.h"
#include <fstream>

namespace flecstan {

bool option_version(const std::string &);
bool option_help(const std::string &);

// -----------------------------------------------------------------------------
// fullfile: helper
// -----------------------------------------------------------------------------

inline std::string
fullfile(const std::string & dir, const std::string & file) {
  // The following may be more than we need, but we might as well try to
  // be OS agnostic. See, e.g., clang++'s JSONCompilationDatabase.cpp file.
  // At present, however, we'd need more code elsewhere in order to fully
  // realize genericity.
  llvm::SmallString<1024> s(dir == "." ? "" : dir);
  llvm::sys::path::append(s, file);
  return s.str();
}

// -----------------------------------------------------------------------------
// CmdArgs
// -----------------------------------------------------------------------------

class CmdArgs
{
public:
  // Input type; see below
  enum class file_t { json_t, make_t, cpp_t };

  // Analyzer's executable file name
  std::string exe;

  // Used while parsing the command-line arguments
  booland<std::string> clang; // expects one argument
  booland<std::vector<std::string>> flags; // expects zero or more arguments
  booland<std::string> dir; // expects one argument
  booland<std::string> yout; // expects one argument

  // Concept/usage sketch
  //
  // clang
  //    flags
  // dir
  //    json
  //    make
  //    cc
  //    yaml (yaml, as input)
  // yout (yaml, as output)
  //
  // jsonfile = f(dir,json)
  // makeinfo = f(dir,make) (parses info from make --dry-run VERBOSE=1)
  // commands = f(clang,flags,dir,cc)

  // Results
  // In the first three vectors, the bools indicate whether or not the files
  // in question (JSON, make-verbose, or C++) were found at the time of command
  // line parsing. The last vector is a simple way to indicate whether the
  // next queued file is a JSON, make-verbose, or C++ file.
  std::vector<std::pair<std::string, bool>> jsonfile;
  std::vector<std::pair<std::string, bool>> makeinfo;
  std::vector<std::pair<clang::tooling::CompileCommand, bool>> commands;

  std::vector<file_t> type;
};

// Re: command-line arguments
exit_status_t
initial(std::size_t &, const int, const char * const * const, CmdArgs &);
exit_status_t
arguments(std::size_t &, const int, const char * const * const, CmdArgs &);

// -----------------------------------------------------------------------------
// Some of the following were originally in this header's associated source
// file. I moved them here because they were needed elsewhere as well.
// -----------------------------------------------------------------------------

extern bool option_toggle(const std::string & opt);

// flecstan_setnfind
#define flecstan_setnfind(...)                                                 \
  static const std::set<std::string> set{__VA_ARGS__};                         \
  return set.find(opt) != set.end()

// ------------------------
// endsin_*
// ------------------------

// endsin_json
inline bool
endsin_json(const std::string & str) {
  return endsin(str, ".json");
}

// endsin_make
inline bool
endsin_make(const std::string & str) {
  // Interpret .txt as our make-verbose files. :-/ I may or may not
  // wish to stick with this scheme, but it's serviceable for now.
  return endsin(str, ".txt");
}

// endsin_cc
inline bool
endsin_cc(const std::string & str) {
  return endsin(str, ".cc") || endsin(str, ".cpp") || endsin(str, ".cxx") ||
         endsin(str, ".C");
}

// endsin_yaml
inline bool
endsin_yaml(const std::string & str) {
  return endsin(str, ".yaml");
}

// ------------------------
// Re: compilation
// ------------------------

inline bool
option_dir(const std::string & opt) {
  flecstan_setnfind("-dir", "--dir", "-directory",
    "--directory"
    "-folder",
    "--folder");
}

inline bool
option_clang(const std::string & opt) {
  flecstan_setnfind("-clang", "--clang", "-clang++", "--clang++");
}

inline bool
option_flags(const std::string & opt) {
  flecstan_setnfind("-flag", "--flag", "-flags", "--flags");
}

// ------------------------
// Re: files (input)
// ------------------------

inline bool
option_json(const std::string & opt) {
  flecstan_setnfind("-json", "--json");
}

inline bool
option_make(const std::string & opt) {
  flecstan_setnfind("-make", "--make");
}

inline bool
option_cc(const std::string & opt) {
  flecstan_setnfind("-cc", "--cc", "-cpp", "--cpp", "-cxx", "--cxx", "-c++",
    "--c++", "-C", "--C");
}

inline bool
option_yaml(const std::string & opt) {
  flecstan_setnfind("-yaml", "--yaml");
}

// ------------------------
// Re: files (output)
// ------------------------

inline bool
option_yout(const std::string & opt) {
  flecstan_setnfind("-yout", "--yout");
}

} // namespace flecstan
