/* -*- C++ -*- */

#include "flecstan-misc.h"

namespace flecstan {



// -----------------------------------------------------------------------------
// getFileLineColumn: helper
//
// Remark: This code largely mirrors the code in clang::SourceLocation::print(),
// which is called by clang::SourceLocation::dump(). Earlier, I (Martin) did
// file/line/column extraction in a different way - one I'd seen other people
// doing. For whatever reason, the other methodology was less robust than this
// one. Specifically, it reliably obtained file/line/column in most contexts,
// but didn't do so in the particular case of extracting those values from clang
// diagnostics *when* the diagnostics were associated with macro-inserted code
// that was being compiled. Outside of the diagnostics context (in other places
// where we want file name etc.), or for diagnostics that weren't triggered by
// macro-inserted code, the old scheme worked. :-/ We're not sure why there
// apparently isn't a single (and simpler!) way to get the desired information.
// -----------------------------------------------------------------------------

void getFileLineColumn(
   // input
   const clang::SourceManager *const man,
   const clang::SourceLocation &loc,

   // output
   // Remark: line and column are strings, not integers, so
   // that we can write "unknown" into them if we need to.
   std::string &file,
   std::string &line,
   std::string &column
) {
   if (man != nullptr && loc.isValid()) {
      if (loc.isFileID()) {
         clang::PresumedLoc pre = man->getPresumedLoc(loc);
         if (pre.isValid()) {
            file   = pre.getFilename();
            line   = std::to_string(pre.getLine  ());
            column = std::to_string(pre.getColumn());
            return;
         }
      } else {
         // Remark: I hope that there isn't any way this can trigger infinite
         // recursion. That it doesn't do so is not entirely obvious, but I'll
         // assume for now that it's OK, because, as stated earlier, our code
         // here essentially mirrors the code in clang::SourceLocation::print().
         // Note that the case in which the recursion occurs can be described
         // as, "the inputs (source manager and location) are entirely valid,
         // but the location isn't a file [id]." Presumably, in this situation,
         // getExpansionLoc() below gets us into a different state of affairs.
         getFileLineColumn(man, man->getExpansionLoc(loc), file, line, column);
         return;
      }
   }

   file   = "unknown";
   line   = "unknown";
   column = "unknown";
}



// -----------------------------------------------------------------------------
// Remark: Our handling of colors, in some of the below functions, may *appear*
// to be more complicated than it needs to be. In particular, we'll explicitly
// set and unset color codes on a per-line, not per-block-of-lines, basis. While
// this clutters the output with more color escape sequences than are necessary,
// we're doing it intentionally: so that colors are preserved if, for example,
// you send the code's output through something that processes individual lines.
// Imagine, for example, using the Unix "grep" command to display lines with the
// word "foo" in them:
//
//    flecstan ... | grep foo
//
// Well, grep isn't the best example, because, depending on how it's configured,
// it may mess with color codes or insert its own. You may, however, be able to
// switch that off, for example with:
//
//    flecstan ... | grep --color=never foo
//
// So, we believe that our line-by-line color encoding gives more flexibility in
// terms of what you can do with the output.
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// debug
// -----------------------------------------------------------------------------

void debug(const std::string &text)
{
   if (!emit_debug)
      return;

   // color
   const std::string &color = emit_color ? color::debug : "";
   const std::string &reset = emit_color ? color::reset : "";

   // content
   std::cout << color << "Debug: ";
   const std::size_t size = text.size();
   for (std::size_t i = 0;  i < size;  ++i) {
      const bool internal_newline = text[i] == '\n' && i < size-1;
      if (internal_newline) std::cout << reset;
      std::cout << text[i];
      if (internal_newline) std::cout << color << "Debug: ";
   }
   std::cout << reset;

   // finish
   if (text != "" && text.back() != '\n')
      std::cout << "\n";
   std::cout << std::flush;
}



// -----------------------------------------------------------------------------
// heading
// -----------------------------------------------------------------------------

bool first_print = true;

void heading(const std::string &text, const bool emit_section)
{
   section_on = emit_section;
   if (!section_on) return;

   static bool first = true;
   if (first)
      first = false;
   else if (emit_formfeed)
      std::cout << "\f";

   const std::string &color = emit_color ? color::heading : "";
   const std::string &reset = emit_color ? color::reset   : "";
   static const std::string line(80,'-');

   if (emit_heading) {
      std::cout << (first_print ? "" : "\n");
      first_print = false;

      if (short_form)
         std::cout
            << color << text << reset;
      else
         std::cout
            << color << line << reset << "\n"
            << color << text << reset << "\n"
            << color << line << reset;

      std::cout << std::endl;
   }
}



// -----------------------------------------------------------------------------
// diagnostic
// Written compactly more so than efficiently. For I/O, I'm not going to care.
// -----------------------------------------------------------------------------

void diagnostic(
   const std::string &label,
   const std::string &text,
   std::string maincolor,
   std::string textcolor,
   const bool keep_long_form // override short-form flag?
) {
   /*
   // spacing
   if (!short_form)
      std::cout << (first_print ? "" : "\n");
   */

   // spacing
   if (!short_form && !first_print)
      std::cout << "\n";
   first_print = false;

   // color
   if (!emit_color)
      textcolor = maincolor = "";
   else if (textcolor == "")
      textcolor = maincolor;
   const std::string &reset = emit_color ? color::reset : "";

   // content
   if (label == "" || (short_form && !keep_long_form))
      std::cout << maincolor << text << reset;
   else {
      std::cout << maincolor << label << ":" << reset << "\n   ";
      std::cout << textcolor;

      const std::size_t size = text.size();
      for (std::size_t i = 0;  i < size;  ++i) {
         const bool internal_newline = text[i] == '\n' && i < size-1;
         if (internal_newline) std::cout << reset;
         std::cout << text[i];
         if (internal_newline) std::cout << "   " << textcolor;
      }

      std::cout << reset;
   }

   // finish
   if (text != "" && text.back() != '\n')
      std::cout << "\n";
   std::cout << std::flush;
}

} // namespace flecstan
