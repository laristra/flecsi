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

#include "flecstan-misc.h"

namespace flecstan {

// -----------------------------------------------------------------------------
// getFileLineColumn: helper
//
// Remark: This code largely mirrors the code in clang::SourceLocation::print(),
// which is called by clang::SourceLocation::dump(). Earlier, I (Martin) did
// file/line/column extraction in a different way - one I'd seen other people
// doing. For whatever reason, the other methodology was less robust than this
// one. Specifically, it reliably obtained file/line/column in most contexts,
// but didn't do so in the particular case of extracting those values from clang
// diagnostics *when* the diagnostics were associated with macro-inserted code.
// Outside of the diagnostics context (in other places where we want file name,
// etc.), or for diagnostics that weren't triggered by macro-inserted code, the
// old scheme worked. :-/
// -----------------------------------------------------------------------------

void
getFileLineColumn(
  // input
  const clang::SourceManager * const sman,
  const clang::SourceLocation & loc,

  // output
  FileLineColumn & flc) {
  if(sman != nullptr && loc.isValid()) {
    if(loc.isFileID()) {
      clang::PresumedLoc pre = sman->getPresumedLoc(loc);
      if(pre.isValid()) {
        flc.file = pre.getFilename();
        flc.line = std::to_string(pre.getLine());
        flc.column = std::to_string(pre.getColumn());
        return;
      }
    }
    else {
      // Remark: I hope that there isn't any way this can trigger infinite
      // recursion. That it doesn't do so is not entirely obvious, but I'll
      // assume for now that it's OK, because, as stated earlier, our code
      // here essentially mirrors the code in clang::SourceLocation::print().
      // Note that the case in which the recursion occurs can be described
      // as, "the inputs (source manager and location) are entirely valid,
      // but the location isn't a file [id]." Presumably, in this situation,
      // getExpansionLoc() below gets us into a different state of affairs.
      getFileLineColumn(sman, sman->getExpansionLoc(loc), flc);
      return;
    }
  }

  flc.file = "unknown";
  flc.line = "unknown";
  flc.column = "unknown";
}

// -----------------------------------------------------------------------------
// Remark: Our handling of colors, in some of the below functions, may *appear*
// to be more complicated than it needs to be. In particular, we'll explicitly
// set and unset color markup on a per-line, not per-block-of-lines, basis.
// While this clutters the output with more color markup than is necessary,
// we're doing it intentionally: so that colors are preserved if, for example,
// you send the code's output through something that processes individual lines.
// Imagine, for example, using the Unix "grep" command to display lines with the
// word "foo" in them:
//
//    flecstan ... | grep foo
//
// Well, grep isn't the best example, because, depending on how it's configured,
// it may mess with color markup or use its own. You may, however, be able to
// switch that off, for example with:
//
//    flecstan ... | grep --color=never foo
//
// So, we believe that our line-by-line color encoding gives more flexibility in
// terms of what you can do with the output.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// debug
// -----------------------------------------------------------------------------

void
debug(const std::string & text) {
  markup.begin();

  if(!emit_debug)
    return;

  // color
  const std::string & color = emit_color ? color::debug : "";
  const std::string & reset = emit_color ? color::reset : "";

  // content
  std::cout << color << "Debug: ";
  const std::size_t size = text.size();
  for(std::size_t i = 0; i < size; ++i) {
    const bool internal_newline = text[i] == '\n' && i < size - 1;
    if(internal_newline)
      std::cout << reset;
    std::cout << text[i];
    if(internal_newline)
      std::cout << color << "Debug: ";
  }
  std::cout << reset;

  // finish
  if(text != "" && text.back() != '\n')
    std::cout << "\n";
  std::cout << std::flush;
}

// -----------------------------------------------------------------------------
// title
// -----------------------------------------------------------------------------

bool first_print = true;

void
title(const std::string & text,
  const bool emit_section,
  const int length,
  const std::string & text_color) {
  markup.begin();

  section_on = emit_section;
  if(!section_on)
    return;

  static bool first = true;
  if(first)
    first = false;
  else if(emit_formfeed)
    std::cout << "\f";

  const std::string & color = emit_color ? color::title : "";
  const std::string & textcolor = emit_color ? text_color : "";
  const std::string & reset = emit_color ? color::reset : "";
  const std::string line(length, '-');

  if(emit_title) {
    std::cout << (first_print ? "" : "\n");
    first_print = false;

    if(short_form)
      std::cout << textcolor << text << "..." << reset;
    else
      std::cout << color << line << reset << "\n"
                << textcolor << text << reset << "\n"
                << color << line << reset;

    std::cout << std::endl;
  }
}

// -----------------------------------------------------------------------------
// diagnostic
// Written compactly more so than efficiently. For I/O, I'm not going to care.
// -----------------------------------------------------------------------------

void
diagnostic(const std::string & label,
  const std::string & text,
  std::string maincolor,
  std::string textcolor,
  const bool keep_long_form, // override short-form flag?
  const bool colon) {
  markup.begin();

  // spacing
  if(!short_form && !first_print)
    std::cout << "\n";
  first_print = false;

  // color
  if(!emit_color)
    textcolor = maincolor = "";
  else if(textcolor == "")
    textcolor = maincolor;
  const std::string & reset = emit_color ? color::reset : "";

  // content
  if(label == "" || (short_form && !keep_long_form))
    std::cout << maincolor << text << reset;
  else {
    std::cout << maincolor << label << (colon ? ":" : "") << reset << "\n   ";
    std::cout << textcolor;

    const std::size_t size = text.size();
    for(std::size_t i = 0; i < size; ++i) {
      const bool internal_newline = text[i] == '\n' && i < size - 1;
      if(internal_newline)
        std::cout << reset;
      std::cout << text[i];
      if(internal_newline)
        std::cout << "   " << textcolor;
    }

    std::cout << reset;
  }

  // finish
  if(text != "" && text.back() != '\n')
    std::cout << "\n";
  std::cout << std::flush;
}

// -----------------------------------------------------------------------------
// print_help
// -----------------------------------------------------------------------------

// helper: subtitle
inline void
subtitle(const std::string & label) {
  title(label, true, 32, color::bold::white);
}

// helper: subsubtitle
inline void
subsubtitle(const std::string & label) {
  diagnostic("", label, color::file);
}

// helper: option
inline void
option(const std::string & label, const std::string & description) {
  // false at the end ==> no colon after printing the option name
  report("-[-]" + label, description, true, false);
}

// print_help
void
print_help(const bool colorize) {
  static bool first = true;
  if(!first)
    return;
  first = false;

  // save current color scheme
  const bool backup_color = emit_color;

  // color scheme for help
  emit_color = colorize;

  // ------------------------
  // Help: big title
  // ------------------------

  title("Help", true, 80, color::bold::white);

  // ------------------------
  // Options: Informational
  // ------------------------

  subtitle("Informational");

  option("version", "Print version number.");

  option("help", "Print this help message, colorized.");

  option("h", "Print this help message, monochrome.");

  // ------------------------
  // Options: Input/Output
  // ------------------------

  subtitle("Input/Output");

  // Input
  subsubtitle("Input");

  option("analyze target ...",
    "Run the analyzer individually on one or more \"make\" targets.\n"
    "Invoke from the same directory as you would \"make\".\n"
    "Does a \"make clean\" before each analysis.\n"
    "Example: flecstan --analyze hydro_1d hydro_2d hydro_3d\n");

  option("make output-from-make-verbose ...",
    "Take, as input, the output produced by a make --dry-run VERBOSE=1.\n"
    "Use \"-\" for standard input.\n"
    "We suggest running \"make clean\" first, then invoking this command\n"
    "from the same directory. But consider using "
    "the --analyze option instead,\n"
    "as it effectively provides a simplified way "
    "of doing what this option does.\n"
    "Example: make --dry-run VERBOSE=1 hydro_2d | flecstan --make -\n");

  option("json file.json ...",
    "Read from a .json format \"compilation database\" of compile commands.\n"
    "Note: an argument ending with .json "
    "is automatically processed as if it\n"
    "had been preceded by --json; thus the \"--json\" option isn't "
    "typically\n"
    "needed explicitly.\n"
    "Example: flecstan --json compile_commands.json\n");

  option("yaml",
    "NOT YET IMPLEMENTED!\n"
    "Read from a .yaml file of the type flecstan can produce as output.\n"
    "Example: flecstan --yaml file.yaml\n");

  // Output
  subsubtitle("Output");

  option("yout", "Write a .yaml file with information about "
                 "the various FleCSI constructs\n"
                 "that this flecstan invocation found.\n");

  // ------------------------
  // Options: Appearance
  // ------------------------

  subtitle("Appearance");

  // Bulk
  subsubtitle("Overall");

  option("long", "Longer-form output for flecstan's reporting. "
                 "Doesn't meaningfully affect\n"
                 "content; just prints it in a bulkier but perhaps "
                 "more readable form.\n"
                 "This is the default.\n"
                 "Compare flecstan invocations with --long "
                 "and --short to see the difference.\n");

  option("short",
    "Shorter-form output for flecstan's reporting. "
    "Doesn't meaningfully affect\n"
    "content; just prints it in a shorter and more compact form.\n"
    "Compare flecstan invocations with --long "
    "and --short to see the difference.\n");

  // Visual
  subsubtitle("Visual");

  option("print",
    "Use \"print mode\" for flecstan's output.\n"
    "Turns text coloring off, and form feeds (between sections) on.\n"
    "Has the same effect as: --no-color --formfeed.\n"
    "Intent: create output that's suitable for simple text printing,\n"
    "with major sections on different pages.\n");

  option("[no-]color[s]",
    "Color the text output (--color), or don't (--no-color).\n"
    "Default: on.\n"
    "Text coloring is created by using ANSI color escape sequences,\n"
    "which should work on most terminals.\n");

  option("[no-]formfeed[s]",
    "Emit form-feeds [or don't] between major output sections.\n"
    "Probably useful only if you're printing output, "
    "or piping to more/less.\n");

  // Files
  subsubtitle("Files");

  option("file-long", "Print file names in long form.\n");

  option("file-medium", "Print file names in medium-length form.\n");

  option("file-short", "Print file names in short form.\n");

  option("file-full", "Print file names with path prepended.\n");

  option("file-strip", "Print file names with path stripped off.\n");

  // Markup
  subsubtitle("Markup");

  option("markup-ansi",
    "Create text coloring by using standard ANSI color escape sequences,\n"
    "which should work on most terminals.\n"
    "This is the default.\n");

  option("markup-html",
    "Emit text-coloring markup that may be suitable for an .html document.\n"
    "This is a \"know what you're doing\" option.\n");

  option("markup-rst",
    "Emit text-coloring markup that may be suitable for a .rst document.\n"
    "This is a \"know what you're doing\" option.\n");

  option("markup-tex",
    "Emit text-coloring markup that may be suitable for a .tex document.\n"
    "This is a \"know what you're doing\" option.\n");

  option("markup-tex-listing",
    "Emit text-coloring markup that may work in the context of a specific\n"
    "type of \\listing macro, found in some .tex documents, that allows \n"
    "input files to use backticks as the TeX escape character. "
    "This was used\n"
    "by flecstan's author to help make some of the presentation slides.\n"
    "This is a pow(\"know what you're doing\",2) option.\n");

  // ------------------------
  // Options: Content
  // ------------------------

  subtitle("Content");

  // Combos
  subsubtitle("Combos");

  option("quiet",
    "Run in quiet mode.\n"
    "Switches off most output, except for errors and a synopsis.\n");

  option("verbose",
    "Run in verbose mode.\n"
    "This is the default.\n"
    "Switches on all manner of output.\n"
    "Note that other options, e.g. -short, can compress the formatting.\n");

  // Sections
  subsubtitle("Sections");

  option("[no-]section-command",
    "Print [or don't] the Command section, and all of its contents.\n"
    "Errors will always be printed, regardless.\n"
    "This section is where flecstan prints informational "
    "chatter about how it\n"
    "is interpreting the command line. Advanced users may "
    "very well wish to\n"
    "switch off this section.\n");

  option("[no-]section-compilation",
    "Print [or don't] the Compilation section, and all of its contents.\n"
    "Errors will always be printed, regardless.\n"
    "This section is where flecstan prints information about "
    "all of the FleCSI\n"
    "macro invocations, and macro-produced code, that it finds. "
    "You may or may\n"
    "not be interested in this information.\n");

  option("[no-]section-analysis",
    "Print [or don't] the Analysis section, and all of its contents.\n"
    "Errors will always be printed, regardless.\n"
    "Most users will want this section to be included, "
    "as it's the place where\n"
    "flecstan prints information regarding its analysis of the code.\n");

  option("[no-]section-summary",
    "Print [or don't] the Summary section, and all of its contents.\n"
    "Errors will always be printed, regardless.\n");

  // General
  subsubtitle("General");

  option("[no-]title[s]", "Print [or don't] section titles.\n"
                          "Default: on.\n");

  option("[no-]file[s]", "Print [or don't] file names.\n"
                         "Default: on.\n");

  option("[no-]report[s]",
    "Print [or don't] \"report\" output.\n"
    "Default: on.\n"
    "In this context, \"report\" refers to certain "
    "informational printing such\n"
    "as FleCSI macros found, matching AST constructs, "
    "and a synopsis. Typically,\n"
    "these are flecstan's output elements that are printed "
    "in dark and light blue.\n");

  option("[no-]scan[ning]",
    "Print [or don't] the message about \"scanning for macros\".\n"
    "Default: on.\n"
    "This is a minor option that can be used to slightly "
    "reduce output bulk.\n");

  option("[no-]visit[ing]",
    "Print [or don't] the message about \"visiting the syntax tree\".\n"
    "Default: on.\n"
    "This is a minor option that can be used to slightly "
    "reduce output bulk.\n");

  option("[no-]macro[s]",
    "Print [or don't] an accounting of all the "
    "FleCSI macros flecstan finds.\n"
    "Default: on.\n"
    "Using this option can considerably reduce output bulk, "
    "if you aren't\n"
    "interested in this information.\n");

  option("[no-]link[s]",
    "Print [or don't] an accounting of all the macro-matching "
    "AST constructs.\n"
    "Default: on.\n"
    "Using this option can considerably reduce output bulk, if you aren't\n"
    "interested in this information.\n");

  option("[no-]column[s]",
    "Print [or don't] column numbers. (Line number are always printed.)\n"
    "Default: off.\n");

  // Diagnostics
  subsubtitle("Diagnostics");

  option("[no-]note[s]", "Print [or don't] informational notes.\n"
                         "These include passed-through clang Ignored, Note, "
                         "and Remark diagnostics.\n"
                         "Default: print them.\n");

  option("[no-]warning[s]",
    "Print [or don't] warnings.\n"
    "These include passed-through clang Warning diagnostics.\n"
    "Default: print them.\n");

  option("[no-]error[s]",
    "Print [or don't] errors.\n"
    "These include passed-through clang Error and Fatal diagnostics.\n"
    "Default: print them.\n"
    "Note: fatal errors are always printed, regardless of this setting.\n");

  // Auxiliary
  subsubtitle("Auxiliary");

  option("[no-]trace[s]",
    "Print [or don't] expansion traces from clang Warning, Error, and Fatal\n"
    "diagnostics.\n"
    "Default: on.\n");

  option("[no-]ccdetail[s]",
    "Print [or don't] certain additional details, "
    "mainly useful for debugging,\n"
    "regarding the compilation commands that flecstan "
    "creates when a user runs\n"
    "the analyzer in its \"Direct Compilation\" mode for advanced users.\n"
    "Default: off.\n");

  // Debugging
  subsubtitle("Debugging");

  option("debug", "Print [or don't] copious amounts of information "
                  "about flecstan's internal\n"
                  "behavior. You really don't want to use this option "
                  "unless you're a flecstan\n"
                  "developer.\n");

  // ------------------------
  // Options: Direct Compilation
  // ------------------------

  subtitle("Direct Compilation");

  option("dir[ectory]\n-[-]folder",
    "Stipulate a directory in which to look for upcoming "
    "input files of any\n"
    "kind (.json, .cc, ...).\n"
    "Example: flecstan --directory /my/files/foobar/ -json "
    "foo.json bar.json\n");

  option("clang[++]",
    "Stipulate a specific clang++ compiler.\n"
    "Note: flecstan is its own compiler, but this option can affect which\n"
    "standard C++ files are involved in #includes.\n"
    "Example: flecstan --clang++ /usr/local/bin/clang++ ...\n");

  option("flag[s]",
    "Stipulate clang++ flags to be used for direct compilation - not to be\n"
    "confused with flecstan flags as we're describing in this help message.\n"
    "Example: flecstan --flags -std=c++17 -Ifoo/bar/ -O3 -Wall ...\n");

  option("cc\n-[-]cpp\n-[-]cxx\n-[-]c++\n-[-]C",
    "Stipulate C++ source files to compile.\n"
    "Note: files ending with .cc, .cpp, .cxx, or .C are "
    "automatically processed\n"
    "as if they are C++ source files, and thus need not be "
    "preceded with any\n"
    "of these options. We suggest using on of these only if "
    "you have C++ source\n"
    "files with extensions other than these.\n"
    "Example: flecstan --cxx foo.cc bar.cpp baz.cxx boo.C\n");

  // restore color scheme
  emit_color = backup_color;
}

} // namespace flecstan
