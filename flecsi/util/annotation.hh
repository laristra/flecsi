/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */
#include <string>
#include <type_traits>

#include <flecsi-config.h>

#if FLECSI_CALIPER_DETAIL != FLECSI_CALIPER_DETAIL_none
#include <caliper/Annotation.h>
#endif

namespace flecsi {
namespace util {

namespace annotation {
/// used for specifying what detail of annotations to collect.
enum class detail { low, medium, high };

#if FLECSI_CALIPER_DETAIL == FLECSI_CALIPER_DETAIL_high
constexpr detail detail_level{detail::high};
#elif FLECSI_CALIPER_DETAIL == FLECSI_CALIPER_DETAIL_medium
constexpr detail detail_level{detail::medium};
#elif FLECSI_CALIPER_DETAIL == FLECSI_CALIPER_DETAIL_low
constexpr detail detail_level{detail::low};
#else // FLECSI_CALIPER_DETAIL == FLECSI_CALIPER_DETAIL_none
#define DISABLE_CALIPER
#endif

/// base for annotation contexts.
template<class T>
struct context {
#if !defined(DISABLE_CALIPER)
  static cali::Annotation ann;
#endif
};
struct execution : context<execution> {
  static constexpr char name[] = "FleCSI-Execution";
};

/**
 *  base for code region annotations.
 *
 *   \tparam CTX annotation context for code region (must inherit from
 * annotation::context).
 */
template<class CTX>
struct region {
  using outer_context = CTX;
  static constexpr detail detail_level = detail::medium;
};
template<class T>
struct execute_task : region<execution> {
  /// Set code region name for regions inheriting from execute_task with the
  /// following prefix.
  inline static const std::string name{"execute_task->" + T::tag};
};
struct execute_task_bind : execute_task<execute_task_bind> {
  inline static const std::string tag{"bind-accessors"};
  static constexpr detail detail_level = detail::high;
};
struct execute_task_prolog : execute_task<execute_task_prolog> {
  inline static const std::string tag{"prolog"};
  static constexpr detail detail_level = detail::high;
};
struct execute_task_user : execute_task<execute_task_user> {
  inline static const std::string tag{"user"};
};
struct execute_task_unbind : execute_task<execute_task_unbind> {
  inline static const std::string tag{"unbind-accessors"};
  static constexpr detail detail_level = detail::high;
};

/**
 * Tag beginning of a custom named code region.
 *
 * This method can be used for custom code regions that are not
 * registered as region structs.
 *
 * \tparam ctx annotation context for named code region.
 * \tparam detail severity detail level to use for code region.
 */
template<class ctx, detail severity>
std::enable_if_t<std::is_base_of<context<ctx>, ctx>::value>
begin(const char * region_name) {
#if !defined(DISABLE_CALIPER)
  if constexpr(severity <= detail_level) {
    ctx::ann.begin(region_name);
  }
#else
  (void)region_name;
#endif
}
template<class ctx, detail severity>
std::enable_if_t<std::is_base_of<context<ctx>, ctx>::value>
begin(const std::string & region_name) {
#if !defined(DISABLE_CALIPER)
  begin<ctx, severity>(region_name.c_str());
#else
  (void)region_name;
#endif
}

/**
 * Tag beginning of code region with caliper annotation.
 *
 * The region is only tagged if caliper is enabled and reg::detail_level
 * is compatible with the current annotation detail level.
 *
 * \tparam reg code region to tag (type inherits from annotation::region).
 */
template<class reg>
std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
  typename reg::outer_context>::value>
begin() {
  begin<typename reg::outer_context, reg::detail_level>(reg::name.c_str());
}

/**
 * Tag beginning of an execute_task region.
 *
 * The execute_task region has multiple phases and is associated with a named
 * task.
 *
 * \tparam reg code region to tag (must inherit from
 * annotation::execute_task). \param std::string task_name name of task to
 * tag.
 */
template<class reg>
std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
                   typename reg::outer_context>::value &&
                 std::is_base_of<execute_task<reg>, reg>::value>
begin(std::string_view task_name) {
#if !defined(DISABLE_CALIPER)
  if constexpr(reg::detail_level <= detail_level) {
    std::string atag{reg::name + "->"};
    atag.append(task_name);
    begin<typename reg::outer_context, reg::detail_level>(atag.c_str());
  }
  else {
    (void)task_name;
  }
#else
  (void)task_name;
#endif
}

/**
 * Tag end of a custom named code region.
 *
 * This method can be used for custom code regions that are not
 * registered as region structs.
 *
 * \tparam ctx annotation context for named code region.
 * \tparam detail severity detail level to use for code region.
 */
template<class ctx, detail severity>
std::enable_if_t<std::is_base_of<context<ctx>, ctx>::value>
end() {
#if !defined(DISABLE_CALIPER)
  if constexpr(severity <= detail_level) {
    ctx::ann.end();
  }
#endif
}

/**
 * Tag end of code region with caliper annotation.
 *
 * The region is only tagged if caliper is enabled and reg::detail_level
 * is compatible with the current annotation detail level.
 *
 * \tparam reg code region to tag (type inherits from annotation::region).
 */
template<class reg>
std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
  typename reg::outer_context>::value>
end() {
  end<typename reg::outer_context, reg::detail_level>();
}

/**
 * Scope guard used for timing scoped regions.
 * \tparam ctx annotation context for named code region.
 * \tparam severity detail level to use for code region.
 */
template<class ctx, detail severity>
class guard
{
public:
  template<class Arg>
  guard(Arg && a) {
    begin<ctx, severity>(std::forward<Arg>(a));
  }
  ~guard() {
    end<ctx, severity>();
  }
};

/**
 * Scope guard used for timing scoped regions.
 * \tparam reg code region to tag (type inherits from annotation::region)
 */
template<class reg>
class rguard
{
public:
  template<class... Arg>
  rguard(Arg &&... a) {
    begin<reg>(std::forward<Arg>(a)...);
  }
  ~rguard() {
    end<reg>();
  }
};
}; // namespace annotation

/// Initialize caliper annotation objects from the context name.
#if !defined(DISABLE_CALIPER)
template<class T>
cali::Annotation annotation::context<T>::ann{T::name};
#endif

} // namespace util
} // namespace flecsi
