/***   CIrewriter.cpp   ******************************************************
 * This code is licensed under the New BSD license.
 * See LICENSE.txt for details.
 *
 * This tutorial was written by Robert Ankeney.
 * Send comments to rrankene@gmail.com.
 * 
 * This tutorial is an example of using the Clang Rewriter class coupled
 * with the RecursiveASTVisitor class to parse and modify C code.
 *
 * Expressions of the form:
 *     (expr1 && expr2)
 * are rewritten as:
 *     L_AND(expr1, expr2)
 * and expressions of the form:
 *     (expr1 || expr2)
 * are rewritten as:
 *     L_OR(expr1, expr2)
 *
 * Functions are located and a comment is placed before and after the function.
 *
 * Statements of the type:
 *   if (expr)
 *      xxx;
 *   else
 *      yyy;
 *
 * are converted to:
 *   if (expr)
 *   {
 *      xxx;
 *   }
 *   else
 *   {
 *      yyy;
 *   }
 *
 * And similarly for while and for statements.
 *
 * Interesting information is printed on stderr.
 *
 * Usage:
 * CIrewriter <options> <file>.c
 * where <options> allow for parameters to be passed to the preprocessor
 * such as -DFOO to define FOO.
 *
 * Generated as output <file>_out.c
 *
 * Note: This tutorial uses the CompilerInstance object which has as one of
 * its purposes to create commonly used Clang types.
 *****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <system_error>
#include <dirent.h>
#include <memory>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"


#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "blender/ast/preprocessor_analyzer.h"
#include "blender/ast/flecsi_info.h"
#include "blender/ast/generator.h"

using namespace clang;

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace blender;

Rewriter rewriter;
int numFunctions = 0;
flecsi_info_t * blender::f_info = new flecsi_info_t();

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info

public:


    virtual bool VisitFunctionDecl(FunctionDecl *func) {
//        numFunctions++;
//        string funcName = func->getNameInfo().getName().getAsString();
//        if (funcName == "do_math") {
//            rewriter.ReplaceText(func->getLocation(), funcName.length(), "add5");
//            errs() << "** Rewrote function def: " << funcName << "\n";
//        }
        return true;
    }

    virtual bool VisitStmt(Stmt *st) {
//        if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
//            rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
//            errs() << "** Rewrote ReturnStmt\n";
//        }
//        if (CallExpr *call = dyn_cast<CallExpr>(st)) {
//            rewriter.ReplaceText(call->getLocStart(), 7, "add5");
//            errs() << "** Rewrote function call\n";
//        }
        return true;
    }

/*
    virtual bool VisitReturnStmt(ReturnStmt *ret) {
        rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
        errs() << "** Rewrote ReturnStmt\n";
        return true;
    }
    virtual bool VisitCallExpr(CallExpr *call) {
        rewriter.ReplaceText(call->getLocStart(), 7, "add5");
        errs() << "** Rewrote function call\n";
        return true;
    }
*/
};




int main(int argc,const  char **argv)
{

  if (argc < 2)
  {
     llvm::errs() << "Usage: CIrewriter <options> <filename>\n";
     return 1;
  }

	using namespace std;
	using namespace llvm;
	using namespace clang;
	using namespace tooling;
	using namespace comments;

	  generator__<ExampleVisitor> gen;
	  gen.generate(argc,argv);


	  for(auto task : blender::f_info->tasks)
		  printf("Task: %s\n",task.c_str());

  return 0;
}

