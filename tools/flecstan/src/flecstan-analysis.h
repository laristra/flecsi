/* -*- C++ -*- */

#ifndef flecstan_analysis
#define flecstan_analysis

#include "flecstan-macro.h"

namespace flecstan {

inline std::string summary_task_reg_dup;
inline std::string summary_task_reg_without_exe;
inline std::string summary_task_exe_without_reg;

inline std::string summary_function_reg_dup;
inline std::string summary_function_reg_without_hand;
inline std::string summary_function_hand_without_reg;

std::string flcc(const macrobase &info, bool print_context = true);



// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

// invoked_matched
template<class T>
exit_status_t invoked_matched(
   const std::vector<InvocationInfo> &inv,
   const std::vector<T> &mat,
   const std::string &name // of FleCSI macro
) {
   exit_status_t status = exit_clean;

   // Sizes
   const std::size_t isize = inv.size();
   const std::size_t msize = mat.size();

   // Consistent?
   // Remark: I'm not absolutely certain, right now, that inconsistent sizes
   // between preprocessor-macro detections, and AST matches, is necessarily
   // a problem in all cases. Keep an eye on this.
   if (msize != isize) {
      status = std::max(
         status,
         intwarn(
            "Re: macro \"" + name + "\".\n"
            "Detected invocation count != "
            "detected C++ abstract syntax tree count."
            "\n   Invocations: " + std::to_string(isize) +
            "\n   Syntax Tree: " + std::to_string(msize),
            true // <== flag: might have been triggered by compilation errors
         )
      );
   }

   return status;
}



// multiple
// Is a FleCSI macro called multiple times?
// Remark: It's perfectly normal for certain FleCSI macros to be called multiple
// times in a code. This is true, for example, of the task registration and task
// execution macros. Some, however, e.g. flecsi_register_program, are presumably
// intended to be called at most once, although it would appear that multiple
// such calls don't necessarily cause problems. The following function can be
// called regarding macros that we believe are intended to be invoked <= once.
template<class T>
exit_status_t multiple(
   const std::vector<T> &mat,
   const std::string &name
) {
   if (mat.size() <= 1)
      return exit_clean;

   std::ostringstream oss;
   oss << "Macro " + name + " is called more than once.\n";
   std::size_t count = 0;
   for (auto m : mat)
      oss << "   Call #" << ++count << ": " << flcc(m) << "\n";
   oss <<
      "This is not currently a problem, unless it caused compilation errors.\n"
      "However, multiple calls go against the intention of the macro."
   ;
   return warning(oss);
}



// -----------------------------------------------------------------------------
// treg
// Task registration helper class
// -----------------------------------------------------------------------------

class treg : public macrobase {
public:
   // from macrobase:
   // ...FileLineColumn location
   // ...FileLineColumn spelling
   // ...vector<string> context

   std::string task;
   booland<std::string> nspace;
   booland<std::string> processor;
   booland<std::string> launch;
   std::string hash;

   // ctor: flecsi_register_task_simple
   treg(const flecsi_register_task_simple &obj) : macrobase(obj),
      task     (obj.task),
      processor(obj.processor),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_task
   treg(const flecsi_register_task &obj) : macrobase(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      processor(obj.processor),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_mpi_task_simple
   treg(const flecsi_register_mpi_task_simple &obj) : macrobase(obj),
      task     (obj.task),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_mpi_task
   treg(const flecsi_register_mpi_task &obj) : macrobase(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      hash     (obj.hash)
   { }
};



// -----------------------------------------------------------------------------
// texe
// Task execution helper class
// -----------------------------------------------------------------------------

class texe : public macrobase {
public:
   // from macrobase:
   // ...FileLineColumn location
   // ...FileLineColumn spelling
   // ...vector<string> context

   std::string task;
   booland<std::string> nspace;
   booland<std::string> launch;
   booland<std::string> type;
   booland<std::string> datatype;
   std::string hash;

   // ctor: flecsi_execute_task_simple
   texe(const flecsi_execute_task_simple &obj) : macrobase(obj),
      task     (obj.task),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_task
   texe(const flecsi_execute_task &obj) : macrobase(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_mpi_task_simple
   texe(const flecsi_execute_mpi_task_simple &obj) : macrobase(obj),
      task     (obj.task),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_mpi_task
   texe(const flecsi_execute_mpi_task &obj) : macrobase(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_reduction_task
   texe(const flecsi_execute_reduction_task &obj) : macrobase(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      launch   (obj.launch),
      type     (obj.type),
      datatype (obj.datatype),
      hash     (obj.hash)
   { }
};



// -----------------------------------------------------------------------------
// analysis
// Declaration only
// -----------------------------------------------------------------------------

exit_status_t analysis(const flecstan::Yaml &);

} // namespace flecstan

#endif
