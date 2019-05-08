/* -*- C++ -*- */

#include "flecstan-analysis.h"
#include <map>

namespace flecstan {

/*
Contents of each of our classes that correspond to FleCSI macros:

vector<MacroCall>: called
   string                unit
   string                macname
   FileLineColumn        location
   FileLineColumn        spelling
   bool                  ast
   vector<vector<Token>> argstok
   vector<string>        argsraw

vector<macro>: matched
   From flecsi_base:
      string         unit
      FileLineColumn location
      FileLineColumn spelling
      vector<string> scope
   Plus macro-specific data; see below
*/



// -----------------------------------------------------------------------------
// Helper constructs
// -----------------------------------------------------------------------------

// flecstan_im
// Helper macro. Pulls out yaml.<field>.[called,matched], where <field>
// represents some FleCSI macro. We're calling this macro at the start
// of many of our functions below, to do things that those functions all
// need to do. Basically, this macro helps makes the code more compact.
#define flecstan_im(field) \
   const auto &call = yaml.field.called; \
   const auto &mat  = yaml.field.matched; \
   (void)call; /* for now, to silence any possible "unused" warnings */ \
   (void)mat;  /* ditto */ \
   exit_status_t status = exit_clean; \
   status = std::max(status, called_matched(call, mat, #field))

// stringify
#define _stringify(macro) #macro
#define  stringify(macro) _stringify(macro)



// uflcs
// For printing: {unit,file,line,column[,scope]} information.
// Dumb name, but we're just calling this internally.
std::string uflcs(const flecsi_base &base, const bool print_scope)
{
   std::ostringstream oss;
   oss << base.unit << ": "
       << print_flc(
     "file ", ", line ", ", column ",
      base.location, base.spelling
   );

   // scope
   if (print_scope) {
      std::string scp;  std::size_t count = 0;
      for (auto str : base.scope)
         scp += (count++ < 2 ? "" : "::") + str;
      oss << ", scope " << scp;
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

static const std::string basic_macro_multiple =
  "This is not a problem, unless it caused compilation or linking errors.\n"
  "However, this usage goes against the intention of the macro.";



// ...string program
static exit_status_t
analyze_flecsi_register_program(const flecstan::Yaml &yaml)
{
   #define macro flecsi_register_program
   flecstan_im(macro);

   // The following seem reasonable to diagnose:
   //    - Multiple calls in one C++ file. (Multiple calls across files
   //      is entirely possible, and fine, given that the macro inserts
   //      an inline variable).
   //    - Multiple calls (anywhere) with different parameters.
   // I won't worry about the fact that the macro does not, at the time
   // of this writing, actually use its parameter.

   // Size, for use below
   const std::size_t size = mat.size();

   // ------------------------
   // Multiple calls in
   // one C++ file?
   // ------------------------

   // .unit is the C++ file
   std::vector<macro> tmp = mat;
   sort(tmp.begin(), tmp.end(),
     [](const macro &a, const macro &b) -> bool { return a.unit < b.unit; });

   for (std::size_t i = 0, j = i;  i < size;  i = j) {
      bool unique = true; // so far
      std::string str;

      while (++j < size && tmp[j].unit == tmp[i].unit) {
         if (unique)
            // then no longer; begin diagnostic
            str = "Macro " stringify(macro) " is called more than once in or "
                  "through " + tmp[i].unit + ".\n   " + uflcs(tmp[i]) + "\n";
         unique = false;

         // (next) duplicate
         str += "   " + uflcs(tmp[j]) + "\n";
      }

      // finish and print diagnostic
      if (!unique)
         status = warning(str += basic_macro_multiple);
   }

   // ------------------------
   // Multiple calls with
   // different parameters?
   // ------------------------

   // .program is the parameter
   bool unique = true; // so far
   for (std::size_t i = 1;  i < size;  ++i)
      if (mat[i].program != mat[i-1].program)
         { unique = false; break; }

   // build and print diagnostic
   if (!unique) {
      std::string str =
         "Macro " stringify(macro) " is called with different parameters.\n";
      for (std::size_t i = 0;  i < size;  ++i)
         str += "   With (" + mat[i].program + "), in " + uflcs(mat[i]) + "\n";
      status = warning(str += basic_macro_multiple);
   }

   // finish
   return status;
   #undef macro
}



// ...string driver
static exit_status_t
analyze_flecsi_register_top_level_driver(const flecstan::Yaml &yaml)
{
   #define macro flecsi_register_top_level_driver
   flecstan_im(macro);

   // For now, we'll find the same issues as we did for flecsi_register_program;
   // see above. Note that the present macro, unlike the above one, does use its
   // parameter. Multiple uses with different parameters would no longer be C++
   // errors, but are presumably still contrary to the macro's intention. That
   // the parameter is of type int(int, char **) could also be checked, but I'll
   // skip that for now; the compiler would find that problem, albeit without as
   // good a diagnostic as we could provide.

   // Size, for use below
   const std::size_t size = mat.size();

   // ------------------------
   // Multiple calls in
   // one C++ file?
   // ------------------------

   // .unit is the C++ file
   std::vector<macro> tmp = mat;
   sort(tmp.begin(), tmp.end(),
     [](const macro &a, const macro &b) -> bool { return a.unit < b.unit; });

   for (std::size_t i = 0, j = i;  i < size;  i = j) {
      bool unique = true; // so far
      std::string str;

      while (++j < size && tmp[j].unit == tmp[i].unit) {
         if (unique)
            // then no longer; begin diagnostic
            str = "Macro " stringify(macro) " is called more than once in or "
                  "through " + tmp[i].unit + ".\n   " + uflcs(tmp[i]) + "\n";
         unique = false;

         // (next) duplicate
         str += "   " + uflcs(tmp[j]) + "\n";
      }

      // finish and print diagnostic
      if (!unique)
         status = warning(str += basic_macro_multiple);
   }

   // ------------------------
   // Multiple calls with
   // different parameters?
   // ------------------------

   // .driver is the parameter
   bool unique = true; // so far
   for (std::size_t i = 1;  i < size;  ++i)
      if (mat[i].driver != mat[i-1].driver)
         { unique = false; break; }

   // build and print diagnostic
   if (!unique) {
      std::string str =
         "Macro " stringify(macro) " is called with different parameters.\n";
      for (std::size_t i = 0;  i < size;  ++i)
         str += "   With (" + mat[i].driver + "), in " + uflcs(mat[i]) + "\n";
      status = warning(str += basic_macro_multiple);
   }

   // finish
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
   flecstan_im(flecsi_color);
   // I'm not aware of any particular checks we need for this.
   return status;
}



// ...no additional parameters
static exit_status_t
analyze_flecsi_colors(const flecstan::Yaml &yaml)
{
   flecstan_im(flecsi_colors);
   // I'm not aware of any particular checks we need for this.
   return status;
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
// Function interface
//    flecsi_register_function
//    flecsi_execute_function
//    flecsi_function_handle
//    flecsi_define_function_type
// We don't do much with these individually, but we look at them together,
// as a whole, in analyze_flecsi_function() below.
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
// We don't do much with these individually, but we look at them together,
// as a whole, in analyze_flecsi_task() below.
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
// Task registration/execution, as a whole
// -----------------------------------------------------------------------------

/*
Elsewhere in our analysis, we examined individual FleCSI macros that deal with
task registration and execution. Here, we'd like to examine task registration
and execution as a whole. Registration, for example, can be done with any of
four macros, all in principle with the same result: a task being associated
with a hash. Task execution, likewise, can be done with any of five different
macros. We'd like to look at registrations in the aggregate, executions in the
aggregate, and the relationship between registrations and executions.

Overall, we'll be looking for three fundamental problems:

   - Duplicate registrations (==> errors)
   - Registrations without executions (==> warnings)
   - Executions without registrations (==> errors)

The last two of these items are self-explanatory, and easy to handle, so we'll
focus this comment the first item: duplicates.

When a task registration macro is called, the macro defines a wrapper function
for the given task, then defines a bool variable whose initialization triggers
registration of the task wrapper function. Originally, the bool variable was
just, well, a regular bool.

A recent (at the time of this writing) change was for the macros to instead
define the bool as an *inline* variable. Because of this, certain macro uses
that would once have produced duplicate registrations (say, #including in many
C++ files a header that invoked a task registration macro) are no longer
necessarily problematic, because C++ arranges for the initialization of an
inline variable to be performed exactly once.

So, we're looking for "duplication" that's still problematic. What qualifies?
Well, macros are inherently messy, and we may not be able to catch all possible
duplication-related misuses, but let's try to think of what we can do.

Our fundamental goal here is to prevent a FleCSI runtime error due to one hash
attempting to key to multiple tasks - the moral equivalent of trying to give a
std::map one key with multiple values. It's possible, even with the inline bool,
to use the macros in such a way that this happens, *without* it being caught at
compile time. A secondary goal might be to catch cases in which the same inline
variable is set up more than once, in the same scope, in one C++ file - which
isn't allowed by C++, even if the variable is inline. Although the compiler will
produce an error in this case, we may be able to diagnose the problem ourselves
and provide a clearer diagnostic than the compiler will.

In *one* C++ file: If the same hash appears multiple times, then either the
inline variable is repeated same-scope in the file (which isn't allowed by C++),
or it is repeated in different scopes (which leads to the original pre-inline-
variable problem of multiple initializations, and isn't allowed by FleCSI).

In *different* C++ files: If we see the same hash, then we should require the
same scope. This case arises if, for instance, someone #includes a header file
with a macro call into more than one C++ file. This shouldn't at present lead
to any run-time problems, with the macro's inserted variable now being inline.
The same hash in a different scope, however, would bring back the FleCSI run-
time error of multiple registrations.  I think we'll only need this same-hash,
different-scope detection in the context of two different files; in the same-
hash case in the same file, we'd have already scanned for an error as outlined
in the previous paragraph, independent of whether the scope was different or
the same.

SKETCH
------

For brevity, let register() denote any of FleCSI's task registration macros,
and say that we have three tasks (really, hash strings): x, y, and z. Consider
two files, a.cc and b.cc, comprises the total code our analyzer is to evaluate.

a.cc

   register(x)     // 1. Fine
   register(x)     // 2. C++ error: compile-time conflict with (1)
   register(y)     // 3. Fine
   namespace {
      register(y)  // 4. FleCSI error: run-time conflict with (3)
   }
   register(z))    // 5. Fine

b.cc

   register(x)     // 6. Fine; it's in a different file from (1)
   namespace {
      register(z)) // 7. FleCSI error: run-time conflict with (5)
   }

Note that for the purposes of the analysis we've described, the ordering of the
C++ files doesn't fundamentally matter - we might look at a.cc before b.cc,
or vice versa. The only effect of file reordering is that the diagnostics might
present themselves in a different way - for example, (5) being an error relative
to (7), rather than the other way around.
*/



static bool same_scope(const flecsi_base &a, const flecsi_base &b)
{
   // Check that the macro calls represented by the two parameters have the same
   // scope. This requires the same size and same individual values, as well as
   // NO unnamed namespaces. The last part is true because we're only calling
   // this function if the two macro calls in question are in different files,
   // and unnamed namespace scope in two different files is different, even if
   // the scopes otherwise appear to be the same.

   if (a.scope.size() != b.scope.size())
      return false;

   for (std::size_t i = a.scope.size();  i-- ; )
      if (a.scope[i] == unnamed_namespace ||
          b.scope[i] == unnamed_namespace ||
          a.scope[i] != b.scope[i])
         return false;

   return true;
}



// Helper: Duplicate task registrations?
// These are errors
static exit_status_t task_reg_dup(
   const std::multimap<std::string,treg> &regs
) {
   exit_status_t status = exit_clean;

   // Recall that class treg (task registration helper class, meant to contain
   // information from *any* of the individual task registration classes) has:
   // unit, location, spelling, scope, task, nspace, processor, launch, hash.

   // All task registrations together, in vector form
   std::vector<treg> tmp;
   for (auto r : regs)
      tmp.push_back(r.second);
   const std::size_t size = tmp.size();

   // Order by:
   //    hash (primary)
   //    unit (secondary)
   sort(
      tmp.begin(),
      tmp.end(),
      [](const treg &a, const treg &b) -> bool
      {
         return
            a.hash <  b.hash || (
            a.hash == b.hash && (
            a.unit <  b.unit
         ));
      }
   );


   // ------------------------
   // Duplicate same-file
   // scope-independent hashes
   // Cases (2) and (4) above
   // ------------------------

   // Note: right now, we're actually *not* detecting example case (2) here,
   // for the following reason. That case results in a compilation error, which
   // in turn means that none of the code constructs from (2) end up in the AST!
   // The data we're looking at here (in tmp, based on parameter regs, based on
   // data from the *AST-matched* parts of the invoked task-registration FleCSI
   // macros) thus don't contain anything about (2). That's fine; the compiler
   // will already have produced an error. Perhaps, however, we could eventually
   // base the present code on the macro invocations themselves, not on their
   // resulting (or not!) AST constructs, and thereby detect the same C++ error
   // in a form that we can report more clearly to the user.

   for (std::size_t i = 0, j = 0;  i < size;  i = j) {
      std::string str;

      while (
         ++j < size &&
         tmp[j].hash == tmp[i].hash &&
         tmp[j].unit == tmp[i].unit
      ) {
         // begin diagnostic
         if (str == "")
            str = "Hash string \"" + tmp[i].hash + "\" was created by multiple "
                  "task registrations\nin one file:\n   " +
                   uflcs(tmp[i]) + "\n";

         // (next) duplicate
         str += "   " + uflcs(tmp[j]) + "\n";

         // for summary
         summary_task_reg_dup +=
            "(" + uflcs(tmp[j],false) + ") dups "
            "(" + uflcs(tmp[i],false) + ")\n";
      }

      if (str != "")
         // finish diagnostic
         status = error(str +=
           "This may have caused a (possibly cryptic) compile-time error.\n"
           "If it didn't, a duplicate hash will still trigger a run-time error."
         );
   }


   // ------------------------
   // Duplicate across-file
   // different-scoped hashes
   // Case (7) above
   // ------------------------

   for (std::size_t i = 0, j = 0;  i < size;  i = j) {
      std::string str;

      while (
         ++j < size &&
         tmp[j].hash == tmp[i].hash
      ) {
         if (tmp[j].unit != tmp[i].unit && !same_scope(tmp[j],tmp[i])) {
            // begin diagnostic
            if (str == "")
               str = "Hash string \"" + tmp[i].hash + "\" was created "
                     "by multiple task registrations\n"
                     "in different scopes in different files:\n   " +
                      uflcs(tmp[i]) + "\n";

            // (next) duplicate
            str += "   " + uflcs(tmp[j]) + "\n";

            // for summary
            summary_task_reg_dup +=
               "(" + uflcs(tmp[j],false) + ") dups "
               "(" + uflcs(tmp[i],false) + ")\n";
         }
      }

      if (str != "")
         // finish diagnostic
         status = error(
            str += "A duplicate hash will trigger a run-time error.");
   }

   return status;
}



// Helper: Task registrations without executions?
// These are warnings
static exit_status_t task_reg_without_exe(
   const std::multimap<std::string,treg> &regs,
   const std::multimap<std::string,texe> &exes
) {
   exit_status_t status = exit_clean;

   for (auto reg : regs)
      if (exes.find(reg.first) == exes.end()) {
         status = warning(
           "The task registered with hash \"" +
            reg.first + "\" here:\n   " + uflcs(reg.second) + "\n"
           "is never invoked with any of FleCSI's task execution macros.\n"
           "Is this intentional?"
         );
         summary_task_reg_without_exe += uflcs(reg.second,false) + "\n";
      }

   return status;
}



// Helper: Task executions without registrations?
// These are errors
static exit_status_t task_exe_without_reg(
   const std::multimap<std::string,treg> &regs,
   const std::multimap<std::string,texe> &exes
) {
   exit_status_t status = exit_clean;

   for (auto exe : exes)
      if (regs.find(exe.first) == regs.end()) {
         status = error(
           "The task executed with hash \"" +
            exe.first + "\" here:\n   " + uflcs(exe.second,false) + "\n"
           "was not registered with any of FleCSI's task registration macros,\n"
           "or was not registered with that hash.\n"
           "This will trigger a run-time error if this line is reached."
         );
         summary_task_exe_without_reg += uflcs(exe.second,false) + "\n";
      }

   return status;
}



// analyze_flecsi_task
static exit_status_t analyze_flecsi_task(const flecstan::Yaml &yaml)
{
   exit_status_t status = exit_clean;

   // Registration and execution information
   // Consolidated across macro variants
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

   // Duplicate registrations?
   // Registrations without executions?
   // Executions without registrations?
   status = std::max(status, task_reg_dup(regs));
   status = std::max(status, task_reg_without_exe(regs,exes));
   status = std::max(status, task_exe_without_reg(regs,exes));

   // done
   return status;
}



// -----------------------------------------------------------------------------
// Function interface, as a whole
// Similar overall to the task registration/execution analysis above;
// see the large comment there for more information.
// -----------------------------------------------------------------------------

// Helper: Duplicate function registrations?
// These are errors
static exit_status_t function_reg_dup(
   const std::multimap<std::string, flecsi_register_function> &regs
) {
   exit_status_t status = exit_clean;

   // flecsi_register_function has:
   // unit, location, spelling, scope, func, nspace, hash

   // fixme
   // Much of the following is similar enough to the task registration material
   // that the two could, with a bit of work, be combined into common code.

   // Function registrations, in vector form
   std::vector<flecsi_register_function> tmp;
   for (auto r : regs)
      tmp.push_back(r.second);
   const std::size_t size = tmp.size();

   // Order by:
   //    hash (primary)
   //    unit (secondary)
   sort(
      tmp.begin(),
      tmp.end(),
      [](
         const flecsi_register_function &a,
         const flecsi_register_function &b
      ) -> bool {
         return
            a.hash <  b.hash || (
            a.hash == b.hash && (
            a.unit <  b.unit
         ));
      }
   );


   // ------------------------
   // Duplicate same-file
   // scope-independent hashes
   // ------------------------

   for (std::size_t i = 0, j = 0;  i < size;  i = j) {
      std::string str;

      while (
         ++j < size &&
         tmp[j].hash == tmp[i].hash &&
         tmp[j].unit == tmp[i].unit
      ) {
         // begin diagnostic
         if (str == "")
            str = "Hash string \"" + tmp[i].hash + "\" was created by multiple "
                  "function registrations\nin one file:\n   " +
                   uflcs(tmp[i]) + "\n";

         // (next) duplicate
         str += "   " + uflcs(tmp[j]) + "\n";

         // for summary
         summary_function_reg_dup +=
            "(" + uflcs(tmp[j],false) + ") dups "
            "(" + uflcs(tmp[i],false) + ")\n";
      }

      if (str != "")
         // finish diagnostic
         status = error(str +=
           "This may have caused a (possibly cryptic) compile-time error.\n"
           "If it didn't, a duplicate hash will still trigger a run-time error."
         );
   }


   // ------------------------
   // Duplicate across-file
   // different-scoped hashes
   // ------------------------

   for (std::size_t i = 0, j = 0;  i < size;  i = j) {
      std::string str;

      while (
         ++j < size &&
         tmp[j].hash == tmp[i].hash
      ) {
         if (tmp[j].unit != tmp[i].unit && !same_scope(tmp[j],tmp[i])) {
            // begin diagnostic
            if (str == "")
               str = "Hash string \"" + tmp[i].hash + "\" was created "
                     "by multiple function registrations\n"
                     "in different scopes in different files:\n   " +
                      uflcs(tmp[i]) + "\n";

            // (next) duplicate
            str += "   " + uflcs(tmp[j]) + "\n";

            // for summary
            summary_function_reg_dup +=
               "(" + uflcs(tmp[j],false) + ") dups "
               "(" + uflcs(tmp[i],false) + ")\n";
         }
      }

      if (str != "")
         // finish diagnostic
         status = error(
            str += "A duplicate hash will trigger a run-time error.");
   }

   return status;
}



// Helper: Function registrations without handle retrievals?
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
            reg.first + "\" here:\n   " + uflcs(reg.second) + "\n"
           "never has its handle retrieved "
              "with a flecsi_function_handle() macro call.\n"
           "Is this intentional?"
         );
         summary_function_reg_without_hand += uflcs(reg.second,false) + "\n";
      }

   return status;
}



// Helper: Handle retrievals without function registrations?
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
            hand.first + "\" here:\n   " + uflcs(hand.second,false) + "\n"
           "was not registered with a flecsi_register_function() macro call,\n"
           "or was not registered with that hash.\n"
           "This will trigger a run-time error if this line is reached."
         );
         summary_function_hand_without_reg += uflcs(hand.second,false) + "\n";
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
      regs .insert(std::pair(val.hash,val));
   for (auto val : yaml.flecsi_function_handle  .matched)
      hands.insert(std::pair(val.hash,val));

   // Duplicate registrations?
   // Registrations without handle retrievals?
   // Handle retrievals without registrations?
   status = std::max(status, function_reg_dup(regs));
   status = std::max(status, function_reg_without_hand(regs,hands));
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

   // expander macro ==> create calls to most of the above analyze_* functions
   #define flecstan_analyze(fun) status = std::max(status,analyze_##fun(yaml))
   flecstan_expand(flecstan_analyze,;)
   #undef flecstan_analyze


   // ------------------------
   // Combined
   // ------------------------

   // task registration/execution
   status = std::max(status,analyze_flecsi_task(yaml));

   // function interface
   status = std::max(status,analyze_flecsi_function(yaml));


   // ------------------------
   // Synopsis
   // ------------------------

   /*
   // probably not; too much color may become distracting...
   const std::string estr = num_error == 0 ? "" :
     (emit_color ? color::error   : "") +
      std::to_string(num_error) + " error"   + (num_error == 1 ? "" : "s") +
     (emit_color ? color::report2 : "");

   const std::string wstr = num_warn  == 0 ? "" :
     (emit_color ? color::warning : "") +
      std::to_string(num_warn ) + " warning" + (num_warn  == 1 ? "" : "s") +
     (emit_color ? color::report2 : "");
   */

   const std::string estr = num_error == 0 ? "" :
      std::to_string(num_error) + " error"   + (num_error == 1 ? "" : "s");
   const std::string wstr = num_warn  == 0 ? "" :
      std::to_string(num_warn ) + " warning" + (num_warn  == 1 ? "" : "s");

   report(
      "Synopsis",
       num_error && num_warn ? estr + "\n" + wstr
    :  num_error             ? estr
    :               num_warn ? wstr
    : "No errors or warnings were detected."
   );


   // ------------------------
   // Finish
   // ------------------------

   return status;
}

} // namespace flecstan
