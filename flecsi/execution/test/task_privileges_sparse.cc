/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <cinchtest.h>

#include <flecsi/data/sparse_accessor.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using mesh_t = flecsi::supplemental::test_mesh_2d_t;

template<size_t PS>
using mesh = data_client_handle_u<mesh_t, PS>;

template<size_t EP, size_t SP, size_t GP>
using field = sparse_accessor<size_t, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, meshes, mesh1);
flecsi_register_field(mesh_t,
  hydro,
  pressure,
  size_t,
  sparse,
  1,
  index_spaces::cells);

void
init(mesh<ro> mesh, sparse_mutator<double> sm) {
  auto rank = execution::context_t::instance().color();

  for(auto c : mesh.cells(owned)) {
    auto gid = c->gid();
    // for most cells, do a checkerboard pattern
    bool parity = (gid / 8 + gid % 8) & 1;
    int start = (parity ? 0 : 1);
    int stop = (parity ? 6 : 5);
    // make a few cells overflow
    if(gid >= 11 && gid <= 13)
      stop = (parity ? 16 : 17);
    for(size_t j = start; j < stop; j += 2) {
      sm(c, j) = rank * 10000 + gid * 100 + j;
    }
  }
} // init

flecsi_register_task(init, flecsi::execution, loc, index);
//----------------------------------------------------------------------------//
// Task A : Init field
//----------------------------------------------------------------------------//

void
task_A(mesh<ro> mesh, field<rw, rw, na> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(owned)) {
    for(auto entry : h.entries(c)) {
      size_t val = c->index()[1] + 100 * (c->index()[0] + 100 * rank);
      h(c, entry) = 1000000000 + val * 100;
    }
  }
  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 5;
    }
  }

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task A exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task A shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }
} // task a

flecsi_register_task(task_A, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task B: Read field
//----------------------------------------------------------------------------//

void
task_B(mesh<ro> mesh, field<rw, ro, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task B exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task B shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task B ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

} // task b

flecsi_register_task(task_B, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task C: Modify ghost
//----------------------------------------------------------------------------//

void
task_C(mesh<ro> mesh, field<rw, ro, wo> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 10 * (rank + 1);
    }
  }

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task C exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task C shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task C ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }
} // task c

flecsi_register_task(task_C, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task D: Modify ghost
//----------------------------------------------------------------------------//

void
task_D(mesh<ro> mesh, field<rw, rw, rw> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(owned)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 10 * (rank + 1);
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 10 * (rank + 3);
    }
  }

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task D exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task D shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task D ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }
} // task d

flecsi_register_task(task_D, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task E: Print field
//----------------------------------------------------------------------------//

void
task_E(mesh<ro> mesh, field<ro, ro, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task E exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task E shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task E ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

} // task e

flecsi_register_task(task_E, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task F: Modify field
//----------------------------------------------------------------------------//
void
task_F(mesh<ro> mesh, field<rw, rw, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(owned)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 10 * (rank + 5);
    }
  }

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task F exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task F shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task F ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }
} // task f

flecsi_register_task(task_F, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task G:
//----------------------------------------------------------------------------//
void
task_G(mesh<ro> mesh, field<rw, ro, na> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task G exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task G shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task G ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

} // modify

flecsi_register_task(task_G, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Task H : Init field
//----------------------------------------------------------------------------//

void
task_H(mesh<ro> mesh, field<rw, rw, na> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H before exclusive cell " << rank << " "
                      << c->id() << " " << c->gid() << " " << c->index()[0]
                      << " " << c->index()[1] << " " << entry << " "
                      << h(c, entry) << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H before shared cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H before ghost cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(owned)) {
    for(auto entry : h.entries(c)) {
      size_t val = c->index()[1] + 100 * (c->index()[0] + 100 * rank);
      h(c, entry) = 1000000000 + val * 100;
    }
  }
  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      h(c, entry) += 5;
    }
  }

  for(auto c : mesh.cells(exclusive)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H exclusive cell " << rank << " " << c->id()
                      << " " << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(shared)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H shared cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }

  for(auto c : mesh.cells(ghost)) {
    for(auto entry : h.entries(c)) {
      CINCH_CAPTURE() << "task H ghost cell " << rank << " " << c->id() << " "
                      << c->gid() << " " << c->index()[0] << " "
                      << c->index()[1] << " " << entry << " " << h(c, entry)
                      << std::endl;
    }
  }
} // task a

flecsi_register_task(task_H, flecsi::execution, loc, index);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;
  supplemental::do_test_mesh_2d_coloring();
  context_t::sparse_index_space_info_t isi;
  isi.index_space = index_spaces::cells;
  isi.max_entries_per_index = 10;
  isi.exclusive_reserve = 8192;
  context_t::instance().set_sparse_index_space_info(isi);
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::supplemental, index, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  auto pm = flecsi_get_mutator(ch, hydro, pressure, double, sparse, 0, 5);
  auto ph = flecsi_get_handle(ch, hydro, pressure, size_t, sparse, 0);

  flecsi_execute_task(init, flecsi::execution, index, ch, pm);
  flecsi_execute_task(task_A, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_B, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_C, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_D, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_E, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_F, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_G, flecsi::execution, index, ch, ph);
  flecsi_execute_task(task_H, flecsi::execution, index, ch, ph);
  auto future = flecsi_execute_task(task_E, flecsi::execution, index, ch, ph);
  future.wait(); // wait before comparing results

  auto & context = execution::context_t::instance();
  if(context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("task_privileges_sparse.blessed"));
  }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(task_privileges, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
