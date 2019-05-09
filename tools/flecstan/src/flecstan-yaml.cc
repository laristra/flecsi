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

#include "flecstan-yaml.h"

namespace flecstan {



// -----------------------------------------------------------------------------
// getVarArgsFunction
// -----------------------------------------------------------------------------

// Here, we receive a CallExpr for code that looks like this:
//
//    namespace::class::foo<bar>(some parameters, __VA_ARGS__);
//
// We wish to extract {type,value} pairs from the function's variadic argument
// list. Below, "start" is the number of parameters that precede __VA_ARGS__.

void getVarArgsFunction(
   const clang::CallExpr *const call,
   std::vector<VarArgTypeValue> &varargs,
   const unsigned start // defaults to 0
) {
   debug("getVarArgsFunction()");
   varargs.clear();

   clang::LangOptions LangOpts;
   LangOpts.CPlusPlus = true;
   const clang::PrintingPolicy Policy(LangOpts);

   const unsigned narg = call->getNumArgs();
   for (unsigned arg = start;  arg < narg;  ++arg) {
      std::string type  = "unknown type";
      std::string value = "unknown value";

      const clang::Expr *const expr = call->getArg(arg);
      if (expr) {
         type  = expr->getType().getAsString();

         std::string s;
         llvm::raw_string_ostream rso(s);
         expr->printPretty(rso, 0, Policy);
         value = rso.str();
      }

      varargs.push_back(VarArgTypeValue(type,value));
   }
}



// -----------------------------------------------------------------------------
// getVarArgsTemplate
// -----------------------------------------------------------------------------

// This (in contrast to the more-general getVarArgsFunction() above) is a
// specialized function for getting the __VA_ARGS__ parameters from a FleCSI
// construct like this:
//
//    using foo = flecsi::execution::function_handle_u<
//       return_type,
//       std::tuple<__VA_ARGS__>
//    >
//
// Here, the varargs are template parameters - not function parameters as they
// were in getVarArgsFunction() - and thus have only {type}, not {type,value}.

void getVarArgsTemplate(
   const clang::TypeAliasDecl *const alias,
   std::vector<VarArgType> &varargs,
   std::string &rhsname
) {
   debug("getVarArgsTemplate()");
   varargs.clear();

   // Example:
   // Consider that our type alias looks like this:
   //    using foo = bar<int,std::tuple<float,double>>
   // Remarks, below, reflect this example, where applicable.

   // Right-hand side:
   //    bar<int,std::tuple<float,double>>
   const clang::ClassTemplateSpecializationDecl *const rhs =
      clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(
         alias->getUnderlyingType().getTypePtr()->getAsCXXRecordDecl());
   flecstan_assert(rhs != nullptr);
   rhsname = rhs->getQualifiedNameAsString();

   // Right-hand side's template arguments:
   //    0. int
   //    1. std::tuple<float,double>
   const clang::TemplateArgumentList &rhsargs = rhs->getTemplateArgs();

   // The std::tuple, as a QualType
   const clang::QualType tupqt = rhsargs.get(1).getAsType();

   // The std::tuple, as a ClassTemplateSpecializationDecl
   const clang::ClassTemplateSpecializationDecl *const tup =
      clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(
         tupqt.getTypePtr()->getAsCXXRecordDecl());
   flecstan_assert(tup != nullptr);

   // The std::tuple's template argument list.
   // Remark: std::tuple is a variadic template class, and getTemplateArgs()
   // actually gives a "template argument list" with just one argument! :-/
   // The one argument represents the argument pack. (Clang doesn't seem to
   // try very hard to make things intuitive or easy.) So, below, we first
   // get the pack, and then we drill down into it.
   const clang::TemplateArgumentList &tupargs = tup->getTemplateArgs();
   flecstan_assert(tupargs.size() == 1); // one pack

   // The "one" argument: the parameter pack
   const clang::TemplateArgument &pack = tupargs.get(0);
   flecstan_assert(pack.getKind() == clang::TemplateArgument::Pack);

   // Extract the pack's contents. This, finally, gives us the __VA_ARGS__ that
   // we wanted in the first place! Too bad that Clang requires so much digging.
   for (auto iter = pack.pack_begin();  iter != pack.pack_end();  ++iter) {
      const clang::TemplateArgument &arg = *iter;
      // for our example TypeAlias, type == "float" or "double"...
      const std::string type = arg.getAsType().getAsString();
      varargs.push_back(VarArgType(type));
   }
}

} // namespace flecstan
