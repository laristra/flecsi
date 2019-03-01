/* -*- C++ -*- */

#ifndef flecstan_visitor
#define flecstan_visitor

#include "flecstan-prep.h"
#include "clang/AST/RecursiveASTVisitor.h"



// -----------------------------------------------------------------------------
// Visitor
// -----------------------------------------------------------------------------

namespace flecstan {

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
   // data
   clang::Sema  &sema;
   Preprocessor &prep;
   Yaml         &yaml;

public:

   // constructor
   Visitor(clang::Sema &s, Preprocessor &p, Yaml &y)
    : sema(s), prep(p), yaml(y)
   {
      debug("ctor: Visitor");
   }

   // destructor
  ~Visitor()
   {
      debug("dtor: Visitor");
   }

   // callbacks for clang::RecursiveASTVisitor; definitions are in .cpp
   bool VisitVarDecl         (const clang::VarDecl          *const);
   bool VisitCallExpr        (const clang::CallExpr         *const);
   bool VisitTypeAliasDecl   (const clang::TypeAliasDecl    *const);
   bool VisitCXXConstructExpr(const clang::CXXConstructExpr *const);
};

} // namespace flecstan

#endif
