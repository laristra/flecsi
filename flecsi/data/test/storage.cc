/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/data/default/default_storage_policy.h"
#include "flecsi/data/new_data.h"

using namespace flecsi;

struct user_meta_data_t {
  user_meta_data_t(size_t _dummy) {}
}; // struct user_meta_data_t

using storage_policy_t =
  data_model::default_storage_policy_t<user_meta_data_t>;

struct data_t : public storage_policy_t {

	static data_t & instance() {
		static data_t d;
		return d;
	} // instance

	template<size_t data_type_t>
	using storage_type_t =
		typename storage_policy_t::template storage_type_t<data_type_t>;

	template<size_t DT, typename T, size_t NS, typename ... Args>
	decltype(auto) register_data(uintptr_t runtime_namespace,
		const const_string_t & key, Args && ... args) {
		return storage_type_t<DT>::template register_data<T,NS>(data_store_,
			runtime_namespace, key, std::forward<Args>(args)...);
	} // register_data

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_accessor(uintptr_t runtime_namespace,
    const const_string_t & key) {
    return storage_type_t<DT>::template get_accessor<T,NS>(data_store_,
			runtime_namespace, key);
  } // get_accessor

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_handle(uintptr_t runtime_namespace,
    const const_string_t & key) {
    return storage_type_t<DT>::template get_handle<T,NS>(data_store_,
			runtime_namespace, key);
  } // get_accessor

private:

	data_t() {}

};

#define register_data(manager, name, data_type, storage_type, ...) \
  manager.register_data<storage_type, data_type, 0>(0, name, ##__VA_ARGS__)

#define get_accessor(manager, name, data_type, storage_type) \
  manager.get_accessor<storage_type, data_type, 0>(0, name)

TEST(storage, dense) {
	data_t & d = data_t::instance();

  register_data(d, "pressure", double, dense, 100);

  auto p = get_accessor(d, "pressure", double, dense);
} // TEST

#undef register_data
#undef get_accessor

/*----------------------------------------------------------------------------*
 * Cinch test Macros
 *
 *  ==== I/O ====
 *  CINCH_CAPTURE()              : Insertion stream for capturing output.
 *                                 Captured output can be written or
 *                                 compared using the macros below.
 *
 *    EXAMPLE:
 *      CINCH_CAPTURE() << "My value equals: " << myvalue << std::endl;
 *
 *  CINCH_COMPARE_BLESSED(file); : Compare captured output with
 *                                 contents of a blessed file.
 *
 *  CINCH_WRITE(file);           : Write captured output to file.
 *
 *  CINCH_ASSERT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 *  CINCH_EXPECT(ASSERTION, ...) : Call Google test macro and automatically
 *                                 dump captured output (from CINCH_CAPTURE)
 *                                 on failure.
 *
 * Google Test Macros
 *
 * Basic Assertions:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_TRUE(condition);     EXPECT_TRUE(condition)
 *  ASSERT_FALSE(condition);    EXPECT_FALSE(condition)
 *
 * Binary Comparison:
 *
 *  ==== Fatal ====             ==== Non-Fatal ====
 *  ASSERT_EQ(val1, val2);      EXPECT_EQ(val1, val2)
 *  ASSERT_NE(val1, val2);      EXPECT_NE(val1, val2)
 *  ASSERT_LT(val1, val2);      EXPECT_LT(val1, val2)
 *  ASSERT_LE(val1, val2);      EXPECT_LE(val1, val2)
 *  ASSERT_GT(val1, val2);      EXPECT_GT(val1, val2)
 *  ASSERT_GE(val1, val2);      EXPECT_GE(val1, val2)
 *
 * String Comparison:
 *
 *  ==== Fatal ====                     ==== Non-Fatal ====
 *  ASSERT_STREQ(expected, actual);     EXPECT_STREQ(expected, actual)
 *  ASSERT_STRNE(expected, actual);     EXPECT_STRNE(expected, actual)
 *  ASSERT_STRCASEEQ(expected, actual); EXPECT_STRCASEEQ(expected, actual)
 *  ASSERT_STRCASENE(expected, actual); EXPECT_STRCASENE(expected, actual)
 *----------------------------------------------------------------------------*/

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
