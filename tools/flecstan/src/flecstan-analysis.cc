/* -*- C++ -*- */

#include "flecstan-analysis.h"
#include <map>

namespace flecstan {

/*
Contents of each of our classes that correspond to FleCSI macros:

vector invoked
   string file
   string line
   string column

vector matched
   string file
   string line
   string column
   vector<string> context
   Plus macro-specific data; see below
*/



// -----------------------------------------------------------------------------
// Helper constructs
// -----------------------------------------------------------------------------

// flecstan_im
// Helper macro. Pulls out yaml.<field>.[invoked,matched], where <field>
// represents some FleCSI macro. We're invoking this macro at the start
// of many of our functions below, to do things that those function all
// need to do. This just makes the code more compact.
#define flecstan_im(field) \
   const auto &inv = yaml.field.invoked; \
   const auto &mat = yaml.field.matched; \
   (void)inv; /* for now, to silence any possible "unused" warnings */ \
   (void)mat; /* ditto */ \
   exit_status_t status = exit_clean; \
   status = std::max(status, invoked_matched(inv, mat, #field))



// stringify
#define _stringify(macro) #macro
#define  stringify(macro) _stringify(macro)



// flcc
// For printing: {file,line,column,context} information.
// Dumb name, but we're just calling this internally.
std::string flcc(const macrobase &info, bool print_context)
{
   std::ostringstream oss;

   // file, line
   oss << "file " << info.file << ", ";
   oss << "line " << info.line;

   // column, if appropriate
   if (emit_column)
      oss << ", column " << info.column;

   // context, if appropriate
   if (print_context) {
      std::string ctx = (info.file == "" ? "<file>" : info.file);
      for (std::size_t c = info.context.size();  c--; )
         ctx += "::" +
            (info.context[c] == "" ? "<namespace>" : info.context[c]);
      oss << ", context " << ctx;
   }

   // done
   return oss.str();
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// For each macro in each of the code blocks below, comments that begin with
//    // ...
// tell us what extra fields are contained in each element of vector "matched,"
// above and beyond those that are mentioned in the "Contents" comment at the
// beginning of this file. (See above.)
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// flecsi_register_program
// flecsi_register_top_level_driver
// -----------------------------------------------------------------------------

// ...string program
static exit_status_t
analyze_flecsi_register_program(const flecstan::Yaml &yaml)
{
   #define macro flecsi_register_program
   flecstan_im(macro);

   // Called multiple times?
   status = std::max(status,multiple(mat,stringify(macro)));

   // I won't worry about the fact that this macro at present doesn't actually
   // use its parameter. The macro isn't intended to be called more than once,
   // so the issue of different calls with different parameters is irrelevant,
   // at least right now.

   return status;
   #undef macro
}



// ...string driver
static exit_status_t
analyze_flecsi_register_top_level_driver(const flecstan::Yaml &yaml)
{
   #define macro flecsi_register_top_level_driver
   flecstan_im(macro);

   // Called multiple times?
   status = std::max(status,multiple(mat,stringify(macro)));

   return status;
   #undef macro
}



// -----------------------------------------------------------------------------
// flecsi_register_global_object
// flecsi_set_global_object
// flecsi_initialize_global_object
// flecsi_get_global_object
// -----------------------------------------------------------------------------

// ...string index
// ...string nspace
// ...string type
static exit_status_t
analyze_flecsi_register_global_object(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_global_object);
   // ( ) write something here
   return status;
}



// ...string index
// ...string nspace
// ...string type
// ...string obj
static exit_status_t
analyze_flecsi_set_global_object(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_set_global_object);
   // ( ) write something here
   return status;
}



// ...string index
// ...string nspace
// ...string type
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_initialize_global_object(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_initialize_global_object);
   // ( ) write something here
   return status;
}



// ...string index
// ...string nspace
// ...string type
static exit_status_t
analyze_flecsi_get_global_object(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_global_object);
   // ( ) write something here
   return status;
}



// -----------------------------------------------------------------------------
// flecsi_color
// flecsi_colors
// -----------------------------------------------------------------------------

// ...no additional parameters
static exit_status_t
analyze_flecsi_color(const flecstan::Yaml &yaml)
{
   #define macro flecsi_color
   flecstan_im(macro);
   // I'm not aware of any particular checks we need for this.
   return status;
   #undef macro
}



// ...no additional parameters
static exit_status_t
analyze_flecsi_colors(const flecstan::Yaml &yaml)
{
   #define macro flecsi_colors
   flecstan_im(macro);
   // I'm not aware of any particular checks we need for this.
   return status;
   #undef macro
}



// -----------------------------------------------------------------------------
// flecsi_register_reduction_operation
// -----------------------------------------------------------------------------

static exit_status_t
analyze_flecsi_register_reduction_operation(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_reduction_operation);
   // ( ) write something here
   return status;
}



// -----------------------------------------------------------------------------
// flecsi_register_function
// flecsi_execute_function
// flecsi_function_handle
// flecsi_define_function_type
// -----------------------------------------------------------------------------

// flecsi_register_function
// ...string func
// ...string nspace
// ...string hash
static exit_status_t
analyze_flecsi_register_function(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_function);
   return status;
}



// flecsi_execute_function
// ...string handle
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_function(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_function);
   return status;
}



// flecsi_function_handle
// ...string func
// ...string nspace
// ...string hash
static exit_status_t
analyze_flecsi_function_handle(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_function_handle);
   return status;
}



// flecsi_define_function_type
// ...string func
// ...string return_type
// ...vector<VarArgType> varargs
static exit_status_t
analyze_flecsi_define_function_type(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_define_function_type);
   return status;
}



// -----------------------------------------------------------------------------
// flecsi_register_data_client
// flecsi_register_field
// flecsi_register_global
// flecsi_register_color
// -----------------------------------------------------------------------------

static exit_status_t
analyze_flecsi_register_data_client(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_data_client);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_register_field(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_field);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_register_global(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_global);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_register_color(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_color);
   // ( ) write something here
   return status;
}



// -----------------------------------------------------------------------------
// flecsi_get_handle
// flecsi_get_client_handle
// flecsi_get_handles
// flecsi_get_handles_all
// -----------------------------------------------------------------------------

static exit_status_t
analyze_flecsi_get_handle(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_handle);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_get_client_handle(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_client_handle);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_get_handles(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_handles);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_get_handles_all(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_handles_all);
   // ( ) write something here
   return status;
}



// -----------------------------------------------------------------------------
// flecsi_get_global
// flecsi_get_color
// flecsi_get_mutator
// -----------------------------------------------------------------------------

static exit_status_t
analyze_flecsi_get_global(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_global);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_get_color(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_color);
   // ( ) write something here
   return status;
}



static exit_status_t
analyze_flecsi_get_mutator(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_get_mutator);
   // ( ) write something here
   return status;
}



// -----------------------------------------------------------------------------
// Task register/execute
// -----------------------------------------------------------------------------

// ------------------------
// Register
// ------------------------

// flecsi_register_task_simple
// ...string task
// ...string processor
// ...string launch
static exit_status_t
analyze_flecsi_register_task_simple(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_task_simple);
   return status;
}



// flecsi_register_task
// ...string task
// ...string nspace
// ...string processor
// ...string launch
static exit_status_t
analyze_flecsi_register_task(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_task);
   return status;
}



// flecsi_register_mpi_task_simple
// ...string task
static exit_status_t
analyze_flecsi_register_mpi_task_simple(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_mpi_task_simple);
   return status;
}



// flecsi_register_mpi_task
// ...string task
// ...string nspace
static exit_status_t
analyze_flecsi_register_mpi_task(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_register_mpi_task);
   return status;
}



// ------------------------
// Execute
// ------------------------

// flecsi_execute_task_simple
// ...string task
// ...string launch
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_task_simple(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_task_simple);
   return status;
}



// flecsi_execute_task
// ...string task
// ...string nspace
// ...string launch
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_task(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_task);
   return status;
}



// flecsi_execute_mpi_task_simple
// ...string task
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_mpi_task_simple(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_mpi_task_simple);
   return status;
}



// flecsi_execute_mpi_task
// ...string task
// ...string nspace
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_mpi_task(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_mpi_task);
   return status;
}



// flecsi_execute_reduction_task
// ...string task
// ...string nspace
// ...string launch
// ...string type
// ...string datatype
// ...vector<VarArgTypeValue> varargs
static exit_status_t
analyze_flecsi_execute_reduction_task(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_execute_reduction_task);
   return status;
}



// -----------------------------------------------------------------------------
// Task registration/execution
// -----------------------------------------------------------------------------

// Helper: Duplicate registrations?
// These are warnings
static exit_status_t task_reg_dup(
   const std::multimap<std::string,treg> &regs
) {
   exit_status_t status = exit_clean;

   for (auto a = regs.cbegin(), b = a;  a != regs.cend();  a = b) {
      bool unique = true;
      std::string str;

      while (++b != regs.cend() && b->first == a->first) {
         if (unique) {
            // then no longer; begin diagnostic
            str = "Hash string \"" + a->first + "\" was created "
                  "by multiple task registrations:\n   " +
                   flcc(a->second) + "\n";
            unique = false;
         }
         // (next) duplicate
         str += "   " + flcc(b->second) + "\n";

         // for summary
         summary_task_reg_dup +=
            "(" + flcc(b->second,false) + ") dups "
            "(" + flcc(a->second,false) + ")\n";
      }

      if (!unique) {
         // finish diagnostic
         str +=
           "This may have caused a (possibly cryptic) compile-time error.\n"
           "If not, a duplicate hash will still trigger a run-time error.\n"
           "Check your task registrations for incorrect task and/or "
           "namespace names.";
         status = error(str);
      }
   }

   return status;
}



// Helper: Registrations without executions?
// These are warnings
static exit_status_t task_reg_without_exe(
   const std::multimap<std::string,treg> &regs,
   const std::multimap<std::string,texe> &exes
) {
   exit_status_t status = exit_clean;

   for (auto reg : regs)
      if (exes.find(reg.first) == exes.end()) {
         status = warning(
           "The task, as registered with hash \"" +
            reg.first + "\" here:\n   " + flcc(reg.second) + "\n"
           "is never invoked with any of FleCSI's task execution macros.\n"
           "Is this intentional?"
         );
         summary_task_reg_without_exe += flcc(reg.second) + "\n";
      }

   return status;
}



// Helper: Executions without registrations?
// These are errors
static exit_status_t task_exe_without_reg(
   const std::multimap<std::string,treg> &regs,
   const std::multimap<std::string,texe> &exes
) {
   exit_status_t status = exit_clean;

   for (auto exe : exes)
      if (regs.find(exe.first) == regs.end()) {
         status = error(
           "The task, as executed with hash \"" +
            exe.first + "\" here:\n   " + flcc(exe.second,false) + "\n"
           "was not registered with any of FleCSI's task registration macros,\n"
           "or was not registered with that hash.\n"
           "This will trigger a run-time error if this line is reached."
         );
         summary_task_exe_without_reg += flcc(exe.second,false) + "\n";
      }

   return status;
}



// analyze_flecsi_task
static exit_status_t analyze_flecsi_task(const flecstan::Yaml &yaml)
{
   exit_status_t status = exit_clean;

   // Consolidate registration and execution information
   std::multimap<std::string,treg> regs; // registrations
   std::multimap<std::string,texe> exes; // executions

   #define flecstan_insert(mmap,mac) \
      for (auto val : yaml.mac.matched) \
         mmap.insert(std::pair(val.hash,val));

   flecstan_insert(regs, flecsi_register_task_simple);
   flecstan_insert(regs, flecsi_register_task);
   flecstan_insert(regs, flecsi_register_mpi_task_simple);
   flecstan_insert(regs, flecsi_register_mpi_task);

   flecstan_insert(exes, flecsi_execute_task_simple);
   flecstan_insert(exes, flecsi_execute_task);
   flecstan_insert(exes, flecsi_execute_mpi_task_simple);
   flecstan_insert(exes, flecsi_execute_mpi_task);
   flecstan_insert(exes, flecsi_execute_reduction_task);

   #undef flecstan_insert

   // Duplicate registration?
   status = std::max(status, task_reg_dup(regs));

   // Registration without execution?
   status = std::max(status, task_reg_without_exe(regs,exes));

   // Execution without registration?
   status = std::max(status, task_exe_without_reg(regs,exes));

   // done
   return status;
}



// -----------------------------------------------------------------------------
// Function interface
// -----------------------------------------------------------------------------

// Helper: Duplicate registrations?
// These are warnings
static exit_status_t function_reg_dup(
   const std::multimap<std::string, flecsi_register_function> &regs
) {
   exit_status_t status = exit_clean;

   for (auto a = regs.cbegin(), b = a;  a != regs.cend();  a = b) {
      bool unique = true;
      std::string str;

      while (++b != regs.cend() && b->first == a->first) {
         if (unique) {
            // then no longer; begin diagnostic
            str = "Hash string \"" + a->first + "\" was created "
                  "by multiple function registrations:\n   " +
                   flcc(a->second) + "\n";
            unique = false;
         }
         // (next) duplicate
         str += "   " + flcc(b->second) + "\n";

         // for summary
         summary_function_reg_dup +=
            "(" + flcc(b->second,false) + ") dups "
            "(" + flcc(a->second,false) + ")\n";
      }

      if (!unique) {
         // finish diagnostic
         str +=
           "This may have caused a (possibly cryptic) compile-time error.\n"
           "If not, a duplicate hash will still trigger a run-time error.\n"
           "Check your function registrations for incorrect task and/or "
           "namespace names.";
         status = error(str);
      }
   }

   return status;
}



// Helper: Registrations without handle retrievals?
// These are warnings
static exit_status_t function_reg_without_hand(
   const std::multimap<std::string, flecsi_register_function> &regs,
   const std::multimap<std::string, flecsi_function_handle  > &hands
) {
   exit_status_t status = exit_clean;

   for (auto reg : regs)
      if (hands.find(reg.first) == hands.end()) {
         status = warning(
           "The function registered with hash \"" +
            reg.first + "\" here:\n   " + flcc(reg.second) + "\n"
           "never has its handle retrieved "
              "with a flecsi_function_handle() macro call.\n"
           "Is this intentional?"
         );
         summary_function_reg_without_hand += flcc(reg.second) + "\n";
      }

   return status;
}



// Helper: Handle retrievals without registrations?
// These are errors
static exit_status_t function_hand_without_reg(
   const std::multimap<std::string,flecsi_register_function> &regs,
   const std::multimap<std::string,flecsi_function_handle  > &hands
) {
   exit_status_t status = exit_clean;

   for (auto hand : hands)
      if (regs.find(hand.first) == regs.end()) {
         status = error(
           "The function whose handle is retrieved with hash \"" +
            hand.first + "\" here:\n   " + flcc(hand.second,false) + "\n"
           "was not registered with a flecsi_register_function() macro call,\n"
           "or was not registered with that hash.\n"
           "This will trigger a run-time error if this line is reached."
         );
         summary_function_hand_without_reg += flcc(hand.second,false) + "\n";
      }

   return status;
}



// analyze_flecsi_function
static exit_status_t analyze_flecsi_function(const flecstan::Yaml &yaml)
{
   exit_status_t status = exit_clean;

   // Registration and handle information
   std::multimap<std::string, flecsi_register_function> regs;
   std::multimap<std::string, flecsi_function_handle  > hands;

   for (auto val : yaml.flecsi_register_function.matched)
      regs.insert (std::pair(val.hash, val));
   for (auto val : yaml.flecsi_function_handle  .matched)
      hands.insert(std::pair(val.hash, val));

   // Duplicate registration?
   status = std::max(status, function_reg_dup(regs));

   // Registration without handle retrieval?
   status = std::max(status, function_reg_without_hand(regs,hands));

   // Handle retrieval without registration?
   status = std::max(status, function_hand_without_reg(regs,hands));

   // done
   return status;
}



// -----------------------------------------------------------------------------
// analysis
// -----------------------------------------------------------------------------

exit_status_t analysis(const flecstan::Yaml &yaml)
{
   debug("analysis()");
   exit_status_t status = exit_clean;

   // ------------------------
   // Per-macro
   // ------------------------

   // expander macro ==> create calls to the above analyze_* functions
   #define flecstan_analyze(fun) \
      status = std::max(status,analyze_##fun(yaml))
   flecstan_expand(flecstan_analyze,;)
   #undef flecstan_analyze

   // ------------------------
   // Combined
   // ------------------------

   // task registration/execution
   status = std::max(status,analyze_flecsi_task(yaml));

   // function interface
   status = std::max(status,analyze_flecsi_function(yaml));

   // done
   return status;
}

} // namespace flecstan
