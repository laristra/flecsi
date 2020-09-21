#pragma once

/*! @file */

#include <limits>

#include <flecsi/execution/execution.h>

namespace flecsi {
namespace execution {
namespace reduction {
#define flecsi_register_operation_types(operation)                             \
  flecsi_register_reduction_operation(operation, int);                         \
  flecsi_register_reduction_operation(operation, long);                        \
  flecsi_register_reduction_operation(operation, short);                       \
  flecsi_register_reduction_operation(operation, unsigned);                    \
  flecsi_register_reduction_operation(operation, size_t);                      \
  flecsi_register_reduction_operation(operation, float);                       \
  flecsi_register_reduction_operation(operation, double);

flecsi_register_operation_types(sum);
flecsi_register_operation_types(min);
flecsi_register_operation_types(max);
flecsi_register_operation_types(product);

// implementation of the reduction operation that reduces 2 doubles

template<typename T, size_t N>
using array_type = std::array<T, N>;

template<typename T>
struct array_sum {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  typedef typename T::value_type value_type;

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    const size_t size = lhs.size();

    if constexpr(EXCLUSIVE) {
      for(size_t i = 0; i < size; i++)
        lhs.at(i) += rhs.at(i);
    }
    else {
      for(size_t i = 0; i < size; i++) {
        int64_t * target = (int64_t *)&lhs.at(i);
        union
        {
          int64_t as_int;
          value_type as_T;
        } oldval, newval;
        do {
          oldval.as_int = *target;
          newval.as_T = oldval.as_T + rhs.at(i);
        } while(
          !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
      } // for

    } // if constexpr
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {
    const size_t size = lhs.size();
    if constexpr(EXCLUSIVE) {
      for(size_t i = 0; i < size; i++)
        lhs.at(i) += rhs.at(i);
    }
    else {
      for(size_t i = 0; i < size; i++) {
        int64_t * target = (int64_t *)&lhs.at(i);
        union
        {
          int64_t as_int;
          value_type as_T;
        } oldval, newval;
        do {
          oldval.as_int = *target;
          newval.as_T = oldval.as_T + rhs.at(i);
        } while(
          !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
      } // for
    } // if constexpr

  } // fold

}; // struct sum

// flecsi_register_reduction_operation(array_sum, array_type<double, 2>);

} // namespace reduction
} // namespace execution
} // namespace flecsi
