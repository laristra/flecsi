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

#include "flecstan-config.h"
#include "clang/Sema/Sema.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/YAMLTraits.h"
#include <iostream>
#include <set>
#include <sstream>

// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

// Perform assertions?
#define FLECSTAN_ASSERT

// Exit status: type, values
using exit_status_t = int;
inline const exit_status_t exit_clean = 0;
inline const exit_status_t exit_error = 1;
inline const exit_status_t exit_fatal = 2;

// flecstan_Assert (unconditional)
// flecstan_assert
#define flecstan_Assert(x) assert(x)

#ifdef FLECSTAN_ASSERT
#define flecstan_assert(x) flecstan_Assert(x)
#else
#define flecstan_assert(x)
#endif

// stringify
#define _stringify(macro) #macro
#define stringify(macro) _stringify(macro)

// version
namespace flecstan {
inline const std::string version =
  stringify(flecstan_VERSION_MAJOR) "." stringify(
    flecstan_VERSION_MINOR) "." stringify(flecstan_VERSION_PATCH);
}

// unnamed_namespace
namespace flecstan {
inline const std::string unnamed_namespace = "<namespace>";
}

// -----------------------------------------------------------------------------
// Colors, for text
// See: https://stackoverflow.com/questions/2616906/
// Bold actually means bold *or* "bright"
// E.g. bold::black looks gray to me
// -----------------------------------------------------------------------------

namespace flecstan {
namespace color {

// bold
namespace bold {
inline const std::string black = "\033[30;1m";
inline const std::string red = "\033[31;1m";
inline const std::string green = "\033[32;1m";
inline const std::string yellow = "\033[33;1m";
inline const std::string blue = "\033[34;1m";
inline const std::string magenta = "\033[35;1m";
inline const std::string cyan = "\033[36;1m";
inline const std::string white = "\033[37;1m";
} // namespace bold

// lite
namespace lite {
inline const std::string black = "\033[30;21m";
inline const std::string red = "\033[31;21m";
inline const std::string green = "\033[32;21m";
inline const std::string yellow = "\033[33;21m";
inline const std::string blue = "\033[34;21m";
inline const std::string magenta = "\033[35;21m";
inline const std::string cyan = "\033[36;21m";
inline const std::string white = "\033[37;21m";
} // namespace lite

// for output
inline std::string debug;
inline std::string title;
inline std::string file;
inline std::string report1;
inline std::string report2;
inline std::string note;
inline std::string warning;
inline std::string error;
inline std::string fatal;
inline std::string reset;

} // namespace color
} // namespace flecstan

// -----------------------------------------------------------------------------
// markup_t
// -----------------------------------------------------------------------------

namespace flecstan {

class markup_t
{
  std::string _prefix = "";
  std::string _suffix = "";

public:
  // constructor
  markup_t() {
    // initialize color scheme
    ansi();
  }

  // begin()
  void begin() const {
    // print markup prefix
    static auto & once = std::cout << _prefix;
    (void)once;
  }

  // destructor
  ~markup_t() {
    // print markup suffix
    static auto & once = std::cout << _suffix;
    (void)once;
  }

  // ------------------------
  // prefix, suffix
  // ------------------------

  void prefix(const std::string & p) {
    _prefix = p;
  }
  void suffix(const std::string & s) {
    _suffix = s;
  }

  // ------------------------
  // schemes
  // ------------------------

  // ansi
  void ansi() {
    _prefix = "";
    color::debug = color::bold::white;
    color::title = color::bold::black;
    color::file = color::lite::magenta;
    color::report1 = color::bold::blue;
    color::report2 = color::bold::cyan;
    color::note = color::lite::green;
    color::warning = color::bold::yellow;
    color::error = color::lite::red;
    color::fatal = color::lite::red;
    color::reset = "\033[0m";
    _suffix = "";
  }

  // html
  void html(const std::string & indent = "") {
    _prefix = "";
    color::debug = indent + "<span style=\"color:lightgray\">";
    color::title = indent + "<span style=\"color:dimgray\">";
    color::file = indent + "<span style=\"color:magenta\">";
    color::report1 = indent + "<span style=\"color:blue\">";
    color::report2 = indent + "<span style=\"color:darkturquoise\">";
    color::note = indent + "<span style=\"color:green\">";
    color::warning = indent + "<span style=\"color:orange\">";
    color::error = indent + "<span style=\"color:red\">";
    color::fatal = indent + "<span style=\"color:red\">";
    color::reset = "</span>";
    _suffix = "";
  }

  // rst
  void rst() {
    html("   ");
    // modify from html()'s...
    _prefix = ".. raw:: html\n\n   <pre class=\"code literal-block\">\n";
    _suffix = "   </pre>\n";
  }

  // tex
  void tex() {
    _prefix = "";
    color::debug = "\\textWhite{}";
    color::title = "\\textGray{}";
    color::file = "\\textMagenta{}";
    color::report1 = "\\textBlueViolet{}";
    color::report2 = "\\textCyan{}";
    color::note = "\\textGreen{}";
    color::warning = "\\textYellow{}";
    color::error = "\\textRed{}";
    color::fatal = "\\textRed{}";
    color::reset = "\\textBlack{}";
    _suffix = "";
  }

  // tex_listing
  void tex_listing() {
    _prefix = "";
    color::debug = "`textWhite``";
    color::title = "`textGray``";
    color::file = "`textMagenta``";
    color::report1 = "`textBlueViolet``";
    color::report2 = "`textCyan``";
    color::note = "`textGreen``";
    color::warning = "`textYellow``";
    color::error = "`textRed``";
    color::fatal = "`textRed``";
    color::reset = "`textBlack";
    _suffix = "";
  }
};

inline markup_t markup;

} // namespace flecstan

// -----------------------------------------------------------------------------
// Diagnostics / informational printing
// -----------------------------------------------------------------------------

namespace flecstan {

// diagnostic (helper)
void diagnostic(const std::string &, // label
  const std::string &, // text
  std::string, // color: label
  std::string = "", // color: text
  const bool = false, // force long form
  const bool = true // ":" after label
);

// ------------------------
// Flags
// ------------------------

// re: sections
inline bool emit_section_command = true;
inline bool emit_section_compilation = true;
inline bool emit_section_analysis = true;
inline bool emit_section_summary = true;

// re: types of printing
inline bool emit_debug = false;
inline bool emit_title = true;
inline bool emit_file = true;
inline bool emit_report = true;
inline bool emit_note = true;
inline bool emit_warning = true;
inline bool emit_error = true;

// re: certain extra printing
inline bool emit_color = true;
inline bool emit_trace = true;
inline bool emit_column = false;
inline bool emit_ccdetail = false;
inline bool emit_formfeed = false;
inline bool emit_macro = true;
inline bool emit_link = true;
inline bool emit_scan = true;
inline bool emit_visit = true;

// re: file name printing
inline bool file_medium = false;
inline bool file_short = false;
inline bool file_strip = false;

// other
inline bool section_on = true;
inline bool short_form = false;

// ------------------------
// debug
// ------------------------

void debug(const std::string &);

inline void
debug(const std::ostringstream & oss) {
  debug(oss.str());
}

template<class T>
inline void
debug(const std::string & varname, const T & value) {
  std::ostringstream oss;
  oss << varname << " == " << value;
  debug(oss);
}

#define flecstan_debug(var) ::flecstan::debug(#var, (var))

// ------------------------
// title
// ------------------------

void title(const std::string &,
  const bool,
  const int length = 80,
  const std::string & = color::title);

inline void
title(const std::ostringstream & oss, const bool emit_section) {
  title(oss.str(), emit_section);
}

// ------------------------
// filename
// ------------------------

// stripdir (helper)
inline std::string
stripdir(const std::string & in) {
  if(!file_strip)
    return in;
  const std::size_t pos = in.rfind("/");
  return pos == std::string::npos ? in : in.substr(pos + 1);
}

// stdin
inline const std::string stdin = "<standard input>";

// filename
inline void
filename(std::string outer, std::string inner) {
  // -file-long
  //    File:
  //       a/b/c/foo.json
  //    File:
  //       a/b/c/foo.json: d/e/f/g/bar.cc

  // -file-medium
  //    File:
  //       d/e/f/g/bar.cc

  // -file-short
  //    d/e/f/g/bar.cc

  // -file-long -file-strip
  //    File:
  //       foo.json
  //    File:
  //       foo.json: bar.cc

  // -file-medium -file-strip
  //    File:
  //       bar.cc

  // -file-short -file-strip
  //    bar.cc

  if(section_on && emit_file) {
    if(outer == "-")
      outer = stdin;
    if(inner == "-")
      inner = stdin;

    const std::string o = file_medium || file_short ? "" : stripdir(outer);

    const std::string i = stripdir(inner);

    const std::string name = o == "" ? i : i == "" ? o : o + ": " + i;

    if(name != "")
      diagnostic(file_short ? "" : "File", name, color::file);
  }
}

// ------------------------
// report
// ------------------------

inline exit_status_t
report(const std::string & label,
  const std::string & str,
  const bool keep_long_form,
  const bool colon = true) {
  if(section_on && emit_report)
    diagnostic(
      label, str, color::report1, color::report2, keep_long_form, colon);
  return exit_clean;
}

inline exit_status_t
report(const std::string & label,
  const std::ostringstream & oss,
  const bool keep_long_form,
  const bool colon = true) {
  return report(label, oss.str(), keep_long_form, colon);
}

// ------------------------
// note
// ------------------------

inline exit_status_t
note(const std::string & str) {
  if(section_on && emit_note)
    diagnostic("Note", str, color::note);
  return exit_clean;
}

inline exit_status_t
note(const std::ostringstream & oss) {
  return note(oss.str());
}

// ------------------------
// warning
// ------------------------

inline std::size_t num_warn = 0;

inline exit_status_t
warning(const std::string & str) {
  num_warn++;
  if(section_on && emit_warning)
    diagnostic("Warning", str, color::warning);
  return exit_clean;
}

inline exit_status_t
warning(const std::ostringstream & oss) {
  return warning(oss.str());
}

inline exit_status_t
intwarn(const std::string & str) {
  return warning("Internal warning from the FleCSI Static Analyzer.\n" + str);
}

inline exit_status_t
intwarn(const std::ostringstream & oss) {
  return intwarn(oss.str());
}

// ------------------------
// error
// ------------------------

inline std::size_t num_error = 0;

inline exit_status_t
error(const std::string & str) {
  // section_on is not required here, as it was with titles, files,
  // reports notes, and warnings. We always print errors, unless the
  // user has explicitly shut them off
  num_error++;
  if(emit_error)
    diagnostic("Error", str, color::error);
  return exit_error;
}

inline exit_status_t
error(const std::ostringstream & oss) {
  return error(oss.str());
}

// ------------------------
// fatal
// interr
// ------------------------

inline exit_status_t
fatal(const std::string & str) {
  // these always print, regardless of either section_on or emit_error
  num_error++;
  diagnostic("ERROR (Terminal)", str, color::fatal);
  return exit_fatal;
}

inline exit_status_t
fatal(const std::ostringstream & oss) {
  return fatal(oss.str());
}

// interr
inline exit_status_t
interr(const std::string & str) {
  return fatal("Internal Error: " + str);
}

inline exit_status_t
interr(const std::ostringstream & oss) {
  return interr(oss.str());
}

// ------------------------
// print_version
// print_help (declaration)
// ------------------------

inline void
print_version() {
  static bool first = true;
  if(!first)
    return;

  // absolutely plain old printing; no colorization
  std::cout << "flecstan (FleCSI Static Analyzer) version " << version
            << std::endl;

  first = false;
}

void print_help(const bool);

} // namespace flecstan

// -----------------------------------------------------------------------------
// Helper constructs
// -----------------------------------------------------------------------------

namespace flecstan {

// quote
inline std::string
quote(const std::string & str) {
  // Actually, try plain unquoted for now, not the following
  //    return '"' + str + '"';
  // But return '""' if empty, so that those diagnostics look sensible.
  return str == "" ? "\"\"" : str;
}

// endsin: helper
// C++20 will have an ends_with(); I'll just do it directly for now
inline bool
endsin(const std::string & str, const std::string & end) {
  return str.size() >= end.size() && &str[str.size() - end.size()] == end;
}

// TokenName
// Get a clang::Token's spelling
inline std::string
TokenName(const clang::Token & token, const clang::Sema & sema) {
  return clang::Lexer::getSpelling(
    token, sema.getSourceManager(), sema.getLangOpts());
}

// booland
template<class T>
class booland
{
  T _value;
  bool _set;

public:
  // constructor: default
  booland() : _set(false) {}

  // constructor: T
  explicit booland(const T & from) : _value(from), _set(true) {}

  // assignment
  booland<T> & operator=(const T & from) {
    _value = from;
    _set = true;
    return *this;
  }

  // has it been set?
  bool set() const {
    return _set;
  }

  // retrieve value
  const T & value() const {
    return _value;
  }
  T & value() {
    return _value;
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// FileLineColumn
// File, line, and column, all as strings (can be, e.g., "unknown")
// -----------------------------------------------------------------------------

namespace flecstan {

// FileLineColumn
class FileLineColumn
{
public:
  std::string file, line, column;

  FileLineColumn() : file(""), line(""), column("") {}

  FileLineColumn(const std::string & f,
    const std::string & l,
    const std::string & c)
    : file(f), line(l), column(c) {}

  bool operator==(const FileLineColumn & rhs) const {
    return file == rhs.file && line == rhs.line && column == rhs.column;
  }
};

// print_flc
// Helper function
inline std::string
print_flc(
  // labels
  const std::string & labf,
  const std::string & labl,
  const std::string & labc,

  // (file,line,column)s
  const FileLineColumn & location,
  const FileLineColumn & spelling) {
  std::ostringstream oss;

  const bool fsame = location.file == spelling.file;
  const bool lsame = location.line == spelling.line;
  const bool csame = location.column == spelling.column;

  // file
  oss << labf << stripdir(location.file);
  if(!fsame)
    oss << "[" << stripdir(spelling.file) << "]";

  // line
  // Remark: morally, the same numeric line, in a different file, is different
  oss << labl << location.line;
  if(!fsame || !lsame)
    oss << "[" << spelling.line << "]";

  // column
  // Remark as above
  if(emit_column) {
    oss << labc << location.column;
    if(!fsame || !lsame || !csame)
      oss << "[" << spelling.column << "]";
  }

  return oss.str();
}

// getFileLineColumn
void getFileLineColumn(const clang::SourceManager * const,
  const clang::SourceLocation &,
  FileLineColumn &);

template<class OBJ>
void
getFileLineColumn(
  // obj needs hasSourceManager(), getSourceManager(), and getLocation()
  const OBJ & obj,
  FileLineColumn & flc) {
  getFileLineColumn(obj.hasSourceManager() ? &obj.getSourceManager() : nullptr,
    obj.getLocation(), flc);
}

} // namespace flecstan

namespace llvm {
namespace yaml {

// MappingTraits<FileLineColumn>
template<>
class MappingTraits<flecstan::FileLineColumn>
{
public:
  static void mapping(IO & io, flecstan::FileLineColumn & c) {
    io.mapRequired("file", c.file);
    io.mapRequired("line", c.line);
    io.mapRequired("column", c.column);
  }
};

} // namespace yaml
} // namespace llvm

// -----------------------------------------------------------------------------
// MacroCall
// Contains information regarding one call to any of the various FleCSI macros
// we'll be looking for.
// -----------------------------------------------------------------------------

namespace flecstan {

class MacroCall
{
public:
  // ------------------------
  // Data
  // ------------------------

  std::string unit; // file we're compiling
  std::string macname; // name of macro
  FileLineColumn location; // nominal location
  FileLineColumn spelling; // spelling location
  mutable bool ast; // AST matched?

  // Arguments
  std::vector<std::vector<clang::Token>> argstok; // as vectors of tokens
  std::vector<std::string> argsraw; // in raw form

  // ------------------------
  // Constructor
  // ------------------------

  MacroCall(const std::string & _unit,
    const clang::Token & token,
    const clang::SourceManager & sman,
    const std::string & _macname)
    : unit(_unit), macname(_macname), ast(false) {
    const clang::SourceLocation tloc = token.getLocation();
    getFileLineColumn(&sman, tloc, location);
    getFileLineColumn(&sman, sman.getSpellingLoc(tloc), spelling);
  }

  // ------------------------
  // Functions
  // ------------------------

  // Number of arguments to the macro
  std::size_t size() const {
    return argstok.size();
  }

  // Location of argument [a], token [t]
  clang::SourceLocation loc(const std::size_t a, const std::size_t t) const {
    flecstan_assert(a < argstok.size());
    flecstan_assert(t < argstok[a].size());
    return argstok[a][t].getLocation();
  }

  // String representation of argument [a], token [t]
  std::string str(const clang::Sema & sema,
    const std::size_t a,
    const std::size_t t) const {
    flecstan_assert(a < argstok.size());
    flecstan_assert(t < argstok[a].size());
    return TokenName(argstok[a][t], sema);
  }

  // String representation of argument [a], with all tokens put together,
  // separated by spaces.
  // 2018-02-08: For now let's change this, and NOT separate by spaces.
  std::string str(const clang::Sema & sema, const std::size_t a) const {
    flecstan_assert(a < argstok.size());
    std::string s;
    for(std::size_t t = 0; t < argstok[a].size(); ++t)
      s += /* (t ? " " : "") + */ str(sema, a, t);
    return s;
  }

  // ------------------------
  // Printing
  // ------------------------

  // flc
  // print file, line, column
  std::string flc() const {
    return print_flc("file ", ", line ", ", column ", location, spelling);
  }

  // report
  // print a report regarding this macro call
  void report(const clang::Sema & sema) const {
    std::ostringstream oss;
    oss << "Name: " << macname
        << "\n"
           // "Unit: " << unit << "\n" // might be confusing here
           "Args:";
    for(std::size_t arg = 0; arg < size(); ++arg)
      oss << (arg ? ", " : " ") << str(sema, arg);

    if(emit_macro)
      flecstan::report("Macro",
        oss.str() +
          print_flc("\nFile: ", "\nLine: ", "\nColumn: ", location, spelling),
        true);
  }
};

} // namespace flecstan

namespace llvm {
namespace yaml {

// MappingTraits<MacroCall>
template<>
class MappingTraits<flecstan::MacroCall>
{
public:
  static void mapping(IO & io, flecstan::MacroCall & c) {
    // Don't map c.macname. The containers of MacroCall objects are given
    // the same names as the macros, and so c.macname would be redundant.
    // Don't map c.argstok either, as clang::Token isn't really agreeable
    // with doing so. Essentially the same information is in argsraw.
    io.mapRequired("unit", c.unit);
    io.mapRequired("location", c.location);
    io.mapRequired("spelling", c.spelling);
    io.mapRequired("matched", c.ast);
    io.mapRequired("arguments", c.argsraw);
  }
};

} // namespace yaml
} // namespace llvm

// -----------------------------------------------------------------------------
// CalledMatched
// -----------------------------------------------------------------------------

namespace flecstan {

template<class T>
class CalledMatched
{
public:
  std::vector<MacroCall> called;
  std::vector<T> matched;
};

} // namespace flecstan

namespace llvm {
namespace yaml {

template<class T>
class MappingTraits<flecstan::CalledMatched<T>>
{
public:
  static void mapping(IO & io, flecstan::CalledMatched<T> & c) {
    io.mapRequired("called", c.called);
    io.mapRequired("matched", c.matched);
  }
};

} // namespace yaml
} // namespace llvm
