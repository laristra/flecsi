/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 12, 2017
//----------------------------------------------------------------------------//

#include "blender/ast/preprocessor_analyzer.h"

namespace blender {

  void
  preprocessor_analyzer_t::MacroDefined(
    const clang::Token & MacroNameTok,
    const clang::MacroDirective * MD
  )
  {
  } // preprocessor_analyzer_t::MacroDefined

  void
  preprocessor_analyzer_t::MacroExpands(
    const clang::Token & MacroNameTok,
    const clang::MacroDefinition & MD,
    clang::SourceRange Range,
    const clang::MacroArgs * Args
  )
  {
  } // preprocessor_analyzer_t::MacroExpands

} // namespace blender

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
