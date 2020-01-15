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

#include "flecstan-analysis.h"
#include "flecstan-arg.h"
#include "flecstan-diagnostic.h"
#include "flecstan-visitor.h"
#include "clang/Tooling/JSONCompilationDatabase.h"

// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

namespace flecstan {

// Consider doing this less stupidly
static flecstan::CmdArgs com;
static flecstan::Yaml yaml;

// stripws
std::string
stripws(const std::string & in) {
  // begin, end
  int b = 0;
  int e = in.size();

  while(b < e && isspace(in[b]))
    b++; // remove beginning spaces
  while(e > b && isspace(in[e - 1]))
    e--; // remove ending spaces

  return in.substr(b, e - b);
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// Consumer
// -----------------------------------------------------------------------------

namespace flecstan {

class Consumer : public clang::ASTConsumer
{
  clang::CompilerInstance & ci;
  Preprocessor & prep;
  exit_status_t & status;

public:
  Consumer(clang::CompilerInstance & ref, Preprocessor & p, exit_status_t & s)
    : ci(ref), prep(p), status(s) {
    debug("ctor: Consumer");
    flecstan_debug(ci.hasSema()); // false
    flecstan_debug((void *)&ci);
  }

  ~Consumer() {
    debug("dtor: Consumer");
  }

  // HandleTranslationUnit
  // override w.r.t. ASTConsumer
  void HandleTranslationUnit(clang::ASTContext & astcontext) override {
    debug("Consumer::HandleTranslationUnit() {");
    flecstan_debug((void *)&ci);
    flecstan_debug(ci.hasSema()); // true
    debug("}");

    if(emit_visit)
      report("", "Visiting the C++ abstract syntax tree...", false);

    if(!ci.hasSema()) {
      status = std::max(status, error("Could not get the Semantic Analyzer "
                                      "for this Compiler Instance."));
      return;
    }

    Visitor visitor(ci.getSema(), prep, yaml);
    visitor.TraverseDecl(astcontext.getTranslationUnitDecl());
    prep.map2yaml(); // see remarks in class Preprocessor's map2yaml()
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// Action
// -----------------------------------------------------------------------------

namespace flecstan {

class Action : public clang::ASTFrontendAction
{
  exit_status_t & status;

public:
  const std::string outer_name;

  Action(exit_status_t & s, const std::string & n) : status(s), outer_name(n) {
    debug("ctor: Action");
  }
  ~Action() {
    debug("dtor: Action");
  }

  // CreateASTConsumer
  // override w.r.t. ASTFrontendAction
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance & ci,
    llvm::StringRef file) override {
    debug("Action::CreateASTConsumer() {");
    flecstan_debug(ci.hasSema()); // false
    flecstan_debug(ci.hasPreprocessor()); // true
    flecstan_debug(file.str());
    debug("}");

    if(outer_name != "")
      filename(outer_name, file.str());

    if(emit_scan)
      report("", "Scanning for FleCSI macros...", false);

    if(!ci.hasPreprocessor()) {
      status = std::max(status, error("Could not get the Preprocessor "
                                      "for this Compiler Instance."));
      return nullptr;
    }

    // Diagnostics
    // See https://stackoverflow.com/questions/31542178/
    ci.getDiagnostics().setClient(new Diagnostic(status));

    // Preprocessor
    Preprocessor * const p = new Preprocessor(file.str(), ci, yaml);
    ci.getPreprocessor().addPPCallbacks(std::unique_ptr<Preprocessor>(p));

    // AST Consumer
    return std::unique_ptr<clang::ASTConsumer>(new Consumer(ci, *p, status));
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------------

namespace flecstan {

class Factory : public clang::tooling::FrontendActionFactory
{
  exit_status_t & status;

public:
  std::string outer_name;

  Factory(exit_status_t & s) : status(s), outer_name("") {
    debug("ctor: Factory");
  }
  ~Factory() {
    debug("dtor: Factory");
  }

  // create
  // override w.r.t. FrontendActionFactory
  Action * create() override {
    debug("Factory::create()");
    return new Action(status, outer_name);
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// IndividualCompileCommands
// -----------------------------------------------------------------------------

namespace flecstan {

class IndividualCompileCommands : public clang::tooling::CompilationDatabase
{
  const clang::tooling::CompileCommand cc;

public:
  // constructor: one compile command
  IndividualCompileCommands(const clang::tooling::CompileCommand & _cc)
    : cc(_cc) {
    debug("ctor: IndividualCompileCommands");
  }

  // destructor
  ~IndividualCompileCommands() {
    debug("dtor: IndividualCompileCommands");
  }

  // getCompileCommands
  // override w.r.t. CompilationDatabase
  std::vector<clang::tooling::CompileCommand> getCompileCommands(
    const llvm::StringRef) const override {
    debug("IndividualCompileCommands::getCompileCommands()");
    return std::vector<clang::tooling::CompileCommand>(1, cc);
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// Helper functions
//    try_json
//    try_make
//    try_cc
// -----------------------------------------------------------------------------

namespace flecstan {

// try_json
std::unique_ptr<clang::tooling::CompilationDatabase>
try_json(const std::pair<std::string, bool> & pair) {
  debug("try_json()");
  const std::string name = pair.first;
  filename(name, "");

  std::string ErrorMessage;
  std::unique_ptr<clang::tooling::CompilationDatabase> ptr;

  if(name == "-") {
    // looks like we'll need to read standard input into a buffer,
    // then send the buffer on to clang...
    // static, because it goes to an llvm::StringRef later
    static std::string buf = static_cast<const std::stringstream &>(
      std::stringstream() << std::cin.rdbuf())
                               .str();
    ptr = clang::tooling::JSONCompilationDatabase::loadFromBuffer(
      buf, // <== why buf needed to be static
      ErrorMessage, clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if(!ptr)
      error("Clang's CompilationDatabase::loadFromBuffer() "
            "reports an error:\n" +
            ErrorMessage);
  }
  else {
    // read from an actual file...
    ptr = clang::tooling::JSONCompilationDatabase::loadFromFile(
      llvm::StringRef(pair.first), ErrorMessage,
      clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if(!ptr)
      error("Clang's CompilationDatabase::loadFromFile() "
            "reports an error:\n" +
            ErrorMessage);
  }

  return ptr;
}

// try_make
std::unique_ptr<clang::tooling::CompilationDatabase>
try_make(const std::pair<std::string, bool> & pair) {
  debug("try_make()");
  const std::string name = pair.first;
  filename(name, "");

  std::ifstream ifs;
  if(name != "-")
    ifs.open(name.c_str());
  std::istream & in = name != "-" ? ifs : std::cin;

  const bool found = bool(in);
  if(!found) {
    error("Could not find make-commands file " + quote(name) + ".");
    return nullptr;
  }

  // static, because it goes to an llvm::StringRef later
  static std::string dbstr;

  // json begin
  // json, because we're going to scan for compilation commands and build
  // a temporary .json file to represent those commands to clang's API.
  dbstr = "[\n";

  // Scan for lines like this:
  //    cd /foo/bar/etc && /a/b/etc/clang++ -some -flags -c x/y/etc/file.cc
  //    ^               ^           ^                      ^
  //    pos_cd          pos_and     pos_clang              pos_file
  // or this:
  //    /a/b/etc/clang++ -some -flags -c x/y/etc/file.cc
  //             ^                      ^
  //             pos_clang              pos_file
  // and ensure that there's a " -c ", and that the ending looks like a C++
  // file. Otherwise, we might get false positives.
  //
  // fixme This function is simple-minded right now. It won't handle, for
  // instance, spaces in the file name. And, some of the parsing may reject
  // valid constructs or accept invalid ones. It should work correctly with
  // what I've actually *seen* from make VERBOSE=1 output, and my goal in
  // the immediate term is to get something that's functional. -Martin

  bool first = true; // for handling commas between {...}s
  for(std::string line; std::getline(in, line);) {
    if(!(endsin(line, ".cc") || endsin(line, ".cpp") || endsin(line, ".cxx") ||
         endsin(line, ".C")))
      continue;

    // Note that "cd " and "[clan]g++ ", below, intentionally end with spaces.
    // fixme Needs more work in order to be really robust. Think, for example,
    // general white space rather than spaces.
    const std::size_t pos_cd = line.find("cd ");
    const std::size_t pos_and = line.find("&&");
    const std::size_t pos_clang = line.find("clang++ ");
    const std::size_t pos_g = line.find("g++ ");
    const std::size_t pos_file = line.rfind(" ");
    const std::size_t pos_space = line.find(" ");

    const std::size_t pos_comp =
      pos_clang != std::string::npos ? pos_clang : pos_g;

    // Form #1
    if(pos_cd == 0 && pos_and != std::string::npos &&
       pos_comp != std::string::npos && pos_file != std::string::npos) {
      dbstr += (first ? "   {" : ",\n   {");
      const std::string dir =
                          stripws(line.substr(3, pos_and - 3)), // start,length
        com = stripws(line.substr(pos_and + 2)), // to end
        file = stripws(line.substr(pos_file + 1)); // to end
      dbstr += "\n      \"directory\" : \"" + dir + "\",";
      dbstr += "\n      \"command\"   : \"" + com + "\",";
      dbstr += "\n      \"file\"      : \"" + file + "\"\n   }";
      first = false;
      continue;
    }

    // Form #2
    // /a/b/etc/clang++ -some -flags -c x/y/etc/file.cc
    if(pos_comp != std::string::npos && pos_file != std::string::npos &&
       pos_space != std::string::npos && pos_comp < pos_space) {
      dbstr += (first ? "   {" : ",\n   {");
      const std::string dir = ".", com = line,
                        file = stripws(line.substr(pos_file + 1)); // to end
      dbstr += "\n      \"directory\" : \"" + dir + "\",";
      dbstr += "\n      \"command\"   : \"" + com + "\",";
      dbstr += "\n      \"file\"      : \"" + file + "\"\n   }";
      first = false;
      continue;
    }
  }

  // json end
  dbstr += "\n]";

  std::string ErrorMessage;
  std::unique_ptr<clang::tooling::CompilationDatabase> ptr =
    clang::tooling::JSONCompilationDatabase::loadFromBuffer(
      dbstr, // <== why dbstr needed to be static
      ErrorMessage, clang::tooling::JSONCommandLineSyntax::AutoDetect);

  if(!ptr)
    error("Clang's CompilationDatabase::loadFromBuffer() "
          "reports an error:\n" +
          ErrorMessage);
  return ptr;
}

// try_cc
bool
try_cc(const clang::tooling::CompileCommand & cc) {
  debug("try_cc()");

  // directory, file
  const std::string & dir = cc.Directory;
  const std::string & file = cc.Filename;

  // full file name
  const std::string full = fullfile(dir, file);

  // print
  filename("", full);

  // try to open
  std::ifstream ifs(full.c_str());
  const bool found = bool(ifs);

  if(found)
    return true;

  error("Could not find C++ file " + quote(full) + ".");
  return false;
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// compilation
// -----------------------------------------------------------------------------

namespace flecstan {

exit_status_t
compilation() {
  debug("compilation()");

  // Factory
  exit_status_t status = exit_clean; // status for overall analysis
  Factory factory(status);

  // Number of JSON/make-verbose/C++ files; and total
  const std::size_t jsize = com.jsonfile.size();
  const std::size_t msize = com.makeinfo.size();
  const std::size_t csize = com.commands.size(), size = jsize + msize + csize;
  if(com.type.size() != size)
    return interr("Vector size mismatch. Contact us.");

  for(std::size_t j = 0, m = 0, c = 0, n = 0; n < size && status != exit_fatal;
      ++n) {
    exit_status_t stat = exit_clean; // status for this iteration
    int run = 0; // will get return value from ClangTool::run()...

    // JSON file
    if(com.type[n] == CmdArgs::file_t::json_t) {
      std::unique_ptr<clang::tooling::CompilationDatabase> ptr =
        try_json(com.jsonfile[j]);
      if(ptr) {
        clang::tooling::ClangTool ctool(*ptr, ptr->getAllFiles());
        factory.outer_name = com.jsonfile[j].first;
        run = ctool.run(&factory);
      }
      else
        stat = exit_error;
      j++;
    }

    // Make-verbose file
    if(com.type[n] == CmdArgs::file_t::make_t) {
      std::unique_ptr<clang::tooling::CompilationDatabase> ptr =
        try_make(com.makeinfo[m]);
      if(ptr) {
        clang::tooling::ClangTool ctool(*ptr, ptr->getAllFiles());
        factory.outer_name = com.makeinfo[m].first;
        run = ctool.run(&factory);
      }
      else
        stat = exit_error;
      m++;
    }

    // C++ file
    if(com.type[n] == CmdArgs::file_t::cpp_t) {
      clang::tooling::CompileCommand & cc = com.commands[c].first;
      IndividualCompileCommands compiles(cc); // just 1 command
      std::vector<std::string> files(1, cc.Filename); // just 1 file
      clang::tooling::ClangTool ctool(compiles, files);
      factory.outer_name = "";
      if(try_cc(cc))
        run = ctool.run(&factory);
      else
        stat = exit_error;
      c++;
    }

    // Per Clang Tooling documentation, ClangTool::run() returns:
    //   0 on success;
    //   1 if any error occurred;
    //   2 if there is no error but some files are
    //     skipped due to missing compile commands.
    if(run != 0) {
      std::ostringstream oss;
      oss << "Clang's ClangTool::run() "
             "reports a nonzero return value: "
          << run << ".";
      if(run == 2)
        oss << "\nMeaning: \"no error, but some files were skipped"
               "\ndue to missing compile commands\".";
      stat = error(oss);
    }

    // bookkeeping
    flecstan_debug(stat);
    if((status = std::max(status, stat)) == exit_fatal)
      return status;
  }

  return status;
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// yamlout
// -----------------------------------------------------------------------------

namespace flecstan {

exit_status_t
yamlout(bool & written, bool & already) {
  debug("yamlout()");

  // No YAML output file was specified. That's fine, we won't create one.
  if(!com.yout.set())
    return exit_clean;

  // A YAML output file was specified. Let's be intelligent, so that we're
  // unlikely to clobber an existing non-YAML file. Specifically, we'll ensure
  // that the file name ends with ".yaml" - a reasonable action, and one that
  // disallows someone from, say, accidentally overwriting a .json, .txt,
  // or .cc file.
  // And, as elsewhere, we'll emit appropriate diagnostics.

  // Ensure .yaml extension
  // We may no longer need this; it's done during command-line processing.
  // Won't do any harm, though.
  const std::string file = com.yout.value();
  if(!endsin(file, ".yaml"))
    com.yout = file + ".yaml";

  // There already?
  std::ifstream ifs(com.yout.value().c_str());
  already = false;
  if(ifs) {
    note("Existing YAML output file " + quote(com.yout.value()) +
         " will be replaced.");
    already = true;
  }
  ifs.close(); // so no weirdnesses with opening for output below

  // Print our internal YAML structure to a string
  std::string str;
  llvm::raw_string_ostream raw(str);
  llvm::yaml::Output yamlout(raw);
  yamlout << yaml;
  raw.flush(); // <-- necessary :-/

  // Open file
  std::ofstream ofs(com.yout.value().c_str());
  if(ofs) {
    // Print string to file
    ofs << raw.str();
    if(ofs) {
      written = true;
      return exit_clean;
    }
    else
      error("Problem writing YAML output to " + quote(com.yout.value()) + ".");
  }
  else
    error("Could not open " + quote(com.yout.value()) + " for YAML output.");

  return exit_error;
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// summary
// -----------------------------------------------------------------------------

namespace flecstan {

exit_status_t
summary() {
  debug("summary()");
  exit_status_t status = exit_clean; // for now

  // Re: Task registration/execution
  if(summary_task_reg_dup != "")
    report("Task registration duplicates", summary_task_reg_dup, true);
  if(summary_task_reg_without_exe != "")
    report("Task registrations without executions",
      summary_task_reg_without_exe, true);
  if(summary_task_exe_without_reg != "")
    report("Task executions without registrations",
      summary_task_exe_without_reg, true);

  // Re: Function interface
  if(summary_function_reg_dup != "")
    report("Function registration duplicates", summary_function_reg_dup, true);
  if(summary_function_reg_without_hand != "")
    report("Function registrations without handle retrievals",
      summary_function_reg_without_hand, true);
  if(summary_function_hand_without_reg != "")
    report("Function handle retrievals without registrations",
      summary_function_hand_without_reg, true);

  // Re: YAML
  bool written = false;
  bool already = false;
  status = std::max(status, yamlout(written, already));
  if(status == exit_fatal)
    return exit_fatal;
  if(written)
    status = note("YAML output file " + quote(com.yout.value()) +
                  (already ? " replaced." : " created."));

  return status;
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// analyze
// -----------------------------------------------------------------------------

namespace flecstan {

exit_status_t
analyze(const int argc, const char * const * const argv) {
  exit_status_t status = exit_clean; // for now

  std::vector<std::string> exes;
  std::vector<std::string> args;

  for(std::size_t a = 1; a < std::size_t(argc);) {
    const std::string str = argv[a];
    if(str == "-analyze" || str == "--analyze") {

      // ------------------------
      // option is -analyze;
      // find *its* arguments
      // ------------------------

      std::size_t i = a;
      while(++i < std::size_t(argc)) {
        const std::string opt = argv[i];
        if(option_toggle(opt) || option_dir(opt) || option_json(opt) ||
           option_make(opt) || option_cc(opt) || option_yaml(opt) ||
           option_clang(opt) || option_flags(opt) || option_yout(opt) ||
           endsin_json(opt) || endsin_make(opt) || endsin_cc(opt) ||
           endsin_yaml(opt)) {
          break; // from while
        }

        // opt is an argument to -analyze
        exes.push_back(opt);
      } // while

      if(i == a + 1) {
        // no arguments to -[-]analyze :-(
        status = std::max(status, error("The " + str +
                                        " option "
                                        "(executable target for \"make\") "
                                        "expects one or more arguments.\n"
                                        "Example: " +
                                        str + " hydro_2d hydro_3d"));
      } // if
      a = i;
    }
    else {

      // ------------------------
      // option is not -analyze;
      // store for future use
      // ------------------------

      args.push_back(str);
      a++;
    } // else

  } // for

  if(status != exit_clean)
    return status;

  // ------------------------
  // Create and spawn other
  // flecstan commands
  // ------------------------

  // check
  assert(exes.size() != 0);

  // create
  std::vector<std::string> commands;
  std::string fyi;
  const std::string invocation = argv[0];

  for(auto exe : exes) {
    std::string cmd = "make --dry-run VERBOSE=1 " + exe + " | " + invocation;
    for(auto arg : args)
      cmd += " " + arg;
    cmd += " -make -";
    commands.push_back(cmd);
    fyi += cmd + "\n";
  }

  // FYI for user
  report("Creating individual analysis commands", fyi, true);

  // run!
  for(auto com : commands) {
    report(
      "Running \"make clean\" plus individual analysis command", com, true);
    system("make clean");
    system(com.c_str());
  }

  // done
  return status;

} // analyze

} // namespace flecstan

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int
main(const int argc, const char * const * const argv) {
  // ------------------------
  // Has --analyze? Then code
  // takes a different path
  // ------------------------

  for(std::size_t a = 1; a < std::size_t(argc); ++a) {
    const std::string str = argv[a];
    if(str == "-analyze" || str == "--analyze")
      return flecstan::analyze(argc, argv);
  }

  // ------------------------
  // Beginning version/help
  // ------------------------

  std::size_t arg = 0;
  while(arg + 1 < std::size_t(argc)) {
    if(flecstan::option_version(argv[arg + 1]))
      flecstan::print_version();
    else if(flecstan::option_help(argv[arg + 1]))
      flecstan::print_help(true); // with coloring
    else if(flecstan::option_h(argv[arg + 1]))
      flecstan::print_help(false); // without coloring
    else
      break;
    // no arguments other than version and/or help?
    if(++arg + 1 == std::size_t(argc))
      return exit_clean;
  }

  // ------------------------
  // Initialize
  // ------------------------

  using namespace flecstan;
  exit_status_t status = exit_clean; // overall status

  // arguments: initial processing
  if((status = std::max(status, initial(arg, argc, argv, com))) == exit_fatal)
    return exit_fatal;

  // ------------------------
  // Sections
  // ------------------------

  // Command
  title("Command", emit_section_command);
  if((status = std::max(status, arguments(arg, argc, argv, com))) == exit_fatal)
    return exit_fatal;

  // Compilation
  title("Compilation", emit_section_compilation);
  if((status = std::max(status, compilation())) == exit_fatal)
    return exit_fatal;

  // Analysis
  title("Analysis", emit_section_analysis);
  if((status = std::max(status, analysis(yaml))) == exit_fatal)
    return exit_fatal;

  // Summary
  title("Summary", emit_section_summary);
  if((status = std::max(status, summary())) == exit_fatal)
    return exit_fatal;

  // ------------------------
  // Finish
  // ------------------------

  status == exit_clean
    ? note("FleCSI static analysis completed.")
    : warning("FleCSI static analysis may be incomplete; errors occurred.");
  return status;
}
