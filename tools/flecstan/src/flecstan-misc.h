/* -*- C++ -*- */

#ifndef flecstan_misc
#define flecstan_misc

#include "clang/Sema/Sema.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/YAMLTraits.h"
#include <iostream>
#include <sstream>

// Perform assertions?
#define FLECSTAN_ASSERT

// Exit status: type, values
using exit_status_t = int;
inline const exit_status_t exit_clean = 0;
inline const exit_status_t exit_error = 1;
inline const exit_status_t exit_fatal = 2;

// flecstan_Assert (unconditional)
// flecstan_assert
#define flecstan_Assert(x) \
   assert(x)

#ifdef FLECSTAN_ASSERT
   #define flecstan_assert(x) flecstan_Assert(x)
#else
   #define flecstan_assert(x)
#endif

// version
namespace flecstan {
   inline const std::string version = "0.0.0";
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
      inline const std::string black   = "\033[30;1m";
      inline const std::string red     = "\033[31;1m";
      inline const std::string green   = "\033[32;1m";
      inline const std::string yellow  = "\033[33;1m";
      inline const std::string blue    = "\033[34;1m";
      inline const std::string magenta = "\033[35;1m";
      inline const std::string cyan    = "\033[36;1m";
      inline const std::string white   = "\033[37;1m";
   }

   // lite
   namespace lite {
      inline const std::string black   = "\033[30;21m";
      inline const std::string red     = "\033[31;21m";
      inline const std::string green   = "\033[32;21m";
      inline const std::string yellow  = "\033[33;21m";
      inline const std::string blue    = "\033[34;21m";
      inline const std::string magenta = "\033[35;21m";
      inline const std::string cyan    = "\033[36;21m";
      inline const std::string white   = "\033[37;21m";
   }

   // for output
   inline const std::string debug   = bold::white;
   inline const std::string heading = bold::black;
   inline const std::string file    = lite::magenta;
   inline const std::string report1 = bold::blue;
   inline const std::string report2 = bold::cyan;
   inline const std::string note    = lite::green;
   inline const std::string warning = bold::yellow;
   inline const std::string error   = lite::red;
   inline const std::string fatal   = lite::red;
   inline const std::string reset   = "\033[0m";
   /*
   inline const std::string debug   = "`textWhite ";
   inline const std::string heading = "`textGray ";
   inline const std::string file    = "`textMagenta ";
   inline const std::string report1 = "`textBlueViolet ";
   inline const std::string report2 = "`textCyan ";
   inline const std::string note    = "`textGreen ";
   inline const std::string warning = "`textYellow ";
   inline const std::string error   = "`textRed ";
   inline const std::string fatal   = "`textRed ";
   inline const std::string reset   = "`textBlack";
   */

} // namespace color
} // namespace flecstan



// -----------------------------------------------------------------------------
// Diagnostics / informational printing
// -----------------------------------------------------------------------------

namespace flecstan {

// diagnostic (helper)
void diagnostic(
   const std::string &, // label
   const std::string &, // text
   std::string,         // color: label
   std::string = "",    // color: text
   const bool  = false  // force long form
);

// ------------------------
// Flags
// ------------------------

// re: sections
inline bool emit_section_command  = true;
inline bool emit_section_compile  = true;
inline bool emit_section_analysis = true;
inline bool emit_section_summary  = true;

// re: types of printing
inline bool emit_debug   = false;
inline bool emit_heading = true;
inline bool emit_file    = true;
inline bool emit_report  = true;
inline bool emit_note    = true;
inline bool emit_warning = true;
inline bool emit_error   = true;

// re: certain extra printing
inline bool emit_color    = true;
inline bool emit_trace    = true;
inline bool emit_column   = false;
inline bool emit_ccline   = false;
inline bool emit_formfeed = false;

// bookkeeping
inline bool section_on = true;
inline bool short_form = false;



// ------------------------
// debug
// ------------------------

void debug(const std::string &);

inline void debug(const std::ostringstream &oss)
{
   debug(oss.str());
}

template<class T>
inline void debug(const std::string &name, const T &value)
{
   std::ostringstream oss;
   oss << name << " == " << value;
   debug(oss);
}

#define flecstan_debug(var) ::flecstan::debug(#var,(var))



// ------------------------
// heading
// ------------------------

void heading(const std::string &, const bool);

inline void heading(const std::ostringstream &oss, const bool emit_section)
{
   heading(oss.str(),emit_section);
}



// ------------------------
// filename
// ------------------------

inline void filename(const std::string &str)
{
   if (section_on && emit_file)
      diagnostic("File", str, color::file);
}

inline void filename(const std::ostringstream &oss)
{
   filename(oss.str());
}



// ------------------------
// report
// ------------------------

inline exit_status_t report(
   const std::string &label,
   const std::string &str
) {
   if (section_on && emit_report)
      diagnostic(label, str, color::report1, color::report2, true);
   return exit_clean;
}

inline exit_status_t report(
   const std::string &label,
   const std::ostringstream &oss
) {
   return report(label,oss.str());
}



// ------------------------
// note
// ------------------------

inline exit_status_t note(const std::string &str)
{
   if (section_on && emit_note)
      diagnostic("Note", str, color::note);
   return exit_clean;
}

inline exit_status_t note(const std::ostringstream &oss)
{
   return note(oss.str());
}



// ------------------------
// warning
// ------------------------

inline exit_status_t warning(const std::string &str)
{
   if (section_on && emit_warning)
      diagnostic("Warning", str, color::warning);
   return exit_clean;
}

inline exit_status_t warning(const std::ostringstream &oss)
{
   return warning(oss.str());
}

inline exit_status_t intwarn(
   const std::string &str,
   // Certain internal warnings can be triggered by compilation errors.
   // I'll assume where otherwise unspecified that this isn't the case,
   // but will print a more forgiving-sounding message where it may be.
   const bool can_be_err_triggered = false
) {
   return warning(
      "Internal warning from the FleCSI Static Analyzer.\n" + str + "\n" +
      (can_be_err_triggered
       ? "This warning may be spurious, if your code has errors "
         "or certain warnings.\n"
         "Otherwise, please report this warning to us."
       : "Please report this warning to us.\n"
   ));
}

inline exit_status_t intwarn(
   const std::ostringstream &oss,
   const bool err_triggered = false
) {
   return intwarn(oss.str(),err_triggered);
}



// ------------------------
// error
// ------------------------

inline exit_status_t error(const std::string &str)
{
   // section_on is not required here, as it was with headings, files,
   // reports, notes, and warnings. We always print errors, unless the
   // user has explicitly shut them off
   if (emit_error)
      diagnostic("Error", str, color::error);
   return exit_error;
}

inline exit_status_t error(const std::ostringstream &oss)
{
   return error(oss.str());
}



// ------------------------
// fatal
// interr
// ------------------------

inline exit_status_t fatal(const std::string &str)
{
   // these always print, regardless of either section_on or emit_error
   diagnostic("ERROR (Terminal)", str, color::fatal);
   return exit_fatal;
}

inline exit_status_t fatal(const std::ostringstream &oss)
{
   return fatal(oss.str());
}

// interr
inline exit_status_t interr(const std::string &str)
{
   return fatal("Internal Error: " + str);
}

inline exit_status_t interr(const std::ostringstream &oss)
{
   return interr(oss.str());
}



// ------------------------
// print_version
// print_help
// ------------------------

inline void print_version()
{
   // absolutely plain old printing; no colorization
   std::cout
      << "flecstan (FleCSI Static Analyzer) version " << version
      <<  std::endl;
}

inline void print_help()
{
   report(
      "Help",
      "Sorry, no command-line help is available at this time."
   );
}

} // namespace flecstan



// -----------------------------------------------------------------------------
// Helper constructs
// -----------------------------------------------------------------------------

namespace flecstan {

// quote
inline std::string quote(const std::string &str)
{
   // Actually, try plain unquoted for now, not the following
   //    return '"' + str + '"';
   // But return '""' if empty, so that those diagnostics look sensible.
   return str == "" ? "\"\"" : str;
}


// endsin: helper
// C++20 will have an ends_with(); I'll just do it directly for now
inline bool endsin(const std::string &str, const std::string &end)
{
   return str.size() >= end.size() && &str[str.size()-end.size()] == end;
}


// TokenName
// Get a clang::Token's spelling
inline std::string TokenName(
   const clang::Token &token,
   const clang::Sema  &sema
) {
   return clang::Lexer::getSpelling(
      token,
      sema.getSourceManager(),
      sema.getLangOpts()
   );
}


// booland
template<class T>
class booland {
   T _value;
   bool _set;

public:

   // constructor: default
   booland() : _set(false) { }

   // constructor: T
   explicit booland(const T &from) : _value(from), _set(true) { }

   // assignment
   booland<T> &operator=(const T &from)
   {
      _value = from;
      _set   = true;
      return *this;
   }

   // has it been set?
   bool set() const { return _set; }

   // retrieve value
   const T &value() const { return _value; }
   T &value() { return _value; }
};

} // namespace flecstan



// -----------------------------------------------------------------------------
// FileLineColumn
// File, line, and column, all as strings (can be, e.g., "unknown")
// -----------------------------------------------------------------------------

namespace flecstan {

// FileLineColumn
class FileLineColumn {
public:
   std::string file, line, column;

   FileLineColumn() :
      file  (""),
      line  (""),
      column("")
   { }

   FileLineColumn(
      const std::string &f,
      const std::string &l,
      const std::string &c
   ) :
      file  (f),
      line  (l),
      column(c)
   { }
};



// print_flc
// Helper function
inline std::string print_flc(
   // labels
   const std::string &labf,
   const std::string &labl,
   const std::string &labc,

   // (file,line,column)s
   const FileLineColumn &location,
   const FileLineColumn &spelling
) {
   std::ostringstream oss;

   const bool fsame = location.file   == spelling.file;
   const bool lsame = location.line   == spelling.line;
   const bool csame = location.column == spelling.column;

   // file
   oss << labf << location.file;
   if (!fsame)
      oss << "[" << spelling.file << "]";

   // line
   // Remark: morally, the same numeric line, in a different file, is different
   oss << labl << location.line;
   if (!fsame || !lsame)
      oss << "[" << spelling.line << "]";

   // column
   // Remark as above
   if (emit_column) {
      oss << labc << location.column;
      if (!fsame || !lsame || !csame)
         oss << "[" << spelling.column << "]";
   }

   return oss.str();
}



// getFileLineColumn
void getFileLineColumn(
   const clang::SourceManager *const,
   const clang::SourceLocation &,
   FileLineColumn &
);

template<class OBJ>
void getFileLineColumn(
   // obj needs hasSourceManager(), getSourceManager(), and getLocation()
   const OBJ &obj,
   FileLineColumn &flc
) {
   getFileLineColumn(
      obj.hasSourceManager() ? &obj.getSourceManager() : nullptr,
      obj.getLocation(),
      flc
   );
}

} // namespace flecstan



namespace llvm {
namespace yaml {

// MappingTraits<FileLineColumn>
template<>
class MappingTraits<flecstan::FileLineColumn> {
public:
   static void mapping(IO &io, flecstan::FileLineColumn &c)
   {
      io.mapRequired("file",   c.file);
      io.mapRequired("line",   c.line);
      io.mapRequired("column", c.column);
   }
};

} // namespace yaml
} // namespace llvm



// -----------------------------------------------------------------------------
// MacroInvocation
// Contains information regarding an invocation of any of the various FleCSI
// macros we'll be looking for.
// -----------------------------------------------------------------------------

namespace flecstan {

class MacroInvocation {
public:

   // ------------------------
   // Data
   // ------------------------

   std::string name;
   FileLineColumn location;
   FileLineColumn spelling;
   mutable bool ast; // AST matched?

   // Arguments
   // Each argument consists of some number of tokens
   std::vector<std::vector<clang::Token>> arguments;


   // ------------------------
   // Constructor
   // ------------------------

   MacroInvocation(
      const clang::Token &token,
      const clang::SourceRange &range,
      const clang::SourceManager &sman,
      const std::string &_name
   ) :
      name(_name), ast(false)
   {
      const clang::SourceLocation tloc = token.getLocation();
      getFileLineColumn(&sman, tloc, location);
      getFileLineColumn(&sman, sman.getSpellingLoc(tloc), spelling);
   }


   // ------------------------
   // Functions
   // ------------------------

   // Number of arguments to the macro
   std::size_t size() const
   {
      return arguments.size();
   }

   // Location of argument [a], token [t]
   clang::SourceLocation loc(
      const std::size_t a,
      const std::size_t t
   ) const {
      flecstan_assert(a < arguments.size());
      flecstan_assert(t < arguments[a].size());
      return arguments[a][t].getLocation();
   }

   // String representation of argument [a], token [t]
   std::string str(
      const clang::Sema &sema,
      const std::size_t a,
      const std::size_t t
   ) const {
      flecstan_assert(a < arguments.size());
      flecstan_assert(t < arguments[a].size());
      return TokenName(arguments[a][t],sema);
   }

   // String representation of argument [a], with all tokens put together,
   // separated by spaces.
   // 2018-02-08: For now let's change this, and NOT separate by spaces.
   std::string str(
      const clang::Sema &sema,
      const std::size_t a
   ) const {
      flecstan_assert(a < arguments.size());
      std::string s;
      for (std::size_t t = 0;  t < arguments[a].size();  ++t)
         s += /* (t ? " " : "") + */ str(sema,a,t);
      return s;
   }

   // flc
   // print file, line, column
   std::string flc() const
   {
      return print_flc("file ", ", line ", ", column ", location, spelling);
   }
};

} // namespace flecstan



// -----------------------------------------------------------------------------
// InvocationInfo
// -----------------------------------------------------------------------------

namespace flecstan {

class InvocationInfo {
public:
   // data
   FileLineColumn location;
   FileLineColumn spelling;

   // ctor: Sema, MacroInvocation
   InvocationInfo(
      const clang::Sema  &sema,
      const MacroInvocation &mi
   ) :
      location(mi.location),
      spelling(mi.spelling)
   {
      std::ostringstream oss;
      oss << "Name: " << mi.name
          << "\nArgs:";
      for (std::size_t arg = 0;  arg < mi.size();  ++arg)
         oss << (arg ? ", " : " ") << mi.str(sema,arg);

      report(
        "Macro",
         oss.str() +
         print_flc("\nFile: ", "\nLine: ", "\nColumn: ", location, spelling)
      );
   }
};

} // namespace flecstan



namespace llvm {
namespace yaml {

// MappingTraits<InvocationInfo>
template<>
class MappingTraits<flecstan::InvocationInfo> {
public:
   static void mapping(IO &io, flecstan::InvocationInfo &c)
   {
      io.mapRequired("location", c.location);
      io.mapRequired("spelling", c.spelling);
   }
};

} // namespace yaml
} // namespace llvm



// -----------------------------------------------------------------------------
// InvokedMatched
// -----------------------------------------------------------------------------

namespace flecstan {

template<class T>
class InvokedMatched {
public:
   std::vector<InvocationInfo> invoked;
   std::vector<T> matched;
};

} // namespace flecstan



namespace llvm {
namespace yaml {

template<class T>
class MappingTraits<flecstan::InvokedMatched<T>> {
public:
   static void mapping(IO &io, flecstan::InvokedMatched<T> &c)
   {
      io.mapRequired("invoked", c.invoked);
      io.mapRequired("matched", c.matched);
   }
};

} // namespace yaml
} // namespace llvm

#endif
