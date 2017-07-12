/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef blender_preprocessor_analyzer_h
#define blender_preprocessor_analyzer_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

#include <clang/Lex/PPCallbacks.h>

namespace blender {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

struct preprocessor_analyzer_t : public clang::PPCallbacks
{

  void
  MacroDefined(
    const clang::Token & MacroNameTok,
    const clang::MacroDirective * MD
  )
  override;

  void
  MacroExpands(
    const clang::Token & MacroNameTok,
    const clang::MacroDefinition & MD,
    clang::SourceRange Range,
    const clang::MacroArgs * Args
  )
  override;

}; // struct preprocessor_analyzer_t

} // namespace blender

#endif // blender_preprocessor_analyzer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
