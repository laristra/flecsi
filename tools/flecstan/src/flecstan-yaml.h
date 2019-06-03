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

#include "flecstan-misc.h"

// -----------------------------------------------------------------------------
// llvm::yaml::
// SequenceTraits<vector<T>>
// -----------------------------------------------------------------------------

namespace llvm {
namespace yaml {

// For std::vector<T,ALLOCATOR>>
template<class T, class ALLOCATOR>
class SequenceTraits<std::vector<T, ALLOCATOR>>
{
public:
  // size
  static std::size_t size(IO &, std::vector<T, ALLOCATOR> & vec) {
    flecstan::debug("SequenceTraits<vector>::size()");
    return vec.size();
  }

  // element
  static T &
  element(IO &, std::vector<T, ALLOCATOR> & vec, const std::size_t index) {
    flecstan::debug("SequenceTraits<vector>::element()");
    flecstan_assert(index < vec.size());
    return vec[index];
  }
};

// For std::vector<std::string>
// It seems that we need this full specialization, in order to avoid
// an ambiguous template instantiation.
template<>
class SequenceTraits<std::vector<std::string>>
{
public:
  // size
  static std::size_t size(IO &, std::vector<std::string> & vec) {
    flecstan::debug("SequenceTraits<std::vector<std::string>>::size()");
    return vec.size();
  }

  // element
  static std::string &
  element(IO &, std::vector<std::string> & vec, const std::size_t index) {
    flecstan::debug("SequenceTraits<std::vector<std::string>>::element()");
    flecstan_assert(index < vec.size());
    return vec[index];
  }
};

} // namespace yaml
} // namespace llvm

// -----------------------------------------------------------------------------
// YAML constructs related to varargs
// -----------------------------------------------------------------------------

// ------------------------
// VarArgType
// VarArgTypeValue
// ------------------------

namespace flecstan {

// VarArgType
class VarArgType
{
public:
  // data
  std::string type;

  // constructors
  VarArgType() : type("") {}

  VarArgType(const std::string & t) : type(t) {}
};

// VarArgTypeValue
class VarArgTypeValue
{
public:
  // data
  std::string type;
  std::string value;

  // constructors
  VarArgTypeValue() : type(""), value("") {}

  VarArgTypeValue(const std::string & t, const std::string & v)
    : type(t), value(v) {}
};

} // namespace flecstan

// ------------------------
// llvm::yaml::
// MappingTraits<above>
// ------------------------

namespace llvm {
namespace yaml {

// MappingTraits<VarArgType>
template<>
class MappingTraits<flecstan::VarArgType>
{
public:
  static void mapping(IO & io, flecstan::VarArgType & c) {
    io.mapRequired("type", c.type);
  }
};

// MappingTraits<VarArgTypeValue>
template<>
class MappingTraits<flecstan::VarArgTypeValue>
{
public:
  static void mapping(IO & io, flecstan::VarArgTypeValue & c) {
    io.mapRequired("type", c.type);
    io.mapRequired("value", c.value);
  }
};

} // namespace yaml
} // namespace llvm

// -----------------------------------------------------------------------------
// Helper functions that deal with the above YAML constructs.
// Just declarations here.
// -----------------------------------------------------------------------------

namespace flecstan {

void getVarArgsFunction(const clang::CallExpr * const,
  std::vector<VarArgTypeValue> &,
  const unsigned = 0);

void getVarArgsTemplate(const clang::TypeAliasDecl * const,
  std::vector<VarArgType> &,
  std::string &);

} // namespace flecstan
