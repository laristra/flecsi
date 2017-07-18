/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef blender_generator_h
#define blender_generator_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include "blender/ast/preprocessor_analyzer.h"

namespace blender {

static llvm::cl::OptionCategory blender_description(
  "blender",
  "Static analysis tool for FleCSI"
);

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<
  typename VISITOR
>
struct generator__ : public VISITOR
{

  //--------------------------------------------------------------------------//
  //! AST consumer to process context.
  //--------------------------------------------------------------------------//

  class consumer_t : public clang::ASTConsumer
  {
  public:

    consumer_t(
      clang::CompilerInstance * ci,
      generator__ * visitor
    )
    :
      ci_(ci),
      visitor_(visitor)
    {}

    void
    HandleTranslationUnit(
      clang::ASTContext & context
    )
    {
      visitor_->TraverseDecl(context.getTranslationUnitDecl());
    } // HandleTranslationUnit

  private:

    generator__ * visitor_;
    clang::CompilerInstance * ci_;

  }; // class consumer_t

  //--------------------------------------------------------------------------//
  //! Action to take on compiler instance.
  //--------------------------------------------------------------------------//

  class action_t : public clang::ASTFrontendAction
  {
  public:

    action_t(
      generator__ * visitor
    )
    :
      visitor_(visitor)
    {}

    virtual
    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(
      clang::CompilerInstance & compilerInstance,
      clang::StringRef file
    )
    {
      compilerInstance.getPreprocessor().addPPCallbacks(
        std::unique_ptr<preprocessor_analyzer_t>(
          new preprocessor_analyzer_t
        )
      );

      return std::unique_ptr<clang::ASTConsumer>(
        new consumer_t(&compilerInstance, visitor_));
    } // CreateASTConsumer

  private:

    generator__ * visitor_;

  }; // class action_t

  //--------------------------------------------------------------------------//
  //! Object factory to create action handler.
  //--------------------------------------------------------------------------//

  class factory_t : public clang::tooling::FrontendActionFactory
  {
  public:

    factory_t(
      generator__ * visitor
    )
    :
      visitor_(visitor)
    {}

    action_t *
    create()
    {
      return new action_t(visitor_);
    } // create

  private:

    generator__ * visitor_;

  }; // class factory_t

  //--------------------------------------------------------------------------//
  // Run the actual analysis.
  //--------------------------------------------------------------------------//

  bool
  run(
    int argc,
    char ** argv
  )
  {
    clang::tooling::CommonOptionsParser op(argc,
      const_cast<const char **>(argv), blender_description);

    clang::tooling::ClangTool tool(op.getCompilations(),
      op.getSourcePathList());

    factory_t f(this);

    return tool.run(&f) == 0;
  } // run

}; // struct generator__

} // namespace blender

#endif // blender_generator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
