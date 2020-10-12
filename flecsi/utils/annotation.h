/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */
#include <string>
#include <type_traits>
#if defined(ENABLE_CALIPER)
#include <caliper/Annotation.h>
#endif

#include <flecsi-config.h>

namespace flecsi {
namespace utils {

class annotation
{
public:
  /// used for specifying what detail of annotations to collect.
  enum class detail { low, medium, high };

  /// base for annotation contexts.
  template<class T>
  struct context {
#if defined(ENABLE_CALIPER)
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
  struct runtime_setup : region<execution> {
    inline static const std::string name{"set-up"};
  };
  struct spl_tlt_init : region<execution> {
    inline static const std::string name{"spl-tlt-init"};
    static constexpr detail detail_level = detail::low;
  };
  struct create_regions : region<execution> {
    inline static const std::string name{"create-regions"};
  };
  struct spl_spmd_init : region<execution> {
    inline static const std::string name{"spl-spmd-init"};
  };
  struct driver : region<execution> {
    inline static const std::string name{"driver"};
    static constexpr detail detail_level = detail::low;
  };
  struct runtime_finish : region<execution> {
    inline static const std::string name{"finish"};
  };

  template<class T>
  struct execute_task : region<execution> {
    /// Set code region name for regions inheriting from execute_task with the
    /// following prefix.
    static std::string const & name() {
      static const std::string name_{"execute_task->" + T::tag};
      return name_;
    }
  };
  struct execute_task_init : execute_task<execute_task_init> {
    inline static const std::string tag{"init-handles"};
    static constexpr detail detail_level = detail::high;
  };
  struct execute_task_initargs : execute_task<execute_task_initargs> {
    inline static const std::string tag{"init-args"};
    static constexpr detail detail_level = detail::high;
  };
  struct execute_task_prolog : execute_task<execute_task_prolog> {
    inline static const std::string tag{"prolog"};
    static constexpr detail detail_level = detail::high;
  };
  struct execute_task_user : execute_task<execute_task_user> {
    inline static const std::string tag{"user"};
  };
  struct execute_task_epilog : execute_task<execute_task_epilog> {
    inline static const std::string tag{"epilog"};
    static constexpr detail detail_level = detail::high;
  };
  struct execute_task_finalize : execute_task<execute_task_finalize> {
    inline static const std::string tag{"finalize-handles"};
    static constexpr detail detail_level = detail::high;
  };

  /**
   * Tag beginning of code region with caliper annotation.
   *
   * The region is only tagged if caliper is enabled and reg::detail_level
   * is compatible with the current annotation detail level.
   *
   * \tparam reg code region to tag (type inherits from annotation::region).
   */
  template<class reg>
  static std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
    typename reg::outer_context>::value>
  begin() {
#if defined(ENABLE_CALIPER)
    if constexpr(reg::detail_level <= detail_level) {
      reg::outer_context::ann.begin(reg::name.c_str());
    }
#endif
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
  static std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
                            typename reg::outer_context>::value &&
                          std::is_base_of<execute_task<reg>, reg>::value>
  begin(std::string task_name) {
#if defined(ENABLE_CALIPER)
    if constexpr(reg::detail_level <= detail_level) {
      std::string atag{reg::name() + "->" + task_name};
      reg::outer_context::ann.begin(atag.c_str());
    }
#endif
  }

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
  static std::enable_if_t<std::is_base_of<context<ctx>, ctx>::value> begin(
    std::string region_name) {
#if defined(ENABLE_CALIPER)
    if constexpr(severity <= detail_level) {
      ctx::ann.begin(region_name.c_str());
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
  static std::enable_if_t<std::is_base_of<context<typename reg::outer_context>,
    typename reg::outer_context>::value>
  end() {
#if defined(ENABLE_CALIPER)
    if constexpr(reg::detail_level <= detail_level) {
      reg::outer_context::ann.end();
    }
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
  static std::enable_if_t<std::is_base_of<context<ctx>, ctx>::value> end() {
#if defined(ENABLE_CALIPER)
    if constexpr(severity <= detail_level) {
      ctx::ann.end();
    }
#endif
  }

#if FLECSI_ANNOTATION_DETAIL == FLECSI_ANNOTATION_DETAIL_high
  static constexpr detail detail_level{detail::high};
#elif FLECSI_ANNOTATION_DETAIL == FLECSI_ANNOTATION_DETAIL_medium
  static constexpr detail detail_level{detail::medium};
#else // FLECSI_ANNOTATION_DETAIL == FLECSI_ANNOTATION_DETAIL_low
  static constexpr detail detail_level{detail::low};
#endif
};

/// Initialize caliper annotation objects from the context name.
#if defined(ENABLE_CALIPER)
template<class T>
cali::Annotation annotation::context<T>::ann{T::name};
#endif

} // namespace utils
} // namespace flecsi
