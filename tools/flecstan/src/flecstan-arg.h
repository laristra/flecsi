/* -*- C++ -*- */

#ifndef flecstan_arg
#define flecstan_arg

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
  // Analyzer's (from *this* source code!) executable file name
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
  //    cc
  //    yaml (yaml, as input)
  // yout (yaml, as output)
  //
  // database = f(dir,json)
  // commands = f(clang,flags,dir,cc)

  // Results
  // In the first two vectors, the bools indicate whether or not the files in
  // question (JSON or C++, respectively) were found at the time of command-
  // line parsing. The third vector is a simple but presently sufficient way
  // to indicate whether the next queued file is a JSON in database, or a C++
  // in commands: true == the former, false == the latter.
  std::vector<std::pair<std::string, bool>> database;
  std::vector<std::pair<clang::tooling::CompileCommand, bool>> commands;
  std::vector<bool> isdb;
};

// Re: command-line arguments
exit_status_t
initial(std::size_t &, const int, const char * const * const, CmdArgs &);
exit_status_t
arguments(std::size_t &, const int, const char * const * const, CmdArgs &);

} // namespace flecstan

#endif
