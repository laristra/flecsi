/* -*- C++ -*- */

#ifndef flecstan_utility
#define flecstan_utility

#include "flecstan-misc.h"

// -----------------------------------------------------------------------------
// Helper functions.
// Just declarations here.
// -----------------------------------------------------------------------------

namespace flecstan {

const clang::CXXMethodDecl * getMethod(const clang::CallExpr * const);
const clang::TemplateArgumentList * getTemplateArgs(
  const clang::CallExpr * const);

clang::QualType getTypeArg(const clang::TemplateArgumentList * const,
  const std::size_t);
std::int64_t getIntArg(const clang::TemplateArgumentList * const,
  const std::size_t);
std::uint64_t getUIntArg(const clang::TemplateArgumentList * const,
  const std::size_t);

std::string getName(const clang::NamedDecl * const);
std::string getQualifiedName(const clang::NamedDecl * const);

const clang::CXXRecordDecl * getClassDecl(const clang::QualType &);
const clang::Expr * normExpr(const clang::Expr * const);
bool isDerivedFrom(const clang::CXXRecordDecl * const, const std::string &);

const clang::CallExpr * getClassCall(const clang::Expr * const,
  const std::string &,
  const std::string &,
  const int,
  const int);

} // namespace flecstan

#endif
