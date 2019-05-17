
// Clang includes
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// C++ includes
#include <iostream>

// -----------------------------------------------------------------------------
// Description
// If you understand this description, and our code, we believe you'll find
// it to be straightforward to build your own application, using the clang
// tooling library, for visiting the nodes in a C++ abstract syntax tree.
// -----------------------------------------------------------------------------

/*
------------------------
Introduction
------------------------

This code presents a sort of "Skeleton AST (Abstract Syntax Tree) Visitor" for
visiting the nodes of one or more C++ codes that you provide as command line
arguments. We're not currently doing anything particularly important with the
AST nodes. Instead, we're just printing (to standard output) a transcript of
what we find, given the inputs you provide.

The code below is arranged into the following sections:

   - Helper constructs (just for printing)
   - class Visitor
   - Definitions of Visitor's Visit* functions
   - class Consumer
   - class Action
   - class Factory
   - class Database
   - visit()
   - main()

We chose a sort of "top down" ordering, in this file, for the above constructs,
so that we don't need forward declarations all over the place. As we'll see,
a Visitor is created by a Consumer, which is created by an Action, which is
created by a Factory. A Factory and a Database are created and used in visit(),
which is called by main().


------------------------
Classes
------------------------

Our five major classes each derive from something in clang::, as follows:

   class Visitor  : public clang::RecursiveASTVisitor<Visitor>
   class Consumer : public clang::ASTConsumer
   class Action   : public clang::ASTFrontendAction
   class Factory  : public clang::tooling::FrontendActionFactory
   class Database : public clang::tooling::CompilationDatabase


------------------------
Data
------------------------

The data we store in those classes proper (excluding data in their clang::
bases) are as follows:

   Visitor
      clang::CompilerInstance &ci
      clang::Sema &sema
      clang::ASTContext &context

   Consumer
      clang::CompilerInstance &ci

   Action
      (none)

   Factory
      (none)

   Database
      (none)

We'll omit a description of how the data are passed around, as that's easy to
see from the code. Remark: Visitor's Visit*() callbacks don't currently use any
of Visitor's data. We set up those particular things (CompilerInstance, Sema,
ASTContext) because a real code is likely to use one or more of them.


------------------------
Terminology
------------------------

   AST = (C++) Abstract Syntax Tree.
   Clang Tooling, or just Tooling, means the Clang Tooling library.


------------------------
Outline
------------------------

Our main() just sees if command-line arguments are given and then calls visit(),
which sets things up and initiates AST visitation.

A chain of events for visiting the AST involves:

   Creating a Factory...
   which creates an Action...
   which creates a Consumer...
   which creates a Visitor...
   which contains callbacks for different types of AST nodes.

Editorial Remark: The above seems, in our opinion, to be overly complicated.
The override in Factory looks like it can create just one Action, and the
override in Action looks like it can create just one Consumer. We're not sure,
then, why a person can't just create a Consumer directly and chuck Factory
and Action entirely. Perhaps this is possible; at the moment, we don't know.


------------------------
Control Flow - Detailed
------------------------

We enter main(), as usual, and with the usual suspects: argc and argv. main()
mainly just calls visit(argc,argv) to do the important work - setting up and
running the process of visiting some ASTs.

visit() initializes up a few things Clang Tooling will need, then initiates
the AST-visiting process in earnest. Let's break this down....

First, visit() makes a Factory object. Factory, one of our classes, derives from
clang::tooling::FrontendActionFactory. Essentially, its purpose is to produce an
action for Tooling to invoke. We'll cover the details in a moment.

Next, visit() makes a Database object. The basic purpose of our Database class,
which derives from clang::tooling::CompilationDatabase, is to encapsulate a
representation of some series of compilation steps. We're not actually going
to be compiling anything, you might say; we're going to be visiting the nodes
of an abstract syntax tree. That's true, but the Tooling system visits an AST
based on having obtained C++ code "as if" the code had been produced by some
particular compilation process. Think, e.g., "include files"; how would Tooling
know where to find them, if not for some stipulated compilation process?

Next, visit() pulls command line arguments (taken to be C++ file names) from
argv, and places them into a clang::ArrayRef<std::string> object. Clang Tooling
seems to like ArrayRef objects. We don't know why it doesn't just accept a good
old-fashioned vector of file names here.

The Database and ArrayRef objects we just created are used now to initialize an
object of class clang::tooling::ClangTool. Think of ClangTool as a high-level
object from which we'll be able to initiate the process of visiting the ASTs of
some input files.

We now call the ClangTool object's run(), which takes (a pointer to) the Factory
object we created earlier.

Through either its construction or its run() call, then, ClangTool has knowledge
of three things: (1) the Database, which provides "compilation commands" in some
form; (2) the files, pulled from argv, that we wish to examine; and (3) the
Factory, which run() queries in order to get an action to perform.

Back to Factory. Its purpose, again, is to produce an action for the Tooling
system to run. Factory produces such an action through its create() override,
which, in our code, returns an object of our Action class.

Action (derived from clang::ASTFrontendAction) produces an "AST consumer" via
its CreateASTConsumer() override. More precisely, it creates a std::unique_ptr
to an AST consumer. (We're not sure why Clang Tooling wants Factory's create()
override to provide a regular pointer, but wants Action's CreateASTConsumer()
override to provide a std::unique_ptr.)

The CreateASTConsumer() function, ultimately called from somewhere in Tooling,
receives a clang::CompilerInstance and an llvm::StringRef. These parameters
describe a particular compiler invocation on a particular C++ file. (It would
seem, however, that we don't actually need the file here, explicitly. In some
form, its representation - probably in the form of an actual AST - is embedded
in the CompilerInstance. If it weren't, we couldn't get much further in our
process without using the file in something more than our diagnostic printing!)

Our Action's CreateASTConsumer() creates an object of our Consumer class,
giving it back to Tooling in the form of a std::unique_ptr.

Consumer (derived from clang::ASTConsumer) has yet another override: function
HandleTranslationUnit(). This function creates an object of our Visitor class,
and calls its TraverseDecl() function. This, ultimately, is the call that makes
Clang Tooling traverse an AST.

Finally, our Visitor class. It derives from clang::RecursiveASTVisitor<Visitor>.
In this class, we provide several functions that begin with "Visit"; these
individual functions correspond to different types of AST nodes for which Clang
Tooling should call back to our code. Note that these Visit*() functions are NOT
overrides.

Many Visit* functions can be provided, for various kinds of nodes in a C++
AST. Ideally, one could compile a list of viable Visit* functions by examining
Clang's RecursiveASTVisitor.h file. In practice, however, at the time of this
writing, RecursiveASTVisitor.h uses macros in order to construct most of its
Visit* functions. That, unfortunately, obfuscates the issue.


------------------------
Pseudocode
------------------------

call main(argc,argv)
   call visit(argc,argv)
      make Factory
      make Database // Contains compilation commands
      make ArrayRef // Contains input file names
      make ClangTool(Database,ArrayRef)
      call ClangTool.run(Factory)
         for each file {
            // As determined by Database's getCompileCommands(file) override...
            for each compile command { // Clang makes trans unit for given file
               get Factory's Action  // Via our create() override
               get Action's Consumer // Via our CreateASTConsumer() override
               call Consumer.HandleTranslationUnit() // <== Our override
                  make Visitor
                  call Visitor.TraverseDecl() // Applied to the translation unit
                     // Pursuant to what's in the AST, and
                     // to what Visit*() functions you provide...
                     Visitor.VisitCXXRecordDecl (...)
                     Visitor.VisitVarDecl (...)
                     Visitor.VisitCallExpr (...)
                     Visitor.VisitTypeAliasDecl (...)
                     Visitor.VisitContinueStmt (...)
                     // ...
            } // For each compile command
         } // For each file
*/

// -----------------------------------------------------------------------------
// Helper constructs - printing
// -----------------------------------------------------------------------------

// macro: printval
#define printval(x) std::cout << #x " == " << (x) << std::endl

// print()
inline void
print() {
  std::cout << std::endl;
}

// print(x)
template<class T>
inline void
print(const T & x, const bool newline = true) {
  std::cout << x;
  if(newline)
    std::cout << std::endl;
}

// -----------------------------------------------------------------------------
// Visitor
// -----------------------------------------------------------------------------

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
  clang::CompilerInstance & ci;
  clang::Sema & sema;
  clang::ASTContext & context;

public:
  Visitor(clang::CompilerInstance & ref)
    : ci(ref), sema(ref.getSema()), context(ref.getASTContext()) {
    print("Visitor::Visitor()");
    printval((void *)&ci);
    printval((void *)&sema);
    printval((void *)&context);
  }

  ~Visitor() {
    print("Visitor::~Visitor()");
  }

  // for various types of AST nodes...
  bool VisitCXXRecordDecl(const clang::CXXRecordDecl * const);
  bool VisitVarDecl(const clang::VarDecl * const);
  bool VisitCallExpr(const clang::CallExpr * const);
  bool VisitTypeAliasDecl(const clang::TypeAliasDecl * const);
  bool VisitContinueStmt(const clang::ContinueStmt * const);
  // ...
};

// -----------------------------------------------------------------------------
// Visitor::Visit*()
// The bool return value tells Clang's AST visitor (the thing that's calling
// these functions) whether or not it should continue the AST traversal. For
// a typical "code analyzer" tool, we'd anticipate that just returning true,
// always, is probably the right thing to do, absent some sort of fatal error.
// -----------------------------------------------------------------------------

// VisitCXXRecordDecl
// Example: struct foo { };
bool
Visitor::VisitCXXRecordDecl(const clang::CXXRecordDecl * const) {
  print("   >>> Visitor::VisitCXXRecordDecl()");
  return true;
}

// VisitVarDecl
// Example: int i = 0;
bool
Visitor::VisitVarDecl(const clang::VarDecl * const) {
  print("   >>> Visitor::VisitVarDecl()");
  return true;
}

// VisitCallExpr
// Example: int i = fun();
// Where fun() is some function.
bool
Visitor::VisitCallExpr(const clang::CallExpr * const) {
  print("   >>> Visitor::VisitCallExpr()");
  return true;
}

// VisitTypeAliasDecl
// Example: using Int = int;
bool
Visitor::VisitTypeAliasDecl(const clang::TypeAliasDecl * const) {
  print("   >>> Visitor::VisitTypeAliasDecl()");
  return true;
}

// VisitContinueStmt
// Example: continue;
// Most applications probably wouldn't find too much value in visiting
// a "continue" statement. We're including this as just another example
// of the many Visit* functions that someone can provide.
bool
Visitor::VisitContinueStmt(const clang::ContinueStmt * const) {
  print("   >>> Visitor::VisitContinueStmt()");
  return true;
}

// -----------------------------------------------------------------------------
// Consumer
// -----------------------------------------------------------------------------

class Consumer : public clang::ASTConsumer
{
  clang::CompilerInstance & ci;

public:
  Consumer(clang::CompilerInstance & ref) : ci(ref) {
    print("Consumer::Consumer()");
    printval((void *)&ci);
  }

  ~Consumer() {
    print("Consumer::~Consumer()");
  }

  // base function override
  void HandleTranslationUnit(clang::ASTContext & context) override {
    print("Consumer::HandleTranslationUnit()");
    Visitor visitor(ci);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
};

// -----------------------------------------------------------------------------
// Action
// -----------------------------------------------------------------------------

class Action : public clang::ASTFrontendAction
{
public:
  Action() {
    print("Action::Action()");
  }

  ~Action() {
    print("Action::~Action()");
  }

  // base function override
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance & ci,
    llvm::StringRef file) override {
    print("Action::CreateASTConsumer()");
    printval(file.str());
    return std::unique_ptr<clang::ASTConsumer>(new Consumer(ci));
  }
};

// -----------------------------------------------------------------------------
// Factory
// -----------------------------------------------------------------------------

class Factory : public clang::tooling::FrontendActionFactory
{
public:
  Factory() {
    print("Factory::Factory()");
  }

  ~Factory() {
    print("Factory::~Factory()");
  }

  // base function override
  Action * create() override {
    print("Factory::create()");
    return new Action();
  }
};

// -----------------------------------------------------------------------------
// Database
// -----------------------------------------------------------------------------

class Database : public clang::tooling::CompilationDatabase
{
public:
  Database() {
    print("Database::Database()");
  }

  ~Database() {
    print("Database::~Database()");
  }

  // base function override
  std::vector<clang::tooling::CompileCommand> getCompileCommands(
    const llvm::StringRef file) const override {
    print("Database::getCompileCommands()");
    printval(file.str());

    // vector<compilation commands>
    std::vector<clang::tooling::CompileCommand> commands;

    // a compilation command for the given file
    {
      clang::tooling::CompileCommand c;
      c.Directory = ".";
      c.Filename = file.str();
      c.CommandLine.push_back("clang++");
      c.CommandLine.push_back("-std=c++14");
      c.CommandLine.push_back(file.str());
      commands.push_back(c);
    }

    // another compilation command for the given file
    {
      clang::tooling::CompileCommand c;
      c.Directory = ".";
      c.Filename = file.str();
      c.CommandLine.push_back("clang++");
      c.CommandLine.push_back("-std=c++14");
      c.CommandLine.push_back("-DFOOBAR"); // new
      c.CommandLine.push_back(file.str());
      commands.push_back(c);
    }

    return commands;
  }
};

// -----------------------------------------------------------------------------
// visit()
// -----------------------------------------------------------------------------

bool
visit(const int argc, const char * const * const argv) {
  print("visit()");

  // make a Factory
  Factory factory;

  // make a Database
  Database db;

  // make an ArrayRef<string> from the command-line arguments, which
  // we take to be the files we should examine with our AST visitor
  std::vector<std::string> vec;
  for(int a = 1; a < argc; ++a)
    vec.push_back(argv[a]);
  const clang::ArrayRef<std::string> files(vec);

  // make a ClangTool, from the Database and the ArrayRef
  clang::tooling::ClangTool ctool(db, files);

  // run the ClangTool, with the Factory
  const int status = ctool.run(&factory);

  printval(status);
  return status == 0;
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int
main(const int argc, const char * const * const argv) {
  if(argc < 2) {
    print(argv[0], false);
    print(": no input files");
    return 1;
  }

  print("main()");
  visit(argc, argv);
}
