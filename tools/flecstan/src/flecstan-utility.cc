/* -*- C++ -*- */

#include "flecstan-utility.h"
#include "clang/AST/ExprCXX.h"

namespace flecstan {



// -----------------------------------------------------------------------------
// get*
// Parameters: clang::CallExpr *
// -----------------------------------------------------------------------------

// getMethod
const clang::CXXMethodDecl *getMethod(
   // CallExpr...
   const clang::CallExpr *const ce
) {
   debug("getMethod()");
   if (!ce) return nullptr;

   // ...Decl
   const clang::Decl *const
      dcl = ce->getCalleeDecl();
   if (!dcl) return nullptr;

   // ...CXXMethodDecl
   const clang::CXXMethodDecl *const
      cmd = clang::dyn_cast<clang::CXXMethodDecl>(dcl);
   if (!cmd) return nullptr;

   return cmd;
}



// getTemplateArgs
const clang::TemplateArgumentList *getTemplateArgs(
   // CallExpr...
   const clang::CallExpr *const ce
) {
   debug("getTemplateArgs()");
   if (!ce) return nullptr;

   // ...CXXMethodDecl
   const clang::CXXMethodDecl *const
      cmd = getMethod(ce);
   if (!cmd) return nullptr;

   // ...TemplateArgumentList
   const clang::TemplateArgumentList *const
      tal = cmd->getTemplateSpecializationArgs();
   if (!tal) return nullptr;

   return tal;
}



// -----------------------------------------------------------------------------
// get*Arg
// Parameters: clang::TemplateArgumentList *, size_t
// -----------------------------------------------------------------------------

// getTypeArg
clang::QualType getTypeArg(
   const clang::TemplateArgumentList *const tal,
   const std::size_t param
) {
   debug("getTypeArg()");
   flecstan_assert(tal != nullptr);
   const clang::TemplateArgument &arg = tal->get(param);
   flecstan_assert(arg.getKind() == clang::TemplateArgument::Type);
   return arg.getAsType();
}

// getIntArg
std::int64_t getIntArg(
   const clang::TemplateArgumentList *const tal,
   const std::size_t param
) {
   debug("getIntArg()");
   flecstan_assert(tal != nullptr);
   const clang::TemplateArgument &arg = tal->get(param);
   flecstan_assert(arg.getKind() == clang::TemplateArgument::Integral);
   return arg.getAsIntegral().getSExtValue();
}

// getUIntArg
std::uint64_t getUIntArg(
   const clang::TemplateArgumentList *const tal,
   const std::size_t param
) {
   debug("getUIntArg()");
   flecstan_assert(tal != nullptr);
   const clang::TemplateArgument &arg = tal->get(param);
   flecstan_assert(arg.getKind() == clang::TemplateArgument::Integral);
   return arg.getAsIntegral().getZExtValue();
}



// -----------------------------------------------------------------------------
// get*Name
// Parameters: clang::NamedDecl *
// -----------------------------------------------------------------------------

// getName
std::string getName(const clang::NamedDecl *const nd)
{
   flecstan_assert(nd != nullptr);
   return nd->getNameAsString();
}

// getQualifiedName
std::string getQualifiedName(const clang::NamedDecl *const nd)
{
   flecstan_assert(nd != nullptr);
   return nd->getQualifiedNameAsString();
}



// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

// getClassDecl
const clang::CXXRecordDecl *getClassDecl(
   // QualType...
   const clang::QualType &qt
) {
   debug("getClassDecl()");

   // ...Type
   const clang::Type *const tp = qt.getTypePtr();
   if (!tp) return nullptr;

   // ...RecordType
   const clang::RecordType *const rt = clang::dyn_cast<clang::RecordType>(tp);
   if (!rt) return nullptr;

   // ...RecordDecl
   const clang::RecordDecl *rd = rt->getDecl();
   if (!rd) return nullptr;

   // ...CXXRecordDecl
   const clang::CXXRecordDecl *cd = clang::dyn_cast<clang::CXXRecordDecl>(rd);
   if (!cd) return nullptr;

   return cd;
}



// normExpr
const clang::Expr *normExpr(const clang::Expr *const expr)
{
   debug("normExpr()");
   const clang::ExprWithCleanups *const
      ec = clang::dyn_cast<clang::ExprWithCleanups>(expr);
   return ec ? ec->getSubExpr() : expr;
}



// isDerivedFrom
bool isDerivedFrom(
   const clang::CXXRecordDecl *const cd,
   const std::string &qualifiedBaseName
) {
   debug("isDerivedFrom()");

   if (!cd)
      return false;

   if (getQualifiedName(cd) == qualifiedBaseName)
      return true;

   for (auto base : cd->bases()) {
      const clang::CXXRecordDecl *const rd =
         base.getType().getTypePtr()->getAsCXXRecordDecl();
      if (isDerivedFrom(rd, qualifiedBaseName))
         return true;
   }

   return false;
}



// getClassCall
const clang::CallExpr *getClassCall(
   // Expr...
   const clang::Expr *const expr,
   const std::string &theclass,
   const std::string &thefunction,
   const int minArgs,
   const int maxArgs_
) {
   debug("getClassCall()");
   if (!expr) return nullptr;

   // ...Expr
   const clang::Expr *const e = normExpr(expr);
   if (!e) return nullptr;

   // ...CallExpr
   const clang::CallExpr *const call = clang::dyn_cast<clang::CallExpr>(e);
   if (!call) return nullptr;

   // ...CXXMethodDecl
   // gives the method
   const clang::CXXMethodDecl *const md = getMethod(call);
   if (!md) return nullptr;

   // ...CXXRecordDecl
   // gives the class containing the method
   const clang::CXXRecordDecl *const rd = md->getParent();
   if (!rd) return nullptr;

   // re: arguments
   const int maxArgs = maxArgs_ < 0 ? minArgs : maxArgs_;
   const int numArgs = call->getNumArgs();
   if (!(minArgs <= numArgs && numArgs <= maxArgs))
      return nullptr;

   // re: names
   debug(std::string("class    (have): ") + getQualifiedName(rd));
   debug(std::string("function (have): ") + getName(md));
   debug(std::string("class    (want): ") + theclass);
   debug(std::string("function (want): ") + thefunction);

   return theclass == getQualifiedName(rd) && thefunction == getName(md)
      ? call
      : nullptr;
}

} // namespace flecstan
