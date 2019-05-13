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

    report("", "Visiting the C++ abstract syntax tree...");
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
  const std::string json_name;

  Action(exit_status_t & s, const std::string & jname)
    : status(s), json_name(jname) {
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

    if(json_name != "")
      filename(json_name + ": " + file.str());

    report("", "Scanning for FleCSI macros...");
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
  std::string json_name;

  Factory(exit_status_t & s) : status(s), json_name("") {
    debug("ctor: Factory");
  }
  ~Factory() {
    debug("dtor: Factory");
  }

  // create
  // override w.r.t. FrontendActionFactory
  Action * create() override {
    debug("Factory::create()");
    return new Action(status, json_name);
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// Database
// -----------------------------------------------------------------------------

namespace flecstan {

class Database : public clang::tooling::CompilationDatabase
{
  const clang::tooling::CompileCommand cc;

public:
  Database(const clang::tooling::CompileCommand & _cc) : cc(_cc) {
    debug("ctor: Database");
  }
  ~Database() {
    debug("dtor: Database");
  }

  // getCompileCommands
  // override w.r.t. CompilationDatabase
  std::vector<clang::tooling::CompileCommand> getCompileCommands(
    const llvm::StringRef file) const override {
    debug("Database::getCompileCommands()");
    return std::vector<clang::tooling::CompileCommand>(1, cc);
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// read_json
// -----------------------------------------------------------------------------

namespace flecstan {

std::unique_ptr<clang::tooling::CompilationDatabase>
read_json(const std::pair<std::string, bool> & pair) {
  debug("read_json()");
  filename(pair.first);

  std::string ErrorMessage;
  std::unique_ptr<clang::tooling::CompilationDatabase> ptr =
    clang::tooling::JSONCompilationDatabase::loadFromFile(
      llvm::StringRef(pair.first), ErrorMessage,
      clang::tooling::JSONCommandLineSyntax::AutoDetect);

  if(!ptr)
    error("Clang's CompilationDatabase::loadFromFile() "
          "reports an error:\n" +
          ErrorMessage);
  return ptr;
}

} // namespace flecstan

// -----------------------------------------------------------------------------
// try_file: helper
// -----------------------------------------------------------------------------

namespace flecstan {

bool
try_file(const clang::tooling::CompileCommand & cc) {
  // directory, file
  const std::string & dir = cc.Directory;
  const std::string & file = cc.Filename;

  // full file name
  const std::string full = fullfile(dir, file);

  // print
  filename(full);

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

  // Number of JSON/C++ files; and total
  const std::size_t jsize = com.database.size();
  const std::size_t csize = com.commands.size(), size = jsize + csize;
  if(com.isdb.size() != size)
    return interr("Vector size mismatch. Contact us.");

  for(std::size_t j = 0, c = 0, n = 0; n < size && status != exit_fatal; ++n) {
    exit_status_t stat = exit_clean; // status for this JSON or C++
    int run = 0; // will get return value from ClangTool::run()...

    if(com.isdb[n]) {
      // JSON database file
      std::unique_ptr<clang::tooling::CompilationDatabase> ptr =
        read_json(com.database[j]);
      if(ptr) {
        clang::tooling::ClangTool ctool(*ptr, ptr->getAllFiles());
        factory.json_name = com.database[j].first;
        run = ctool.run(&factory);
      }
      else
        stat = exit_error;
      j++;
    }
    else {
      // C++ file
      clang::tooling::CompileCommand & cc = com.commands[c].first;
      Database database(cc);
      std::vector<std::string> files(1, cc.Filename);
      clang::tooling::ClangTool ctool(database, files);
      factory.json_name = "";
      if(try_file(cc))
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
  // disallows someone from, say, accidentally overwriting a .cc or .json file!
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
    report("Task registration duplicates", summary_task_reg_dup);
  if(summary_task_reg_without_exe != "")
    report(
      "Task registrations without executions", summary_task_reg_without_exe);
  if(summary_task_exe_without_reg != "")
    report(
      "Task executions without registrations", summary_task_exe_without_reg);

  // Re: Function interface
  if(summary_function_reg_dup != "")
    report("Function registration duplicates", summary_function_reg_dup);
  if(summary_function_reg_without_hand != "")
    report("Function registrations without handle retrievals",
      summary_function_reg_without_hand);
  if(summary_function_hand_without_reg != "")
    report("Function handle retrievals without registrations",
      summary_function_hand_without_reg);

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
// main
// -----------------------------------------------------------------------------

int
main(const int argc, const char * const * const argv) {
  // ------------------------
  // Beginning version/help
  // ------------------------

  std::size_t arg = 0;
  while(arg + 1 < std::size_t(argc)) {
    if(flecstan::option_version(argv[arg + 1])) {
      static bool already = false;
      if(!already)
        flecstan::print_version();
      already = true;
    }
    else if(flecstan::option_help(argv[arg + 1])) {
      static bool already = false;
      if(!already)
        flecstan::print_help();
      already = true;
    }
    else
      break;
    // no arguments other than version or help?
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
  heading("Command", emit_section_command);
  if((status = std::max(status, arguments(arg, argc, argv, com))) == exit_fatal)
    return exit_fatal;

  // Compilation
  heading("Compilation", emit_section_compile);
  if((status = std::max(status, compilation())) == exit_fatal)
    return exit_fatal;

  // Analysis
  heading("Analysis", emit_section_analysis);
  if((status = std::max(status, analysis(yaml))) == exit_fatal)
    return exit_fatal;

  // Summary
  heading("Summary", emit_section_summary);
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
