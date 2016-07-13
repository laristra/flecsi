/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include "flecsi/data/default/default_storage_policy.h"
#include "flecsi/data/new_data.h"
#include "flecsi/data/data_client.h"

using namespace flecsi;

struct user_meta_data_t {
  void initialize() {}
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
		const const_string_t & key, size_t versions=1, Args && ... args) {
		return storage_type_t<DT>::template register_data<T,NS>(data_store_,
			runtime_namespace, key, versions, std::forward<Args>(args)...);
	} // register_data

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_accessor(uintptr_t runtime_namespace,
    const const_string_t & key, size_t version=0) {
    return storage_type_t<DT>::template get_accessor<T,NS>(data_store_,
			runtime_namespace, key, version);
  } // get_accessor

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_handle(uintptr_t runtime_namespace,
    const const_string_t & key, size_t version=0) {
    return storage_type_t<DT>::template get_handle<T,NS>(data_store_,
			runtime_namespace, key, version);
  } // get_accessor

private:

	data_t() {}

};

using new_data_t = data_model::new_data_t<>;

enum mesh_index_spaces_t : size_t {
  vertices,
  edges,
  faces,
  cells
}; // enum mesh_index_spaces_t

enum class privileges : size_t {
  read,
  read_write,
  write_discard
}; // enum data_access_type_t

struct mesh_t : public data_client_t {

  size_t indices(size_t index_space_id) override {

    switch(index_space_id) {
      case cells:
        return 100;
        break;
      default:
        // FIXME: lookup user-defined index space
        assert(false && "unknown index space");
    } // switch
  }

}; // struct mesh_t

#define register_data(client, name, versions, data_type, storage_type, ...) \
  new_data_t::instance().register_data<storage_type, data_type,             \
    data_name_space_t::user>(client, name, versions, ##__VA_ARGS__)

#define get_accessor(client, name, version, data_type, storage_type) \
  new_data_t::instance().get_accessor<storage_type, data_type,       \
    data_name_space_t::user>(client, name, version)

#define get_handle(client, name, version, data_type, storage_type,        \
  privileges)                                                             \
  new_data_t::instance().get_handle<storage_type, data_type, privileges,  \
    data_name_space_t::user>(client, name, version)

/*----------------------------------------------------------------------------*
 * Dense storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, dense) {
  mesh_t m;

  // Register 3 versions
  register_data(m, "pressure", 3, double, dense, cells);

  // Initialize
  {
  auto p0 = get_accessor(m, "pressure", 0, double, dense);
  auto p1 = get_accessor(m, "pressure", 1, double, dense);
  auto p2 = get_accessor(m, "pressure", 2, double, dense);

  for(size_t i(0); i<100; ++i) {
    p0[i] = i;
    p1[i] = 1000 + i;
    p2[i] = -double(i);
  } // for
  } // scope

  // Test
  {
  auto p0 = get_accessor(m, "pressure", 0, double, dense);
  auto p1 = get_accessor(m, "pressure", 1, double, dense);
  auto p2 = get_accessor(m, "pressure", 2, double, dense);

  for(size_t i(0); i<100; ++i) {
    ASSERT_EQ(p0[i], i);
    ASSERT_EQ(p1[i], 1000+p0[i]);
    ASSERT_EQ(p2[i], -p0[i]);
  } // for
  } // scope
} // TEST

/*----------------------------------------------------------------------------*
 * Scalar storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, scalar) {
  mesh_t m;

  struct my_data_t {
    double t;
    size_t n;
  }; // struct my_data_t

  register_data(m, "simulation data", 2, my_data_t, scalar);

  // initialize simulation data
  {
  auto s0 = get_accessor(m, "simulation data", 0, my_data_t, scalar);
  auto s1 = get_accessor(m, "simulation data", 1, my_data_t, scalar);
  s0->t = 0.5;
  s0->n = 100;
  s1->t = 1.5;
  s1->n = 200;
  } // scope

  {
  auto s0 = get_accessor(m, "simulation data", 0, my_data_t, scalar);
  auto s1 = get_accessor(m, "simulation data", 1, my_data_t, scalar);

  ASSERT_EQ(s0->t, 0.5);
  ASSERT_EQ(s0->n, 100);
  ASSERT_EQ(s1->t, 1.0 + s0->t);
  ASSERT_EQ(s1->n, 100 + s0->n);
  } // scope
} // TEST

/*----------------------------------------------------------------------------*
 * Sparse storage type.
 *----------------------------------------------------------------------------*/

TEST(storage, sparse) {
  mesh_t m;

  register_data(m, "materials", 1, double, sparse, 100, 4);

  {
  auto mats = get_accessor(m, "materials", 0, double, sparse);
  auto data = mats.data();

  for(size_t i(0); i<400; ++i) {
    data[i] = 0.0;
  } // for

  for(size_t i(0); i<100; ++i) {
    std::cout << "index: " << i << " equals: " << mats[i] << std::endl;
  } // for
  } // scope
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
