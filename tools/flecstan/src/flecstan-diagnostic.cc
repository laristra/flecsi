/* -*- C++ -*- */

#include "flecstan-misc.h"
#include "flecstan-diagnostic.h"



// -----------------------------------------------------------------------------
// Diagnostic::HandleDiagnostic
// -----------------------------------------------------------------------------

namespace flecstan {

void Diagnostic::HandleDiagnostic(
   clang::DiagnosticsEngine::Level level,
   const clang::Diagnostic &diag
) {
   // Construct our own message about Clang++ finding something...


   // ------------------------
   // Intro
   // ------------------------

   std::ostringstream oss;
   oss << "Clang++ reports ";

   switch (level) {
      case clang::DiagnosticsEngine::Ignored :
         oss << "an \"ignored\" diagnostic:";
         break;
      case clang::DiagnosticsEngine::Note :
         oss << "a note:";
         break;
      case clang::DiagnosticsEngine::Remark :
         oss << "a remark:";
         break;
      case clang::DiagnosticsEngine::Warning :
         oss << "a warning:";
         break;
      case clang::DiagnosticsEngine::Error :
         oss << "an error:";
         break;
      case clang::DiagnosticsEngine::Fatal :
         oss << "a fatal error:";
         break;
      default :
         oss << "a diagnostic:";
         break;
   }


   // ------------------------
   // Clang's message
   // ------------------------

   // Remark: The given SmallVector size is for efficiency only;
   // it can end up being larger, without overrunning any buffer.
   llvm::SmallVector<char,100> out;
   diag.FormatDiagnostic(out);
   const std::string text(out.begin(),out.end());


   // ------------------------
   // Expansion trace
   // ------------------------

   // Extract any expansion trace information. Remark: clang++'s output might
   // lead someone to believe that this information would be placed into clang
   // notes and sent to the present function override (HandleDiagnostic()) as
   // such. Alas, the information is actually embedded elsewhere. So, below,
   // I've basically followed what clang does in DiagnosticRenderer.cpp, and
   // specifically in the emitSingleMacroExpansion() and emitMacroExpansions()
   // member functions of the DiagnosticRenderer class. For now I've simplified,
   // and don't deal with concepts like "ignored" and "macro depth/limit" that
   // those functions deal with. At some point I may study their code carefully
   // to see exactly how and why that's done. Presumably, it's an issue of
   // respecting things like certain diagnostic-emitting limits that somebody
   // might set on a command line.
   std::string expansion;
   static clang::LangOptions LangOpts;
   LangOpts.CPlusPlus = true;

   // Let's do the expansion trace only for Clang warnings, errors, and fatals.
   // Otherwise, I've noticed that the trace can end up being repeated, e.g. for
   // both an original error as well as a later error-related note.
   if (level == clang::DiagnosticsEngine::Warning ||
       level == clang::DiagnosticsEngine::Error ||
       level == clang::DiagnosticsEngine::Fatal
   ) {
      const clang::SourceLocation &loc = diag.getLocation();
      if (loc.isValid() && diag.hasSourceManager()) {
         for (clang::FullSourceLoc floc(loc,diag.getSourceManager());
              floc.isValid() && floc.isMacroID();
              floc = floc.getImmediateMacroCallerLoc()
         ) {
            clang::StringRef MacroName =
               clang::Lexer::getImmediateMacroNameForDiagnostics(
                  floc, floc.getManager(), LangOpts);

            FileLineColumn exp;
            getFileLineColumn(
               &diag.getSourceManager(), floc.getSpellingLoc(), exp);

            expansion +=
               "\n   " +
              (MacroName.empty()
                 ?  "from"
                 : ("from macro \"" + MacroName.str() + "\"")
               ) +
               " (file " + exp.file +
               ", line " + exp.line +
              (emit_column ? (", column " + exp.column) : "") + ")";
         }
      }
   }


   // ------------------------
   // File, line, column
   // ------------------------

   FileLineColumn flc;
   getFileLineColumn(diag,flc);


   // ------------------------
   // Overall message
   // ------------------------

   // create
   oss
      << "\n   " << text
      << "\nFile: " << flc.file
      << "\nLine: " << flc.line
      << (emit_column     ? ("\nColumn: "         + flc.column) : "")
      << (emit_trace &&
          expansion != "" ? ("\nExpansion trace:" + expansion ) : "")
   ;

   // emit
   status = std::max(status,
      level == clang::DiagnosticsEngine::Error ||
      level == clang::DiagnosticsEngine::Fatal
    ? error  (oss)
    : level == clang::DiagnosticsEngine::Warning
    ? warning(oss)
    : note   (oss)
   );
}

} // namespace flecstan
