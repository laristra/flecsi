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

#include "flecstan-arg.h"

namespace flecstan {

// fixme
// This file arguably has some redundancies and maintenance headaches, with
// regards to the bookkeeping and such that's involved in handling command
// arguments. Think about how to improve the situation. There are various
// ways that this could be done, e.g. string to function-pointer map to deal
// with the various command arguments.

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// bail
inline exit_status_t
bail() {
  return fatal("Error while parsing command line. Bailing out.");
}

// fixdir
// Needs work to be OS-agnostic; doesn't currently handle Windows backslashes,
// for instance. We'd probably need additional work elsewhere, in this respect,
// if we're going to support Windows.
std::string
fixdir(const std::string & in) {
  // begin, end
  int b = 0;
  int e = in.size();

  while(b < e && in[b] == ' ')
    b++; // remove beginning spaces
  while(e > b && in[e - 1] == ' ')
    e--; // remove ending spaces
  while(e > b && in[e - 1] == '/')
    e--; // remove ending /s

  // Note that internal spaces aren't removed - they shouldn't be. This
  // includes the unlikely but possible scenario of, say, an input like
  // "foo/bar /". The rightmost directory here is "bar ", not "bar". We
  // remove the ending /, but preserve the [formerly internal] space.

  return in.substr(b, e - b);
}

// macros
#define flecstan_paste(a, b) a##b

#define flecstan_toggle(flag)                                                  \
  inline bool flecstan_paste(option_, flag)(const std::string & opt) {         \
    static const std::set<std::string> set{                                    \
      "-" #flag, "--" #flag, "-" #flag "s", "--" #flag "s"};                   \
    return set.find(opt) != set.end();                                         \
  }                                                                            \
  inline bool flecstan_paste(option_no_, flag)(const std::string & opt) {      \
    static const std::set<std::string> set{                                    \
      "-no-" #flag, "--no-" #flag, "-no-" #flag "s", "--no-" #flag "s"};       \
    return set.find(opt) != set.end();                                         \
  }

#define flecstan_setnfind(...)                                                 \
  static const std::set<std::string> set{__VA_ARGS__};                         \
  return set.find(opt) != set.end()

// -----------------------------------------------------------------------------
// option_*
// Perhaps it isn't worthwhile to have so many forms in each case. Our intention
// is to provide a tool that's easy for users to invoke. With several available
// forms for each option, users don't need to worry about remembering precisely
// which form (-- or -, singular or plural, etc.) is correct.
// -----------------------------------------------------------------------------

// ------------------------
// Simple flags
// ------------------------

// version
bool
option_version(const std::string & opt) {
  flecstan_setnfind("-version", "--version");
}

// help
bool
option_help(const std::string & opt) {
  flecstan_setnfind("-help", "--help");
}

// quiet
inline bool
option_quiet(const std::string & opt) {
  flecstan_setnfind("-quiet", "--quiet");
}

// verbose
inline bool
option_verbose(const std::string & opt) {
  flecstan_setnfind("-verbose", "--verbose");
}

// short
inline bool
option_short(const std::string & opt) {
  flecstan_setnfind("-short", "--short");
}

// long
inline bool
option_long(const std::string & opt) {
  flecstan_setnfind("-long", "--long");
}

// print
inline bool
option_print(const std::string & opt) {
  flecstan_setnfind("-print", "--print");
}

// debug
inline bool
option_debug(const std::string & opt) {
  flecstan_setnfind("-debug", "--debug");
}
inline bool
option_no_debug(const std::string & opt) {
  flecstan_setnfind("-no-debug", "--no-debug");
}

// heading
inline bool
option_heading(const std::string & opt) {
  flecstan_setnfind("-heading", "--heading", "-headings", "--headings",
    "-header", "--header", "-headers", "--headers");
}

inline bool
option_no_heading(const std::string & opt) {
  flecstan_setnfind("-no-heading", "--no-heading", "-no-headings",
    "--no-headings", "-no-header", "--no-header", "-no-headers",
    "--no-headers");
}

// various
flecstan_toggle(file) flecstan_toggle(report) flecstan_toggle(note)
  flecstan_toggle(warning) flecstan_toggle(error) flecstan_toggle(column)
    flecstan_toggle(color) flecstan_toggle(trace)

  // compilation command line
  inline bool option_ccline(const std::string & opt) {
  flecstan_setnfind("-cc-line", "--cc-line", "-cc-lines", "--cc-lines");
}

inline bool
option_no_ccline(const std::string & opt) {
  flecstan_setnfind(
    "-no-cc-line", "--no-cc-line", "-no-cc-lines", "--no-cc-lines");
}

// sections
inline bool
option_command(const std::string & opt) {
  flecstan_setnfind("-command", "--command");
}
inline bool
option_no_command(const std::string & opt) {
  flecstan_setnfind("-no-command", "--no-command");
}

inline bool
option_compile(const std::string & opt) {
  flecstan_setnfind("-compilation", "--compilation");
}
inline bool
option_no_compile(const std::string & opt) {
  flecstan_setnfind("-no-compilation", "--no-compilation");
}

inline bool
option_analysis(const std::string & opt) {
  flecstan_setnfind("-analysis", "--analysis");
}
inline bool
option_no_analysis(const std::string & opt) {
  flecstan_setnfind("-no-analysis", "--no-analysis");
}

inline bool
option_summary(const std::string & opt) {
  flecstan_setnfind("-summary", "--summary");
}
inline bool
option_no_summary(const std::string & opt) {
  flecstan_setnfind("-no-summary", "--no-summary");
}

// ------------------------
// General options
// ------------------------

inline bool
option_clang(const std::string & opt) {
  flecstan_setnfind("-clang", "--clang", "-clang++", "--clang++");
}

inline bool
option_flags(const std::string & opt) {
  flecstan_setnfind("-flag", "--flag", "-flags", "--flags");
}

inline bool
option_dir(const std::string & opt) {
  flecstan_setnfind("-dir", "--dir", "-directory", "--directory");
}

inline bool
option_json(const std::string & opt) {
  flecstan_setnfind("-json", "--json");
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

inline bool
option_yout(const std::string & opt) {
  flecstan_setnfind("-yout", "--yout");
}

// cleanup
#undef flecstan_paste
#undef flecstan_toggle
#undef flecstan_setnfind

// ------------------------
// Any of the above
// ------------------------

// fixme
// This is part of the aforementioned maintenance headache.
inline bool
option_any(const std::string & opt) {
  return option_version(opt) || option_help(opt) || option_quiet(opt) ||
         option_verbose(opt) || option_short(opt) || option_long(opt) ||
         option_print(opt) ||

         option_debug(opt) || option_no_debug(opt) || option_heading(opt) ||
         option_no_heading(opt) || option_file(opt) || option_no_file(opt) ||
         option_report(opt) || option_no_report(opt) || option_note(opt) ||
         option_no_note(opt) || option_warning(opt) || option_no_warning(opt) ||
         option_error(opt) || option_no_error(opt) || option_color(opt) ||
         option_no_color(opt) || option_trace(opt) || option_no_trace(opt) ||

         option_ccline(opt) || option_no_ccline(opt) || option_column(opt) ||
         option_no_column(opt) ||

         option_command(opt) || option_no_command(opt) || option_compile(opt) ||
         option_no_compile(opt) || option_analysis(opt) ||
         option_no_analysis(opt) || option_summary(opt) ||
         option_no_summary(opt) ||

         option_clang(opt) || option_flags(opt) || option_dir(opt) ||
         option_json(opt) || option_cc(opt) || option_yaml(opt) ||
         option_yout(opt);
}

// -----------------------------------------------------------------------------
// endsin_*
// -----------------------------------------------------------------------------

inline bool
endsin_json(const std::string & str) {
  return endsin(str, ".json");
}

inline bool
endsin_cc(const std::string & str) {
  return endsin(str, ".cc") || endsin(str, ".cpp") || endsin(str, ".cxx") ||
         endsin(str, ".C");
}

inline bool
endsin_yaml(const std::string & str) {
  return endsin(str, ".yaml");
}

// -----------------------------------------------------------------------------
// process_clang
// -----------------------------------------------------------------------------

exit_status_t
process_clang(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_clang()");

  // Argument?
  if(int(++i) == argc || option_any(argv[i]) || endsin_json(argv[i]) ||
     endsin_cc(argv[i]) || endsin_yaml(argv[i])) {
    i--;
    return error("The " + opt +
                 " option (clang++ compiler) "
                 "expects an argument.\n"
                 "Example: " +
                 opt + " /usr/bin/clang++");
  }

  // Note (before Save, so com.clang.set() is correct)
  note(std::string(com.clang.set() ? "Changing " : "Setting ") +
       "C++ compiler to " + quote(argv[i]) + ".");

  // Save
  com.clang = argv[i];
  return exit_clean;
}

// -----------------------------------------------------------------------------
// process_flags
// -----------------------------------------------------------------------------

exit_status_t
process_flags(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_flags()");

  // Bookkeeping
  const bool already = com.flags.set(); // previously set?
  com.flags.value().clear(); // clear
  com.flags = std::vector<std::string>{}; // so set() is correct

  // Argument[s]?
  bool arguments = false;
  while(true) {
    if(int(++i) == argc || option_any(argv[i]) || endsin_json(argv[i]) ||
       endsin_cc(argv[i]) || endsin_yaml(argv[i])) {
      // No more arguments to the current option, so we're done
      i--;
      if(arguments) {
        // Had at least one argument
        std::string all;
        for(std::size_t s = 0; s < com.flags.value().size(); ++s)
          all += (s ? " " : "") + quote(com.flags.value()[s]);
        return note(std::string(already ? "Changing " : "Setting ") +
                    "C++ flags to " + all + ".");
      }
      // Had no arguments
      return note("Clearing flags.");
    }

    // Have an[other] argument
    arguments = true;
    const std::string flag = argv[i];

    if(flag != "") {
      // Save
      note("Found flag " + quote(flag) + ".");
      com.flags.value().push_back(flag);
    }
  }

  return exit_clean;
}

// -----------------------------------------------------------------------------
// process_dir
// -----------------------------------------------------------------------------

exit_status_t
process_dir(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_dir()");

  // Argument?
  if(int(++i) == argc || option_any(argv[i]) || endsin_json(argv[i]) ||
     endsin_cc(argv[i]) || endsin_yaml(argv[i])) {
    i--;
    return error("The " + opt +
                 " option (JSON/C++ file directory) "
                 "expects an argument.\n"
                 "Example: " +
                 opt +
                 " /some/directory/\n"
                 "The trailing / is optional.");
  }

  // Remove trailing /s (helps with later diagnostics/printing)
  const std::string dir = fixdir(argv[i]);

  // Note (before Save, so com.dir.set() is correct)
  note(std::string(com.dir.set() ? "Changing " : "Setting ") + "directory to " +
       quote(dir) + ".");

  // Save
  com.dir = dir;
  return exit_clean;
}

// -----------------------------------------------------------------------------
// one_json
// process_json
// -----------------------------------------------------------------------------

exit_status_t
one_json(const char * const * const argv, const std::size_t i, CmdArgs & com) {
  debug("one_json()");

  // Full, path-prefixed file name
  // File extension == .json?
  std::string full = fullfile(com.dir.value(), argv[i]);
  const bool json = endsin(full, ".json");

  // Look for the file as-is (file extension or not)
  std::ifstream ifs(full.c_str());
  bool found = bool(ifs);

  if(found) {
    // File found; just print a note if it doesn't end in .json
    if(!json)
      note("File name " + quote(full) +
           " doesn't end "
           "in .json, but we'll use it.");
  }
  else
    // File not found; try to be smart...
    if(json)
    // Name ends in .json; there's nothing more to try
    warning("Could not find JSON file " + quote(full) + ".");
  else {
    // Name doesn't end in .json; try appending
    std::ifstream ifs((full + ".json").c_str());
    if((found = bool(ifs))) // =, not ==
      // OK, that works, make it permanent
      full += ".json";
    else
      // Still not found; oh well
      warning("Could not find JSON file " + quote(full) + " or " +
              quote(full + ".json") + ".");
  }

  // Save
  note("Queueing database " + quote(full) + ".");
  com.database.push_back(std::make_pair(full, found));
  com.isdb.push_back(true);

  return exit_clean;
}

exit_status_t
process_json(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_json()");
  exit_status_t status = exit_clean;

  // Argument[s]?
  bool arguments = false;
  while(true) {
    if(int(++i) == argc || // no more arguments, or...
       option_any(argv[i]) || // looks like another flag
       endsin_cc(argv[i]) || // looks like a C++ file
       endsin_yaml(argv[i]) // looks like a YAML file
    ) {
      // No more arguments to the current option, so we're done
      i--;
      return arguments
               // Had at least one argument
               ? status
               // Had no arguments
               : error("The " + opt +
                       " option (JSON compilation database(s)) "
                       "expects one or more arguments.\n"
                       "Example: " +
                       opt + " foo.json bar.json");
    }

    // Have an[other] argument
    arguments = true;

    status = std::max(status, one_json(argv, i, com));
    if(status == exit_fatal)
      return status;
  }

  return status;
}

// -----------------------------------------------------------------------------
// one_cc
// process_cc
// -----------------------------------------------------------------------------

exit_status_t
one_cc(const char * const * const argv, const std::size_t i, CmdArgs & com) {
  debug("one_cc()");

  // Plain file name
  // Full, path-prefixed file name
  // File extension == .cc etc.?
  std::string plain = argv[i];
  std::string full = fullfile(com.dir.value(), plain);
  const bool cc = endsin(full, ".cc") || endsin(full, ".cpp") ||
                  endsin(full, ".cxx") || endsin(full, ".C");

  // Look for the file as-is (file extension or not)
  std::ifstream ifs(full.c_str());
  bool found = bool(ifs);

  if(found) {
    // File found; just print a note if it doesn't end in .cc etc.
    if(!cc)
      note("File name " + quote(plain) +
           " doesn't end "
           "in .cc, .cpp, .cxx, or .C, but we'll use it.");
  }
  else
    // File not found; try to be smart...
    if(cc)
    // Name ends in .cc etc.; there's nothing more to try
    warning("Could not find C++ file " + quote(full) + ".");
  else {
    // Name doesn't end in .cc etc.; try appending
    std::ifstream ifs;
    if((found = (ifs.open((full + ".cc").c_str()), bool(ifs))))
      full += ".cc", plain += ".cc";
    else if((found = (ifs.open((full + ".cpp").c_str()), bool(ifs))))
      full += ".cpp", plain += ".cpp";
    else if((found = (ifs.open((full + ".cxx").c_str()), bool(ifs))))
      full += ".cxx", plain += ".cxx";
    else if((found = (ifs.open((full + ".C").c_str()), bool(ifs))))
      full += ".C", plain += ".C";
    else
      // Still not found; oh well
      warning("Could not find C++ file " + quote(full) + ", " +
              quote(full + ".cc") + ", " + quote(full + ".cpp") + ", " +
              quote(full + ".cxx") + " or " + quote(full + ".C") + ".");
  }

  // Save...
  clang::tooling::CompileCommand ctcc;
  // ...directory
  ctcc.Directory = com.dir.value() != "" ? com.dir.value() : ".";
  // ...command clang++
  ctcc.CommandLine.push_back(
    com.clang.value() != "" ? com.clang.value() : "clang++");
  // ...command flags
  for(std::size_t s = 0; s < com.flags.value().size(); ++s)
    ctcc.CommandLine.push_back(com.flags.value()[s]);
  // ...command file
  ctcc.CommandLine.push_back(plain);
  // ...file name
  ctcc.Filename = plain;

  com.commands.push_back(std::make_pair(ctcc, found));
  com.isdb.push_back(false);

  // Note
  std::ostringstream oss;
  oss << "Queueing C++ file " << full;
  if(emit_ccline) {
    std::string str;
    for(std::size_t s = 0; s < ctcc.CommandLine.size(); ++s)
      str += "   " + ctcc.CommandLine[s] + "\n";
    oss << "\nClang tooling CompileCommand will receive:\n"
           "Directory:\n   "
        << quote(ctcc.Directory)
        << "\n"
           "CommandLine:\n"
        << str << "Filename:\n   " << quote(ctcc.Filename);
  }
  note(oss);

  return exit_clean;
}

exit_status_t
process_cc(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_cc()");
  exit_status_t status = exit_clean;

  // Argument[s]?
  bool arguments = false;
  while(true) {
    if(int(++i) == argc || // no more arguments, or...
       option_any(argv[i]) || // looks like another flag
       endsin_json(argv[i]) || // looks like a JSON file
       endsin_yaml(argv[i]) // looks like a YAML file
    ) {
      // No more arguments to the current option, so we're done
      i--;
      return arguments
               // Had at least one argument
               ? status
               // Had no arguments
               : error("The " + opt +
                       " option (C++ file(s)) "
                       "expects one or more arguments.\n"
                       "Example: " +
                       opt + " one.cc two.cpp three.cxx four.C");
    }

    // Have an[other] argument
    arguments = true;

    status = std::max(status, one_cc(argv, i, com));
    if(status == exit_fatal)
      return status;
  }

  return status;
}

// -----------------------------------------------------------------------------
// process_yaml
// -----------------------------------------------------------------------------

exit_status_t
process_yaml(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_yaml()");
  error("Unfortunately, " + opt + " isn't implemented yet. Sorry.");
  return exit_fatal;
}

// -----------------------------------------------------------------------------
// one_yout
// process_yout
// -----------------------------------------------------------------------------

exit_status_t
one_yout(const char * const * const argv, const std::size_t i, CmdArgs & com) {
  debug("one_yout()");

  const std::string file =
    endsin_yaml(argv[i]) ? argv[i] : argv[i] + std::string(".yaml");

  // Warning, if already given.
  // Note (before Save, so com.yout.set() is correct).
  if(com.yout.set())
    warning("YAML output file was previously set to " +
            quote(com.yout.value()) +
            ".\n"
            "Only the newest value will be used.");
  note(std::string(com.yout.set() ? "Changing " : "Setting ") +
       "YAML output file to " + quote(file) + ".");

  // Save
  com.yout = file;
  return exit_clean;
}

exit_status_t
process_yout(const int argc,
  const char * const * const argv,
  std::size_t & i,
  const std::string & opt,
  CmdArgs & com) {
  debug("process_yout()");

  // Argument?
  if(int(++i) == argc || option_any(argv[i]) || endsin_json(argv[i]) ||
     endsin_cc(argv[i])) {
    i--;
    return error("The " + opt +
                 " option (YAML output file) "
                 "expects an argument.\n"
                 "Example: " +
                 opt + " foo.yaml");
  }

  return one_yout(argv, i, com);
}

// -----------------------------------------------------------------------------
// default_json
// -----------------------------------------------------------------------------

// Standard JSON file name stipulated by clang
static const std::string default_json_file = "compile_commands.json";

bool
default_json(CmdArgs & com) {
  debug("default_json()");

  // Plain file name
  // Full, path-prefixed file name
  // File extension == .cc etc.?
  const std::string plain = default_json_file;
  std::string full = fullfile(com.dir.value(), plain);

  // Because we're here at all
  note("No input file(s) specified; looking for " + quote(full) + ".");

  // Look in the given directory
  std::ifstream ifs(full.c_str());
  if(!ifs) {
    if(full == plain)
      warning("Could not find JSON file " + quote(full) + ".");
    else {
      // Look in the current directory
      note("No input file(s) specified; looking for " + quote(plain) + ".");
      if(ifs.open(plain.c_str()), bool(ifs))
        full = plain;
      else
        warning("Could not find JSON file " + quote(full) + " or " +
                quote(plain) + ".");
    }
  }

  // Still not found
  if(!ifs) // not necessarily the same test as above; notice the ifs.open()
    return false;

  // Save
  note("Queueing database " + quote(full) + ".");
  com.database.push_back(std::make_pair(full, true));
  com.isdb.push_back(true);
  return true;
}

// -----------------------------------------------------------------------------
// option_toggle - helper
// -----------------------------------------------------------------------------

// fixme
// Another part of the maintenance headache.
bool
option_toggle(const std::string & opt) {
  // version, help
  if(option_version(opt)) {
    print_version();
  }
  if(option_help(opt)) {
    print_help();
  }

  // quiet?
  else if(option_quiet(opt)) {
    emit_debug = false;
    emit_heading = false;
    emit_note = false;
    emit_warning = false;
    emit_section_command = false;
    emit_section_compile = false;
    emit_section_summary = false;
    // Make no changes, yea or nay, regarding:
    //    - printing of the analysis section
    //    - printing of file names
    //    - printing of reports
    //    - printing of errors
    // Users who really don't want to see the analysis section,
    // etc., can explicitly set the necessary flags.
  }

  // verbose?
  else if(option_verbose(opt)) {
    emit_heading = true;
    emit_file = true;
    emit_report = true;
    emit_note = true;
    emit_warning = true;
    emit_error = true;
    emit_section_command = true;
    emit_section_compile = true;
    emit_section_analysis = true;
    emit_section_summary = true;
  }

  // short, long
  else if(option_short(opt)) {
    short_form = true;
  }
  else if(option_long(opt)) {
    short_form = false;
  }

  // print?
  else if(option_print(opt)) {
    emit_color = false;
    emit_formfeed = true;
  }

  else if(option_debug(opt)) {
    emit_debug = true;
  }
  else if(option_no_debug(opt)) {
    emit_debug = false;
  }
  else if(option_heading(opt)) {
    emit_heading = true;
  }
  else if(option_no_heading(opt)) {
    emit_heading = false;
  }
  else if(option_file(opt)) {
    emit_file = true;
  }
  else if(option_no_file(opt)) {
    emit_file = false;
  }
  else if(option_report(opt)) {
    emit_report = true;
  }
  else if(option_no_report(opt)) {
    emit_report = false;
  }
  else if(option_note(opt)) {
    emit_note = true;
  }
  else if(option_no_note(opt)) {
    emit_note = false;
  }
  else if(option_warning(opt)) {
    emit_warning = true;
  }
  else if(option_no_warning(opt)) {
    emit_warning = false;
  }
  else if(option_error(opt)) {
    emit_error = true;
  }
  else if(option_no_error(opt)) {
    emit_error = false;
  }

  else if(option_command(opt)) {
    emit_section_command = true;
  }
  else if(option_no_command(opt)) {
    emit_section_command = false;
  }
  else if(option_compile(opt)) {
    emit_section_compile = true;
  }
  else if(option_no_compile(opt)) {
    emit_section_compile = false;
  }
  else if(option_analysis(opt)) {
    emit_section_analysis = true;
  }
  else if(option_no_analysis(opt)) {
    emit_section_analysis = false;
  }
  else if(option_summary(opt)) {
    emit_section_summary = true;
  }
  else if(option_no_summary(opt)) {
    emit_section_summary = false;
  }

  else if(option_column(opt)) {
    emit_column = true;
  }
  else if(option_no_column(opt)) {
    emit_column = false;
  }
  else if(option_ccline(opt)) {
    emit_ccline = true;
  }
  else if(option_no_ccline(opt)) {
    emit_ccline = false;
  }

  else if(option_color(opt)) {
    emit_color = true;
  }
  else if(option_no_color(opt)) {
    emit_color = false;
  }
  else if(option_trace(opt)) {
    emit_trace = true;
  }
  else if(option_no_trace(opt)) {
    emit_trace = false;
  }

  else
    return false;

  return true;
}

// -----------------------------------------------------------------------------
// initial
// arguments
//
// Process command-line arguments.
//
// The two functions (initial() and arguments()) are split, so that things that
// might affect the latter's output are processed before such output can occur.
// -----------------------------------------------------------------------------

exit_status_t
initial(
  // counter, to coordinate with arguments() below
  std::size_t & i, // arrives as 0

  // input
  const int argc,
  const char * const * const argv,

  // output
  CmdArgs & com) {
  // These just shouldn't happen
  if(argc <= 0)
    return interr("argc <= 0. This shouldn't happen.");
  if(argv == nullptr)
    return interr("argv == nullptr. This shouldn't happen.");

  // Executable name
  com.exe = argv[0];

  // Arguments
  while(int(++i) < argc) {
    const std::string opt = argv[i];
    if(!option_toggle(opt))
      return --i, exit_clean;
  }

  return exit_clean;
}

exit_status_t
arguments(
  // counter, to coordinate with initial() above
  std::size_t & i,

  // input
  const int argc,
  const char * const * const argv,

  // output
  CmdArgs & com) {
  debug("arguments()");
  exit_status_t status = exit_clean;

  // Arguments
  while(int(++i) < argc) {
    const std::string opt = argv[i];

    // Simple option?
    if(option_toggle(opt)) {
      // work already done
    }

// More-complex option?
#define ARGS argc, argv, i, opt, com
    else if(option_dir(opt)) {
      if(process_dir(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_json(opt)) {
      if(process_json(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_cc(opt)) {
      if(process_cc(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_yaml(opt)) {
      if(process_yaml(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_clang(opt)) {
      if(process_clang(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_flags(opt)) {
      if(process_flags(ARGS) == exit_fatal)
        return bail();
    }

    else if(option_yout(opt)) {
      if(process_yout(ARGS) == exit_fatal)
        return bail();
    }
#undef ARGS

    // Direct-specified JSON or C++ file?
    else if(endsin_json(opt)) {
      if(one_json(argv, i, com) == exit_fatal)
        return bail();
    }
    else if(endsin_cc(opt)) {
      if(one_cc(argv, i, com) == exit_fatal)
        return bail();
    }

    // Direct-specified YAML file? This is an error, actually, because
    // we wouldn't know if it's an input or output YAML file. (Contrast
    // this with JSON and C++ files, which can only be input.)
    else if(endsin_yaml(opt)) {
      error("Ambiguous command-line argument: " + quote(opt) +
            "."
            "\nYAML files can be either input or output."
            "\nPrefix with -yaml if INPUT."
            "\nPrefix with -yout if OUTPUT.");
      return bail();
    }

    // Who knows?
    else {
      status = std::max(status,
        opt == ""
          ? error("Blank command-line argument: " + quote(opt) + ".")
          : opt[0] == '-'
              ? error("Unknown command-line flag: " + quote(opt) + ".")
              : error("Ambiguous command-line argument: " + quote(opt) +
                      "."
                      "\nIf a JSON file, use -json or suffix file with .json."
                      "\nIf a C++ file, use -cc "
                      "or suffix file with .cc, .cpp, .cxx, or .C."
                      "\nIf a YAML input file, use -yaml."
                      "\nIf a YAML output file, use -yout."));
    }
  }

  // Explicit problem (which would have already been handled, diagnostic-wise)
  if(status != exit_clean)
    return status;

  // Implicit problem (no input files were given, or found!)
  if(!(com.database.size() || // no given JSON
       com.commands.size() || // no given C++
       default_json(com))) // no default JSON
    return error("No input file(s) specified, and default (" +
                 default_json_file +
                 ") not found.\n"
                 "You can provide JSON and/or C++ input files.");

  // No problem
  return exit_clean;
}

} // namespace flecstan
