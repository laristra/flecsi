/* -*- C++ -*- */

/* -----------------------------------------------------------------------------
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2019, Triad National Security, LLC
   All rights reserved.
----------------------------------------------------------------------------- */

#pragma once

#include "flecstan-macro.h"

namespace flecstan {

inline std::string summary_task_reg_dup;
inline std::string summary_task_reg_without_exe;
inline std::string summary_task_exe_without_reg;

inline std::string summary_function_reg_dup;
inline std::string summary_function_reg_without_hand;
inline std::string summary_function_hand_without_reg;

std::string uflcs(const flecsi_base &info, bool print_scope = true);



// -----------------------------------------------------------------------------
// called_matched
// Helper function
// -----------------------------------------------------------------------------

template<class T>
exit_status_t called_matched(
   const std::vector<MacroCall> &call,
   const std::vector<T> &mat,
   const std::string &macname
) {
   exit_status_t status = exit_clean;

   // Sizes
   const std::size_t csize = call.size();
   const std::size_t msize = mat .size();

   // Consistent?
   // Remark: I'm not absolutely certain, right now, that inconsistent sizes
   // between preprocessor-macro detections, and AST matches, is necessarily
   // a problem in all cases. Keep an eye on this.
   if (msize != csize)
      status = std::max(
         status,
         intwarn(
            "Re: macro \"" + macname + "\".\n"
            "Detected call count != "
            "detected C++ abstract syntax tree count."
            "\n   Macro Calls: " + std::to_string(csize) +
            "\n   Syntax Tree: " + std::to_string(msize),
            true // <== flag: might have been triggered by compilation errors
         )
      );

   return status;
}



// -----------------------------------------------------------------------------
// treg
// Task registration helper class
// -----------------------------------------------------------------------------

class treg : public flecsi_base {
public:
   // from flecsi_base:
   // ...string         unit
   // ...FileLineColumn location
   // ...FileLineColumn spelling
   // ...vector<string> scope

   std::string task;
   booland<std::string> nspace;
   booland<std::string> processor;
   booland<std::string> launch;
   std::string hash;

   // ctor: flecsi_register_task_simple
   treg(const flecsi_register_task_simple &obj) : flecsi_base(obj),
      task     (obj.task),
      processor(obj.processor),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_task
   treg(const flecsi_register_task &obj) : flecsi_base(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      processor(obj.processor),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_mpi_task_simple
   treg(const flecsi_register_mpi_task_simple &obj) : flecsi_base(obj),
      task     (obj.task),
      hash     (obj.hash)
   { }

   // ctor: flecsi_register_mpi_task
   treg(const flecsi_register_mpi_task &obj) : flecsi_base(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      hash     (obj.hash)
   { }
};



// -----------------------------------------------------------------------------
// texe
// Task execution helper class
// -----------------------------------------------------------------------------

class texe : public flecsi_base {
public:
   // from flecsi_base:
   // ...string         unit
   // ...FileLineColumn location
   // ...FileLineColumn spelling
   // ...vector<string> scope

   std::string task;
   booland<std::string> nspace;
   booland<std::string> launch;
   booland<std::string> type;
   booland<std::string> datatype;
   std::string hash;

   // ctor: flecsi_execute_task_simple
   texe(const flecsi_execute_task_simple &obj) : flecsi_base(obj),
      task     (obj.task),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_task
   texe(const flecsi_execute_task &obj) : flecsi_base(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      launch   (obj.launch),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_mpi_task_simple
   texe(const flecsi_execute_mpi_task_simple &obj) : flecsi_base(obj),
      task     (obj.task),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_mpi_task
   texe(const flecsi_execute_mpi_task &obj) : flecsi_base(obj),
      task     (obj.task),
      nspace   (obj.nspace),
      hash     (obj.hash)
   { }

   // ctor: flecsi_execute_reduction_task
   texe(const flecsi_execute_reduction_task &obj) : flecsi_base(obj),
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
