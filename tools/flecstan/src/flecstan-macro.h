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

#include "flecstan-yaml.h"

// -----------------------------------------------------------------------------
// flecsi_base
// All of our classes for representing FleCSI's macros with YAML will have this
// class as a base. It contains information that's common to all: macro's name,
// relevant file, and position (line and column).
// -----------------------------------------------------------------------------

namespace flecstan {

class flecsi_base
{
public:
  // data
  std::string unit;
  FileLineColumn location;
  FileLineColumn spelling;
  std::vector<std::string> scope; // namespace scope

  // ctor: default
  flecsi_base() : unit(""), location("", "0", "0"), spelling("", "0", "0") {}

  // ctor: MacroCall
  // Various values are extracted from the MacroCall
  flecsi_base(const MacroCall & mc)
    : unit(mc.unit), location(mc.location), spelling(mc.spelling) {}

  // map
  // Stipulate mapRequired for all of flecsi_base's data. This will be called
  // from within MappingTraits<class>::mapping for our various classes that
  // derive from flecsi_base.
  void map(llvm::yaml::IO & io) {
    io.mapRequired("unit", unit);
    io.mapRequired("location", location);
    io.mapRequired("spelling", spelling);
    io.mapRequired("scope", scope);
  }
};

} // namespace flecstan

// -----------------------------------------------------------------------------
// Helper macros
//
// Indentation is intended to illustrate how these are supposed to be used.
// The *_done macros insert closing brackets - matching the opening brackets
// inserted by other macros - plus semicolons when appropriate. Those could
// be written directly, but doing so might confuse an editor's parens-matching
// algorithm, considering that the opening brackets were concealed in another
// macro. So, we recommend using (and have used) the *_done macros instead.
//
// Remark: These are all designed to be called from within the global namespace;
// they'll put things into either flecstan:: or llvm::yaml::, as appropriate.
// -----------------------------------------------------------------------------

// Macros:
//    flecstan_class
//    flecstan_class_done
// Purpose: make class containing YAML data, for a particular FleCSI macro.

#define flecstan_class(macname)                                                \
  namespace flecstan {                                                         \
  class macname : public flecsi_base                                           \
  {                                                                            \
  public:                                                                      \
    macname() {}                                                               \
    macname(const MacroCall & mc) : flecsi_base(mc) {}

#define flecstan_class_done                                                    \
  }                                                                            \
  ;                                                                            \
  } /* namespace flecstan */

// Macros:
//    flecstan_maptraits
//    flecstan_map
//    flecstan_maptraits_done
// Purpose: make YAML mappings for classes as defined with the above macros.

#define flecstan_maptraits(macname)                                            \
  namespace llvm {                                                             \
  namespace yaml {                                                             \
  template<>                                                                   \
  class MappingTraits<flecstan::macname>                                       \
  {                                                                            \
  public:                                                                      \
    static void mapping(IO & io, flecstan::macname & c) {                      \
      c.map(io); /* see class flecsi_base, above */

#define flecstan_map(field) io.mapRequired(#field, c.field)

#define flecstan_maptraits_done                                                \
  }                                                                            \
  }                                                                            \
  ;                                                                            \
  } /* namespace yaml */                                                       \
  } /* namespace llvm */

// Expander
// Applies the given macro to our various classes / FleCSI-macro names.
// Reduces repetitiveness elsewhere.

#define flecstan_expand(apply_macro, sym)                                      \
                                                                               \
  apply_macro(flecsi_register_program)                                         \
    sym apply_macro(flecsi_register_top_level_driver) sym                      \
                                                                               \
      apply_macro(flecsi_register_global_object) sym apply_macro(              \
        flecsi_set_global_object)                                              \
        sym apply_macro(flecsi_initialize_global_object) sym apply_macro(      \
          flecsi_get_global_object) sym                                        \
                                                                               \
          apply_macro(flecsi_register_task_simple) sym apply_macro(            \
            flecsi_register_task)                                              \
            sym apply_macro(flecsi_register_mpi_task_simple) sym apply_macro(  \
              flecsi_register_mpi_task) sym                                    \
                                                                               \
              apply_macro(flecsi_color) sym apply_macro(flecsi_colors) sym     \
                                                                               \
                apply_macro(flecsi_execute_task_simple)                        \
                  sym apply_macro(flecsi_execute_task) sym apply_macro(        \
                    flecsi_execute_mpi_task_simple)                            \
                    sym apply_macro(flecsi_execute_mpi_task) sym apply_macro(  \
                      flecsi_execute_reduction_task) sym                       \
                                                                               \
                      apply_macro(flecsi_register_reduction_operation) sym     \
                                                                               \
                        apply_macro(flecsi_register_function) sym apply_macro( \
                          flecsi_execute_function)                             \
                          sym apply_macro(flecsi_function_handle)              \
                            sym apply_macro(flecsi_define_function_type) sym   \
                                                                               \
                              apply_macro(flecsi_register_data_client)         \
                                sym apply_macro(flecsi_register_field)         \
                                  sym apply_macro(flecsi_register_global)      \
                                    sym apply_macro(flecsi_register_color) sym \
                                                                               \
                                      apply_macro(flecsi_get_handle)           \
                                        sym apply_macro(                       \
                                          flecsi_get_client_handle)            \
                                          sym apply_macro(flecsi_get_handles)  \
                                            sym apply_macro(                   \
                                              flecsi_get_handles_all) sym      \
                                                                               \
                                              apply_macro(flecsi_get_global)   \
                                                sym apply_macro(               \
                                                  flecsi_get_color)            \
                                                  sym apply_macro(             \
                                                    flecsi_get_mutator) sym

// flecsi_is_at
// flecsi_has_attribute_at
// flecsi_has_attribute
// flecsi_get_all_handles
// flecsi_put_all_handles

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// For several classes X:
//    X
//    MappingTraits<X>
// Every such pair corresponds to a specific FleCSI macro.
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// From FleCSI's execution.h
// Top-Level Driver Interface
// -----------------------------------------------------------------------------

// flecsi_register_program ( program )
flecstan_class(flecsi_register_program) std::string program;
flecstan_class_done

  flecstan_maptraits(flecsi_register_program) flecstan_map(program);
flecstan_maptraits_done

  // flecsi_register_top_level_driver ( driver )
  flecstan_class(flecsi_register_top_level_driver) std::string driver;
flecstan_class_done

  flecstan_maptraits(flecsi_register_top_level_driver) flecstan_map(driver);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Object Registration Interface
  // -----------------------------------------------------------------------------

  // flecsi_register_global_object ( index, nspace, type )
  flecstan_class(flecsi_register_global_object) std::string index;
std::string nspace;
std::string type;
flecstan_class_done

  flecstan_maptraits(flecsi_register_global_object) flecstan_map(index);
flecstan_map(nspace);
flecstan_map(type);
flecstan_maptraits_done

  // flecsi_set_global_object ( index, nspace, type, obj )
  flecstan_class(flecsi_set_global_object) std::string index;
std::string nspace;
std::string type;
std::string obj;
flecstan_class_done

  flecstan_maptraits(flecsi_set_global_object) flecstan_map(index);
flecstan_map(nspace);
flecstan_map(type);
flecstan_map(obj);
flecstan_maptraits_done

  // flecsi_initialize_global_object ( index, nspace, type, ... )
  flecstan_class(flecsi_initialize_global_object) std::string index;
std::string nspace;
std::string type;
std::vector<VarArgTypeValue> varargs;
flecstan_class_done

  flecstan_maptraits(flecsi_initialize_global_object) flecstan_map(index);
flecstan_map(nspace);
flecstan_map(type);
flecstan_map(varargs);
flecstan_maptraits_done

  // flecsi_get_global_object ( index, nspace, type )
  flecstan_class(flecsi_get_global_object) std::string index;
std::string nspace;
std::string type;
flecstan_class_done

  flecstan_maptraits(flecsi_get_global_object) flecstan_map(index);
flecstan_map(nspace);
flecstan_map(type);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Task Registration Interface
  // -----------------------------------------------------------------------------

  // flecsi_register_task_simple ( task, processor, launch )
  flecstan_class(flecsi_register_task_simple) std::string task;
std::string processor;
std::string launch;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_register_task_simple) flecstan_map(task);
flecstan_map(processor);
flecstan_map(launch);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_register_task ( task, nspace, processor, launch )
  flecstan_class(flecsi_register_task) std::string task;
std::string nspace;
std::string processor;
std::string launch;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_register_task) flecstan_map(task);
flecstan_map(nspace);
flecstan_map(processor);
flecstan_map(launch);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_register_mpi_task_simple ( task )
  flecstan_class(flecsi_register_mpi_task_simple) std::string task;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_register_mpi_task_simple) flecstan_map(task);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_register_mpi_task ( task, nspace )
  flecstan_class(flecsi_register_mpi_task) std::string task;
std::string nspace;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_register_mpi_task) flecstan_map(task);
flecstan_map(nspace);
flecstan_map(hash);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Task Execution Interface: color
  // -----------------------------------------------------------------------------

  // flecsi_color ( )
  flecstan_class(flecsi_color) flecstan_class_done

  flecstan_maptraits(flecsi_color) flecstan_maptraits_done

  // flecsi_colors ( )
  flecstan_class(flecsi_colors) flecstan_class_done

  flecstan_maptraits(flecsi_colors) flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Task Execution Interface: execute
  // -----------------------------------------------------------------------------

  // flecsi_execute_task_simple ( task, launch, ... )
  flecstan_class(flecsi_execute_task_simple) std::string task;
std::string launch;
std::vector<VarArgTypeValue> varargs;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_task_simple) flecstan_map(task);
flecstan_map(launch);
flecstan_map(varargs);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_execute_task ( task, nspace, launch, ... )
  flecstan_class(flecsi_execute_task) std::string task;
std::string nspace;
std::string launch;
std::vector<VarArgTypeValue> varargs;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_task) flecstan_map(task);
flecstan_map(nspace);
flecstan_map(launch);
flecstan_map(varargs);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_execute_mpi_task_simple ( task, ... )
  flecstan_class(flecsi_execute_mpi_task_simple) std::string task;
std::vector<VarArgTypeValue> varargs;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_mpi_task_simple) flecstan_map(task);
flecstan_map(varargs);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_execute_mpi_task ( task, nspace, ... )
  flecstan_class(flecsi_execute_mpi_task) std::string task;
std::string nspace;
std::vector<VarArgTypeValue> varargs;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_mpi_task) flecstan_map(task);
flecstan_map(nspace);
flecstan_map(varargs);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_execute_reduction_task ( task, nspace, launch, type, datatype, ... )
  flecstan_class(flecsi_execute_reduction_task) std::string task;
std::string nspace;
std::string launch;
std::string type;
std::string datatype;
std::vector<VarArgTypeValue> varargs;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_reduction_task) flecstan_map(task);
flecstan_map(nspace);
flecstan_map(launch);
flecstan_map(type);
flecstan_map(datatype);
flecstan_map(varargs);
flecstan_map(hash);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Reduction Interface
  // -----------------------------------------------------------------------------

  // flecsi_register_reduction_operation ( type, datatype )
  flecstan_class(flecsi_register_reduction_operation) std::string type;
std::string datatype;
flecstan_class_done

  flecstan_maptraits(flecsi_register_reduction_operation) flecstan_map(type);
flecstan_map(datatype);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's execution.h
  // Function Interface
  // -----------------------------------------------------------------------------

  // flecsi_register_function ( func, nspace )
  flecstan_class(flecsi_register_function) std::string func;
std::string nspace;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_register_function) flecstan_map(func);
flecstan_map(nspace);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_execute_function ( handle, ... )
  flecstan_class(flecsi_execute_function) std::string handle;
std::vector<VarArgTypeValue> varargs;
flecstan_class_done

  flecstan_maptraits(flecsi_execute_function) flecstan_map(handle);
flecstan_map(varargs);
flecstan_maptraits_done

  // flecsi_function_handle ( func, nspace )
  flecstan_class(flecsi_function_handle) std::string func;
std::string nspace;
std::string hash;
flecstan_class_done

  flecstan_maptraits(flecsi_function_handle) flecstan_map(func);
flecstan_map(nspace);
flecstan_map(hash);
flecstan_maptraits_done

  // flecsi_define_function_type ( func, return_type, ... )
  flecstan_class(flecsi_define_function_type) std::string func;
std::string return_type;
std::vector<VarArgType> varargs;
flecstan_class_done

  flecstan_maptraits(flecsi_define_function_type) flecstan_map(func);
flecstan_map(return_type);
flecstan_map(varargs);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's data.h
  // Registration
  // -----------------------------------------------------------------------------

  // flecsi_register_data_client ( client_type, nspace, name )
  flecstan_class(flecsi_register_data_client) std::string client_type;
bool data_client_t;
std::string nspace;
std::string name;
flecstan_class_done

  flecstan_maptraits(flecsi_register_data_client) flecstan_map(client_type);
flecstan_map(data_client_t);
flecstan_map(nspace);
flecstan_map(name);
flecstan_maptraits_done

  // flecsi_register_field
  //    ( client_type, nspace, name, data_type, storage_class, versions, ...)
  // The macro in question calls the register_field() that's defined (at the
  // time of this writing) in field.h. That function has "size_t INDEX_SPACE =
  // 0" as its last template argument. So, the macro's ##__VA_ARGS__, which is
  // the last template argument it sends to register_field(), is an (optional)
  // index space.
    flecstan_class(flecsi_register_field) std::string client_type;
std::string nspace;
std::string name;
std::string data_type;
std::string storage_class;
std::uint32_t versions;
std::uint32_t index_space;
std::string type;
flecstan_class_done

  flecstan_maptraits(flecsi_register_field) flecstan_map(client_type);
flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(storage_class);
flecstan_map(versions);
flecstan_map(index_space);
flecstan_map(type);
flecstan_maptraits_done

  // flecsi_register_global ( nspace, name, data_type, versions)
  flecstan_class(flecsi_register_global) std::string nspace;
std::string name;
std::string data_type;
std::uint32_t versions;
flecstan_class_done

  flecstan_maptraits(flecsi_register_global) flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(versions);
flecstan_maptraits_done

  // flecsi_register_color ( nspace, name, data_type, versions)
  flecstan_class(flecsi_register_color) std::string nspace;
std::string name;
std::string data_type;
std::uint32_t versions;
flecstan_class_done

  flecstan_maptraits(flecsi_register_color) flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(versions);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's data.h
  // Some handle-related macros.
  // -----------------------------------------------------------------------------

  // flecsi_get_handle
  //    ( client_handle, nspace, name, data_type, storage_class, version )
  flecstan_class(flecsi_get_handle) std::string client_handle;
std::string nspace;
std::string name;
std::string data_type;
std::string storage_class;
std::uint32_t version;
std::string type;
flecstan_class_done

  flecstan_maptraits(flecsi_get_handle) flecstan_map(client_handle);
flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(storage_class);
flecstan_map(version);
flecstan_map(type);
flecstan_maptraits_done

  // flecsi_get_client_handle ( client_type, nspace, name )
  flecstan_class(flecsi_get_client_handle) std::string client_type;
std::string nspace;
std::string name;
flecstan_class_done

  flecstan_maptraits(flecsi_get_client_handle) flecstan_map(client_type);
flecstan_map(nspace);
flecstan_map(name);
flecstan_maptraits_done

  // flecsi_get_handles ( client, nspace, data_type, storage_class, version, ...
  // )
    flecstan_class(flecsi_get_handles) std::string client;
std::string nspace;
std::string data_type;
std::string storage_class;
std::string version;
std::vector<VarArgTypeValue> varargs;
flecstan_class_done

  flecstan_maptraits(flecsi_get_handles) flecstan_map(client);
flecstan_map(nspace);
flecstan_map(data_type);
flecstan_map(storage_class);
flecstan_map(version);
flecstan_map(varargs);
flecstan_maptraits_done

  // flecsi_get_handles_all ( client, data_type, storage_class, version, ... )
  flecstan_class(flecsi_get_handles_all) std::string client;
std::string data_type;
std::string storage_class;
std::string version;
std::vector<VarArgTypeValue> varargs;
flecstan_class_done

  flecstan_maptraits(flecsi_get_handles_all) flecstan_map(client);
flecstan_map(data_type);
flecstan_map(storage_class);
flecstan_map(version);
flecstan_map(varargs);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's data.h
  // Some flecsi_get_* macros.
  // -----------------------------------------------------------------------------

  // flecsi_get_global ( nspace, name, data_type, version )
  flecstan_class(flecsi_get_global) std::string nspace;
std::string name;
std::string data_type;
std::uint32_t version;
flecstan_class_done

  flecstan_maptraits(flecsi_get_global) flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(version);
flecstan_maptraits_done

  // flecsi_get_color ( nspace, name, data_type, version )
  flecstan_class(flecsi_get_color) std::string nspace;
std::string name;
std::string data_type;
std::uint32_t version;
flecstan_class_done

  flecstan_maptraits(flecsi_get_color) flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(version);
flecstan_maptraits_done

  // flecsi_get_mutator
  //    ( client_handle, nspace, name, data_type, storage_class, version, slots
  //    )
    flecstan_class(flecsi_get_mutator) std::string client_handle;
std::string nspace;
std::string name;
std::string data_type;
std::string storage_class;
std::uint32_t version;
std::string slots;
flecstan_class_done

  flecstan_maptraits(flecsi_get_mutator) flecstan_map(client_handle);
flecstan_map(nspace);
flecstan_map(name);
flecstan_map(data_type);
flecstan_map(storage_class);
flecstan_map(version);
flecstan_map(slots);
flecstan_maptraits_done

  // -----------------------------------------------------------------------------
  // From FleCSI's data.h
  // Miscellaneous macros.
  // -----------------------------------------------------------------------------

  /*
  flecsi_is_at            (            index_space )
  flecsi_has_attribute_at ( attribute, index_space )
  flecsi_has_attribute    ( attribute)

  flecsi_get_all_handles
     ( client, storage_class,              handles, hashes, namespaces, versions
  ) flecsi_put_all_handles ( client, storage_class, num_handles, handles,
  hashes, namespaces, versions )

  Apparently, these are deprecated; so, we won't implement them at this time.
  The first three are used only in the flecsi/data/test/storage_class.cc, but
  I noticed while working elsewhere that this test isn't currently included in
  the test suite. As for the flecsi_[put|get]_all_handles macros, they aren't
  called from anywhere in the current FleCSI code. Also, they insert code with
  calls to function templates "put_all_handles" and "get_all_handles"; those
  are nowhere to be found in FleCSI.
  */

  // -----------------------------------------------------------------------------
  // Yaml
  // MappingTraits<Yaml>
  // -----------------------------------------------------------------------------

  namespace flecstan {

  class Yaml
  {
  public:
#define flecstan_vectors(macname)                                              \
  CalledMatched<flecstan::macname> macname;                                    \
                                                                               \
  void push(const flecstan::macname & value) {                                 \
    macname.matched.push_back(value);                                          \
  }                                                                            \
                                                                               \
  void push(flecstan::macname & value, const std::vector<std::string> & scp) { \
    value.scope = scp;                                                         \
    push(value);                                                               \
  }

    flecstan_expand(flecstan_vectors, )

#undef flecstan_vectors
  };

} // namespace flecstan

namespace llvm {
namespace yaml {

template<>
class MappingTraits<flecstan::Yaml>
{
public:
  static void mapping(IO & io, flecstan::Yaml & c) {
    flecstan_expand(flecstan_map, ;)
  }
};

} // namespace yaml
} // namespace llvm

// -----------------------------------------------------------------------------
// Cleanup
// -----------------------------------------------------------------------------

#undef flecstan_class
#undef flecstan_class_done

#undef flecstan_maptraits
#undef flecstan_map
#undef flecstan_maptraits_done
