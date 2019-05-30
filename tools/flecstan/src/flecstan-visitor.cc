/* -*- C++ -*- */

#include "flecstan-visitor.h"
#include "flecstan-utility.h"

namespace flecstan {

/*
-----------------------------------------------------------------
   Class for YAML                        Where processed below
-----------------------------------------------------------------
   flecsi_register_program               VisitVarDecl()
   flecsi_register_top_level_driver      VisitVarDecl()
-----------------------------------------------------------------
   flecsi_register_global_object         VisitCallExpr()
   flecsi_set_global_object              VisitCallExpr()
   flecsi_initialize_global_object       VisitCallExpr()
   flecsi_get_global_object              VisitCallExpr()
-----------------------------------------------------------------
   flecsi_register_task_simple           VisitVarDecl()
   flecsi_register_task                  VisitVarDecl()
   flecsi_register_mpi_task_simple       VisitVarDecl()
   flecsi_register_mpi_task              VisitVarDecl()
-----------------------------------------------------------------
   flecsi_color                          VisitCallExpr()
   flecsi_colors                         VisitCallExpr()
-----------------------------------------------------------------
   flecsi_execute_task_simple            VisitCallExpr()
   flecsi_execute_task                   VisitCallExpr()
   flecsi_execute_mpi_task_simple        VisitCallExpr()
   flecsi_execute_mpi_task               VisitCallExpr()
   flecsi_execute_reduction_task         VisitCallExpr()
-----------------------------------------------------------------
   flecsi_register_reduction_operation   VisitVarDecl()
-----------------------------------------------------------------
   flecsi_register_function              VisitVarDecl()
   flecsi_execute_function               VisitCallExpr()
   flecsi_function_handle                VisitCallExpr()
   flecsi_define_function_type           VisitTypeAliasDecl()
-----------------------------------------------------------------
   flecsi_register_data_client           VisitVarDecl()
   flecsi_register_field                 VisitVarDecl()
   flecsi_register_global                VisitVarDecl()
   flecsi_register_color                 VisitVarDecl()
-----------------------------------------------------------------
   flecsi_get_handle                     VisitCallExpr()
   flecsi_get_client_handle              VisitCallExpr()
   flecsi_get_handles                    VisitCallExpr()
   flecsi_get_handles_all                VisitCallExpr()
-----------------------------------------------------------------
   flecsi_get_global                     VisitCallExpr()
   flecsi_get_color                      VisitCallExpr()
   flecsi_get_mutator                    VisitCallExpr()
-----------------------------------------------------------------
*/

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

// match
static bool
match(const clang::CallExpr *& call, // output
  const clang::Expr * const expr,
  const MacroInvocation & mi,
  const std::string & macro_want,
  const std::string & theclass = "",
  const std::string & thefunction = "",
  int min = -1,
  int max = -1) {
  call = nullptr;
  if(mi.name != macro_want)
    return false;

  if(theclass == "" && thefunction == "") {
    mi.ast = true;
    return true;
  }

  if(min == -1 && max == -1) {
    min = std::numeric_limits<int>::min();
    max = std::numeric_limits<int>::max();
  }
  else if(max == -1)
    max = min;

  call = getClassCall(expr, theclass, thefunction, min, max);
  if(call) {
    mi.ast = true;
    report("Link", "Function call: " + theclass + "::" + thefunction +
                     "\n"
                     "Matches macro: " +
                     mi.name + " (" + mi.flc() + ")");
  }
  return call;
}

// strarg
static std::string
strarg(const clang::CallExpr * const call, const std::size_t n) {
  if(!call)
    return intwarn("strarg(): call == nullptr."), "";
  auto expr = call->getArg(n);
  if(!expr)
    return intwarn("strarg(): expr == nullptr."), "";
  auto bind = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr);
  if(!bind)
    return intwarn("strarg(): bind == nullptr."), "";
  auto conx = clang::dyn_cast<clang::CXXConstructExpr>(bind->getSubExpr());
  if(!conx)
    return intwarn("strarg(): conx == nullptr."), "";
  auto cast = clang::dyn_cast<clang::ImplicitCastExpr>(conx->getArg(0));
  if(!cast)
    return intwarn("strarg(): cast == nullptr."), "";
  auto strx = clang::dyn_cast<clang::StringLiteral>(cast->getSubExpr());
  if(!strx)
    return intwarn("strarg(): strx == nullptr."), "";

  return strx->getString().str();
}

// nscontext
static void
nscontext(const clang::Decl * const decl, std::vector<std::string> & ctx) {
  for(const clang::DeclContext * dc = decl->getDeclContext(); dc;
      dc = dc->getParent()) {
    // namespace?
    auto ns = clang::dyn_cast<clang::NamespaceDecl>(dc);
    if(ns)
      ctx.push_back(ns->getNameAsString());
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Visit*
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// For these functions, the bool return value tells Clang's AST visitor (the
// thing that's calling these functions) whether or not it should continue the
// AST traversal. For a typical code analysis tool, we'd anticipate that just
// returning true, always, is probably the right thing to do, absent some sort
// of terminal error.

// -----------------------------------------------------------------------------
// VisitVarDecl
// -----------------------------------------------------------------------------

// flecsi_register_program
// flecsi_register_top_level_driver
// flecsi_register_task_simple
// flecsi_register_task
// flecsi_register_mpi_task_simple
// flecsi_register_mpi_task
// flecsi_register_reduction_operation
// flecsi_register_function
// flecsi_register_data_client
// flecsi_register_field
// flecsi_register_global
// flecsi_register_color

bool
Visitor::VisitVarDecl(const clang::VarDecl * const var) {
  debug("Visitor::VisitVarDecl()");
  flecstan_debug(var->getNameAsString());

  // associated macro?
  const MacroInvocation * const iptr = prep.invocation(var->getLocStart());
  if(!iptr)
    return true;
  const MacroInvocation & mi = *iptr;

  // for use below
  const clang::Expr * const expr = var->getInit();
  std::size_t pos = 0;
  const clang::CallExpr * call; // tbd

  // context
  std::vector<std::string> ctx;
  nscontext(var, ctx);

  // ------------------------
  // Top-level driver
  // interface
  // ------------------------

  // flecsi_register_program(program)
  if(match(call, expr, mi, "flecsi_register_program")) {
    flecsi_register_program c(mi);
    c.program = mi.str(sema, pos++);
    yaml.push(c, ctx);
  }

  // flecsi_register_top_level_driver(driver)
  if(match(call, expr, mi, "flecsi_register_top_level_driver")) {
    flecsi_register_top_level_driver c(mi);
    c.driver = mi.str(sema, pos++);
    yaml.push(c, ctx);
  }

  // ------------------------
  // Task registration
  // interface
  // ------------------------

  // flecsi_register_task_simple(task, processor, launch)
  if(match(call, expr, mi, "flecsi_register_task_simple",
       "flecsi::execution::task_interface_u", "register_task", 3)) {
    flecsi_register_task_simple c(mi);
    c.task = mi.str(sema, pos++);
    c.processor = mi.str(sema, pos++);
    c.launch = mi.str(sema, pos++);
    c.hash = strarg(call, 2);
    yaml.push(c, ctx);
  }

  // flecsi_register_task(task, nspace, processor, launch)
  if(match(call, expr, mi, "flecsi_register_task",
       "flecsi::execution::task_interface_u", "register_task", 3)) {
    flecsi_register_task c(mi);
    c.task = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.processor = mi.str(sema, pos++);
    c.launch = mi.str(sema, pos++);
    c.hash = strarg(call, 2);
    yaml.push(c, ctx);
  }

  // flecsi_register_mpi_task_simple(task)
  if(match(call, expr, mi, "flecsi_register_mpi_task_simple",
       "flecsi::execution::task_interface_u", "register_task", 3)) {
    flecsi_register_mpi_task_simple c(mi);
    c.task = mi.str(sema, pos++);
    c.hash = strarg(call, 2);
    yaml.push(c, ctx);
  }

  // flecsi_register_mpi_task(task, nspace)
  if(match(call, expr, mi, "flecsi_register_mpi_task",
       "flecsi::execution::task_interface_u", "register_task", 3)) {
    flecsi_register_mpi_task c(mi);
    c.task = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.hash = strarg(call, 2);
    yaml.push(c, ctx);
  }

  // ------------------------
  // Reduction
  // interface
  // ------------------------

  // flecsi_register_reduction_operation(name, operation_type)
  if(match(call, expr, mi, "flecsi_register_reduction_operation",
       "flecsi::execution::task_interface_u", "register_reduction_operation",
       0)) {
    flecsi_register_reduction_operation c(mi);
    c.name = mi.str(sema, pos++);
    c.operation_type = mi.str(sema, pos++);
    yaml.push(c, ctx);
  }

  // ------------------------
  // Function
  // interface
  // ------------------------

  // flecsi_register_function(func, nspace)
  if(match(call, expr, mi, "flecsi_register_function",
       "flecsi::execution::function_interface_u", "register_function", 0)) {
    flecsi_register_function c(mi);
    c.func = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.hash = c.nspace + "::" + c.func;
    yaml.push(c, ctx);
  }

  // flecsi_execute_function(handle, ...)
  // See VisitCallExpr()

  // flecsi_function_handle(func, nspace)
  // See VisitCallExpr()

  // flecsi_define_function_type(func, return_type, ...)
  // See VisitTypeAliasDecl()

  // ------------------------
  // Registration
  // ------------------------

  // flecsi_register_data_client(client_type, nspace, name)
  if(match(call, expr, mi, "flecsi_register_data_client",
       "flecsi::data::data_client_interface_u", "register_data_client", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(call);
    const clang::QualType qt = getTypeArg(ta, 0);
    const clang::CXXRecordDecl * const cd = getClassDecl(qt);
    flecsi_register_data_client c(mi);
    c.client_type = mi.str(sema, pos++);
    c.data_client_t = cd && isDerivedFrom(cd, "flecsi::data::data_client_t");
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    yaml.push(c, ctx);
  }

  // flecsi_register_field(client_type, nspace, name, data_type,
  //                       storage_class, versions, ...)
  if(match(call, expr, mi, "flecsi_register_field",
       "flecsi::data::field_interface_u", "register_field", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(call);
    flecsi_register_field c(mi);
    c.client_type = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.storage_class = mi.str(sema, pos++);
    c.versions = getUIntArg(ta, 5); // 5 = template argument position
    c.index_space = getUIntArg(ta, 6);
    yaml.push(c, ctx);
  }

  // flecsi_register_global(nspace, name, data_type, versions, ...)
  if(match(call, expr, mi, "flecsi_register_global",
       "flecsi::data::field_interface_u", "register_field", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(call);
    flecsi_register_global c(mi);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.versions = getUIntArg(ta, 5);
    yaml.push(c, ctx);
  }

  // flecsi_register_color(nspace, name, data_type, versions, ...)
  if(match(call, expr, mi, "flecsi_register_color",
       "flecsi::data::field_interface_u", "register_field", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(call);
    flecsi_register_color c(mi);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.versions = getUIntArg(ta, 5);
    yaml.push(c, ctx);
  }

  return true;

} // VisitVarDecl

// -----------------------------------------------------------------------------
// VisitCallExpr
// -----------------------------------------------------------------------------

// flecsi_register_global_object
// flecsi_set_global_object
// flecsi_initialize_global_object
// flecsi_get_global_object
// flecsi_color
// flecsi_colors
// flecsi_execute_task_simple
// flecsi_execute_task
// flecsi_execute_mpi_task_simple
// flecsi_execute_mpi_task
// flecsi_execute_reduction_task
// flecsi_execute_function
// flecsi_function_handle
// flecsi_get_handle
// flecsi_get_client_handle
// flecsi_get_handles
// flecsi_get_handles_all
// flecsi_get_global
// flecsi_get_color
// flecsi_get_mutator

bool
Visitor::VisitCallExpr(const clang::CallExpr * const expr) {
  debug("Visitor::VisitCallExpr()");

  // associated macro?
  const MacroInvocation * const iptr = prep.invocation(expr->getLocStart());
  if(!iptr)
    return true;
  const MacroInvocation & mi = *iptr;

  // for use below
  std::size_t pos = 0;
  const clang::CallExpr * call; // tbd

  // context
  // Doesn't work at the moment, for CallExpr
  // std::vector<std::string> ctx;
  // nscontext(expr,ctx);

  // ------------------------
  // Object registration
  // interface
  // ------------------------

  // flecsi_register_global_object(index, nspace, type)
  if(match(call, expr, mi, "flecsi_register_global_object",
       "flecsi::execution::context_u", "register_global_object")) {
    flecsi_register_global_object c(mi);
    c.index = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.type = mi.str(sema, pos++);
    yaml.push(c);
  }

  // flecsi_set_global_object(index, nspace, type, obj)
  if(match(call, expr, mi, "flecsi_set_global_object",
       "flecsi::execution::context_u", "set_global_object")) {
    flecsi_set_global_object c(mi);
    c.index = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.type = mi.str(sema, pos++);
    c.obj = mi.str(sema, pos++);
    yaml.push(c);
  }

  // flecsi_initialize_global_object(index, nspace, type, ...)
  if(match(call, expr, mi, "flecsi_initialize_global_object",
       "flecsi::execution::context_u", "initialize_global_object")) {
    flecsi_initialize_global_object c(mi);
    c.index = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.type = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs, 1);
    yaml.push(c);
  }

  // flecsi_get_global_object(index, nspace, type)
  if(match(call, expr, mi, "flecsi_get_global_object",
       "flecsi::execution::context_u", "get_global_object")) {
    flecsi_get_global_object c(mi);
    c.index = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.type = mi.str(sema, pos++);
    yaml.push(c);
  }

  // ------------------------
  // Task execution
  // interface: color
  // ------------------------

  // Remark: For flecsi_color[s], I specifically needed to have the last
  // two match() arguments, as opposed to having calls like this:
  //    match(call, expr, mi, "flecsi_color")
  //    match(call, expr, mi, "flecsi_colors")
  // That's because both of those macros, while simple, actually generate
  // two CallExprs in the AST: one to instance(), and the other to color()
  // or colors(). Without the additional check for the exact name of the
  // function being called in the "call expression" (CallExpr), each macro
  // invocation inadvertently generated two identical entries in the YAML.

  // flecsi_color
  if(match(call, expr, mi, "flecsi_color", "flecsi::execution::context_u",
       "color")) {
    flecsi_color c(mi);
    yaml.push(c);
  }

  // flecsi_colors
  if(match(call, expr, mi, "flecsi_colors", "flecsi::execution::context_u",
       "colors")) {
    flecsi_colors c(mi);
    yaml.push(c);
  }

  // ------------------------
  // Task execution
  // interface: execute
  // ------------------------

  // flecsi_execute_task_simple(task, launch, ...)
  if(match(call, expr, mi, "flecsi_execute_task_simple",
       "flecsi::execution::task_interface_u", "execute_task")) {
    flecsi_execute_task_simple c(mi);
    c.task = mi.str(sema, pos++);
    c.launch = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs);
    c.hash = c.task;
    yaml.push(c);
  }

  // flecsi_execute_task(task, nspace, launch, ...)
  if(match(call, expr, mi, "flecsi_execute_task",
       "flecsi::execution::task_interface_u", "execute_task")) {
    flecsi_execute_task c(mi);
    c.task = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.launch = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs);
    c.hash = c.nspace + "::" + c.task;
    yaml.push(c);
  }

  // flecsi_execute_mpi_task_simple(task, ...)
  if(match(call, expr, mi, "flecsi_execute_mpi_task_simple",
       "flecsi::execution::task_interface_u", "execute_task")) {
    flecsi_execute_mpi_task_simple c(mi);
    c.task = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs);
    c.hash = c.task;
    yaml.push(c);
  }

  // flecsi_execute_mpi_task(task, nspace, ...)
  if(match(call, expr, mi, "flecsi_execute_mpi_task",
       "flecsi::execution::task_interface_u", "execute_task")) {
    flecsi_execute_mpi_task c(mi);
    c.task = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs);
    c.hash = c.nspace + "::" + c.task;
    yaml.push(c);
  }

  // flecsi_execute_reduction_task(task, nspace, launch, type, datatype, ...)
  if(match(call, expr, mi, "flecsi_execute_reduction_task",
       "flecsi::execution::task_interface_u", "execute_task")) {
    flecsi_execute_reduction_task c(mi);
    c.task = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.launch = mi.str(sema, pos++);
    c.type = mi.str(sema, pos++);
    c.datatype = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs);
    c.hash = c.nspace + "::" + c.task;
    yaml.push(c);
  }

  // ------------------------
  // Function
  // interface
  // ------------------------

  // flecsi_register_function(func, nspace)
  // See VisitVarDecl()

  // flecsi_execute_function(handle, ...)
  if(match(call, expr, mi, "flecsi_execute_function",
       "flecsi::execution::function_interface_u", "execute_function")) {
    flecsi_execute_function c(mi);
    c.handle = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs, 1);
    yaml.push(c);
  }

  // flecsi_function_handle(func, nspace)
  if(match(call, expr, mi, "flecsi_function_handle",
       "flecsi::utils::const_string_t", "hash")) {
    flecsi_function_handle c(mi);
    c.func = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.hash = c.nspace + "::" + c.func;
    yaml.push(c);
  }

  // flecsi_define_function_type(func, return_type, ...)
  // See VisitTypeAliasDecl()

  // ------------------------
  // Handle-related
  // ------------------------

  // flecsi_get_handle
  if(match(call, expr, mi, "flecsi_get_handle",
       "flecsi::data::field_interface_u", "get_handle", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(expr);
    flecsi_get_handle c(mi);
    c.client_handle = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.storage_class = mi.str(sema, pos++);
    c.version = getUIntArg(ta, 5); // 5 = template argument position
    yaml.push(c);
  }

  // flecsi_get_client_handle
  if(match(call, expr, mi, "flecsi_get_client_handle",
       "flecsi::data::data_client_interface_u", "get_client_handle")) {
    flecsi_get_client_handle c(mi);
    c.client_type = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    yaml.push(c);
  }

  // flecsi_get_handles
  if(match(call, expr, mi, "flecsi_get_handles",
       "flecsi::data::field_interface_u", "get_handles", 2)) {
    flecsi_get_handles c(mi);
    c.client = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.storage_class = mi.str(sema, pos++);
    c.version = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs, 2);
    yaml.push(c);
  }

  // flecsi_get_handles_all
  if(match(call, expr, mi, "flecsi_get_handles_all",
       "flecsi::data::field_interface_u", "get_handles", 2)) {
    flecsi_get_handles_all c(mi);
    c.client = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.storage_class = mi.str(sema, pos++);
    c.version = mi.str(sema, pos++);
    getVarArgsFunction(expr, c.varargs, 2);
    yaml.push(c);
  }

  // ------------------------
  // For some flecsi_get_*
  // macros
  // ------------------------

  // flecsi_get_global
  if(match(call, expr, mi, "flecsi_get_global",
       "flecsi::data::field_interface_u", "get_handle", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(expr);
    flecsi_get_global c(mi);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.version = getUIntArg(ta, 5); // 5 = template argument position
    yaml.push(c);
  }

  // flecsi_get_color
  if(match(call, expr, mi, "flecsi_get_color",
       "flecsi::data::field_interface_u", "get_handle", 1)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(expr);
    flecsi_get_color c(mi);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.version = getUIntArg(ta, 5); // 5 = template argument position
    yaml.push(c);
  }

  // flecsi_get_mutator
  if(match(call, expr, mi, "flecsi_get_mutator",
       "flecsi::data::field_interface_u", "get_mutator", 2)) {
    const clang::TemplateArgumentList * const ta = getTemplateArgs(expr);
    flecsi_get_mutator c(mi);
    c.client_handle = mi.str(sema, pos++);
    c.nspace = mi.str(sema, pos++);
    c.name = mi.str(sema, pos++);
    c.data_type = mi.str(sema, pos++);
    c.storage_class = mi.str(sema, pos++);
    c.version = getUIntArg(ta, 5); // 5 = template argument position
    pos++; // c.version pulled via other means; skip to c.slots...
    c.slots = mi.str(sema, pos++);
    yaml.push(c);
  }

  return true;

} // VisitCallExpr

// -----------------------------------------------------------------------------
// VisitTypeAliasDecl
// -----------------------------------------------------------------------------

// flecsi_define_function_type

bool
Visitor::VisitTypeAliasDecl(const clang::TypeAliasDecl * const tad) {
  debug("Visitor::VisitTypeAliasDecl()");
  flecstan_debug(tad->getNameAsString());

  // associated macro?
  const MacroInvocation * const iptr = prep.invocation(tad->getLocStart());
  if(!iptr)
    return true;
  const MacroInvocation & mi = *iptr;

  // for use below
  std::size_t pos = 0;

  // context
  std::vector<std::string> ctx;
  nscontext(tad, ctx);

  // ------------------------
  // Function
  // interface
  // ------------------------

  // flecsi_register_function(func, nspace)
  // See VisitVarDecl()

  // flecsi_execute_function(handle, ...)
  // See VisitCallExpr()

  // flecsi_function_handle(func, nspace)
  // See VisitCallExpr()

  // flecsi_define_function_type(func, return_type, ...)
  if(mi.name == "flecsi_define_function_type") {
    std::string thetype;

    flecsi_define_function_type c(mi);
    c.func = mi.str(sema, pos++);
    c.return_type = mi.str(sema, pos++);
    getVarArgsTemplate(tad, c.varargs, thetype);
    yaml.push(c, ctx);

    report("Link", "Function type: " + tad->getNameAsString() + " = " +
                     thetype +
                     "\n"
                     "Matches macro: " +
                     mi.name + " (" + mi.flc() + ")");
  }

  return true;

} // VisitTypeAliasDecl

} // namespace flecstan
