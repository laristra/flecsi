/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef visitor_h
#define visitor_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

#include <clang/AST/RecursiveASTVisitor.h>

namespace blender {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct visitor_t : public clang::RecursiveASTVisitor<visitor_t>
{

  virtual
  bool
  VisitFunctionDecl(
    clang::FunctionDecl * func
  )
  {
  } // VisitFunctionDecl

  virtual
  bool
  VisitStmt(
    clang::Stmt * st
  )
  {
  } // VisitStmt

}; // class visitor_t

} // namespace blender

#endif // visitor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
