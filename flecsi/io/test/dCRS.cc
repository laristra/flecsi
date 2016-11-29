/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>
#include <mpi.h>
#include <parmetis.h>

// Need to figure out where to put this...
#include "flecsi/partition/mpi_utils.h"

#include "flecsi/io/simple_definition.h"
#include "flecsi/io/set_utils.h"


TEST(dCRS, init) {

  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  //flecsi::io::simple_definition_t sd("simple2d-4x4.msh");

  //--------------------------------------------------------------------------//
  // Create dCRS from mesh definition.
  //--------------------------------------------------------------------------//
  size_t quot = sd.num_cells()/size;
  size_t rem = sd.num_cells()%size;

#if 0
  if(rank == 0) {
    std::cout << "quot: " << quot << std::endl;
    std::cout << "rem: " << rem << std::endl;
  }
#endif

  // Balance the distribution
  size_t init_indices = quot + ((rank >= (size - rem)) ? 1 : 0);

#if 0
  std::cout << "rank " << rank << " has " << init_indices << std::endl;
#endif

  std::vector<idx_t> vtxdist = { 0 };

  for(size_t r(0); r<size; ++r) {
    const size_t indices = quot + ((r >= (size - rem)) ? 1 : 0);
    vtxdist.push_back(vtxdist[r] + indices);
  } // for

  if(rank == 0) {
    std::cout << "vtxdist: ";
    for(auto i: vtxdist) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if

  std::vector<idx_t> xadj = { 0 };
  std::vector<idx_t> adjncy;
  for(size_t i(0); i<init_indices; ++i) {
		//
    auto neighbors = flecsi::io::cell_neighbors(sd, vtxdist[rank] + i, 2);

#if 0
    if(rank == 0) {
      std::cout << "vertex " << vtxdist[rank]+i << std::endl;
      for(auto n: neighbors) {
        std::cout << n << " ";
      } // for
      std::cout << std::endl;
    } // if
#endif

    for(auto n: neighbors) {
      adjncy.push_back(n);
    } // for

    xadj.push_back(xadj[i] + neighbors.size());
  } // for

#if 0
  if(rank == 0) {
    std::cout << "xadj: ";
    for(auto i: xadj) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;

    std::cout << "adjncy: ";
    for(auto i: adjncy) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

  //--------------------------------------------------------------------------//
  // Call ParMETIS partitioner
  //--------------------------------------------------------------------------//

  idx_t wgtflag = 0;
  idx_t numflag = 0;
  idx_t ncon = 1;
  std::vector<real_t> tpwgts(size);

  real_t sum = 0.0;
  for(size_t i(0); i<tpwgts.size(); ++i) {
    if(i == (tpwgts.size()-1)) {
      tpwgts[i] = 1.0 - sum;
    }
    else {
      tpwgts[i] = 1.0/size;
      sum += tpwgts[i];
    } // if

#if 0
    if(rank == 0) {
      std::cout << tpwgts[i] << std::endl;
    } // if
#endif
  } // for

  real_t ubvec = 1.05;
  idx_t options = 0;
  idx_t edgecut;
  MPI_Comm comm = MPI_COMM_WORLD; 
  std::vector<idx_t> part(init_indices);

  int result = ParMETIS_V3_PartKway(&vtxdist[0], &xadj[0], &adjncy[0],
    nullptr, nullptr, &wgtflag, &numflag, &ncon, &size, &tpwgts[0],
    &ubvec, &options, &edgecut, &part[0], &comm);

  //if(rank == 0) {
    std::cout << "rank " << rank << ": ";
    for(size_t i(0); i<init_indices; ++i) {
      std::cout << "[" << part[i] << ", " << vtxdist[rank]+i << "] ";
    } // for
    std::cout << std::endl;
  //} // if

  std::vector<idx_t> send_cnts(size, 0);
  std::vector<std::vector<idx_t>> sbuffers;

  std::set<size_t> primary;

  for(size_t r(0); r<size; ++r) {
    std::vector<idx_t> indices;
//    if(r != rank) {
      for(size_t i(0); i<init_indices; ++i) {
        if(part[i] == r) {
          indices.push_back(vtxdist[rank] + i);
        }
        else if(part[i] == rank) {
          primary.insert(vtxdist[rank] + i);
        } // if
      } // for
//    } // if
    sbuffers.push_back(indices);
    send_cnts[r] = indices.size();
  } // for

//  if(rank == 0) {
    size_t rcnt(0);
    for(auto r: sbuffers) {
      std::cout << "rank " << rank << " sends " << rcnt++ <<
        " " << send_cnts[rcnt] << ": ";
      for(auto i: r) {
        std::cout << i << " ";
      } // for
      std::cout << std::endl;
    } // for
 // } // if

  std::vector<idx_t> recv_cnts(size);
  result = MPI_Alltoall(&send_cnts[0], 1,
    flecsi::dmp::mpi_typetraits<idx_t>::type(), &recv_cnts[0], 1,
    flecsi::dmp::mpi_typetraits<idx_t>::type(), MPI_COMM_WORLD);

  if(rank == 0) {
    std::cout << "rank " << rank << " receives: ";
    for(size_t i(0); i<size; ++i) {
      std::cout << recv_cnts[i] << " ";
    } // for
    std::cout << std::endl;
  } // if

  // Begin receive operations
  std::vector<std::vector<idx_t>> rbuffers(size);
  std::vector<MPI_Request> requests;
  for(size_t r(0); r<size; ++r) {
    if(recv_cnts[r]) {
      rbuffers[r].resize(recv_cnts[r]);

      // Add a new request
      requests.push_back({});
      MPI_Irecv(&rbuffers[r][0], recv_cnts[r],
        flecsi::dmp::mpi_typetraits<idx_t>::type(),
        r, 0, MPI_COMM_WORLD, &requests[requests.size()-1]);
    } // if
  } // for

  // Begin send operations
  for(size_t r(0); r<size; ++r) {
    if(send_cnts[r]) {
      sbuffers[r].resize(send_cnts[r]);
      MPI_Send(&sbuffers[r][0], send_cnts[r],
        flecsi::dmp::mpi_typetraits<idx_t>::type(),
        r, 0, MPI_COMM_WORLD);
    } // if
  } // for

  // Wait on the receive operations
  std::vector<MPI_Status> status(requests.size());
  MPI_Waitall(requests.size(), &requests[0], &status[0]);

//  if(rank == 0) {
    //std::cout << "rank " << rank << " got" << std::endl;
    for(size_t r(0); r<size; ++r) {
      if(recv_cnts[r]) {
        for(auto i: rbuffers[r]) {
          std::cout << "inserting " << i << " for rank " << rank << std::endl;
          //std::cout << i << " ";
          primary.insert(i);
        } // for
        //std::cout << std::endl;
      } // if
    } // for
//  } // if

//  if(rank == 0) {
    std::cout << "rank " << rank << " primary partition:" << std::endl;
    for(auto i: primary) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
//  } // if

  //--------------------------------------------------------------------------//
  // Form dependency closure.
  //--------------------------------------------------------------------------//

	//
  auto closure = flecsi::io::cell_closure(sd, primary, 1);

#if 0
  std::cout << "closure" << std::endl;
  for(auto i: closure) {
    std::cout << i << std::endl;
  } // for
#endif

  // Subtracting out the initial set leaves just the nearest
  // neighbors. This is similar to the image of the adjacency
  // graph of the initial indices.
  auto nn = flecsi::io::set_difference(closure, primary);

#if 0
  std::cout << "nearest neighbors" << std::endl;
  for(auto i: nn) {
    std::cout << i << std::endl;
  } // for
#endif

  // The closure of the nearest neighbors intersected with
  // the initial indeces gives the shared indices. This is similar to
  // the preimage of the nearest neighbors.
  auto nnclosure = flecsi::io::cell_closure(sd, nn, 1);
  auto shared = flecsi::io::set_intersection(nnclosure, primary);

#if 0
  std::cout << "shared" << std::endl;
  for(auto i: shared) {
    std::cout << i << std::endl;
  } // for
#endif

  // One can iteratively add halos of nearest neighbors, e.g.,
  // here we add the next nearest neighbors.
  auto nnn = flecsi::io::set_difference(flecsi::io::cell_closure(sd, nn, 1),
    closure);

#if 0
  std::cout << "next nearest neighbors" << std::endl;
  for(auto i: nnn) {
    std::cout << i << std::endl;
  } // for
#endif

  //--------------------------------------------------------------------------//
  // Get full neighbor information.
  //--------------------------------------------------------------------------//

  auto request_indices_set = flecsi::io::set_union(nn, nnn);

  std::vector<size_t> request_indices_vector(request_indices_set.begin(),
    request_indices_set.end());

  size_t request_indices_size = request_indices_set.size();
  size_t max_request_indices(0);

  std::cout << "rank " << rank << " indices set: " <<
    request_indices_size << std::endl;

  result = MPI_Allreduce(&request_indices_size, &max_request_indices, 1,
    flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_MAX, MPI_COMM_WORLD);
    
  if(rank == 0) {
    std::cout << "max indices: " << max_request_indices << std::endl;
  } // if

#if 0
  if(rank == 0) {
    std::cout << "request indices: ";
    for(auto i: request_indices_set) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;
  } // if
#endif

  std::vector<size_t> input_indices(size*max_request_indices,
    std::numeric_limits<size_t>::max());
  std::vector<size_t> info_indices(size*max_request_indices);
  std::vector<size_t> input_offsets(size*max_request_indices);
  std::vector<size_t> info_offsets(size*max_request_indices);

  for(size_t v(0); v<size; ++v) {

    size_t off(0);
    const size_t voff = v*max_request_indices;
    for(auto i: request_indices_set) {
      input_indices[voff + off++] = i;
    } // for

    if(rank == 0) {
      std::cout << "rank " << rank << " requests: ";
      for(size_t i(0); i<max_request_indices; ++i) {
        std::cout << input_indices[voff + i] << " ";
      } // for
      std::cout << std::endl;
    } // if
  } // for

//  std::vector<std::vector<size_t>> input_indices(max_request_indices,
//    std::numeric_limits<size_t>::max());

//  for(auto i: request_indices_set) {
//    input_indices[i] = i;
//  } // for


  result = MPI_Alltoall(&input_indices[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(),
    &info_indices[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

  // Reset input indices to use to send back information
  std::fill(input_indices.begin(), input_indices.end(),
    std::numeric_limits<size_t>::max());

  for(size_t r(0); r<size; ++r) {
    if(r == rank) {
      continue;
    } // if

    size_t * info = &info_indices[r*max_request_indices];
    size_t * offset = &input_offsets[r*max_request_indices];
    size_t * input = &input_indices[r*max_request_indices];
    for(size_t i(0); i<max_request_indices; ++i) {
      auto match = primary.find(info[i]);
      if(match != primary.end()) {
        input[i] = rank;
        offset[i] = std::distance(primary.begin(), match);
      } // if
    } // for
  } // for

  result = MPI_Alltoall(&input_indices[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(),
    &info_indices[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

  result = MPI_Alltoall(&input_offsets[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(),
    &info_offsets[0], max_request_indices,
    flecsi::dmp::mpi_typetraits<size_t>::type(), MPI_COMM_WORLD);

  // This needs a real constructor (probably)
  struct neighbor_t {
    size_t id;
    size_t rank;
    size_t offset;

    bool
    operator < (
      const neighbor_t & n
    ) const
    {
      return id < n.id;
    } // operator <

  }; // struct

  std::set<neighbor_t> full_neighbor_set;
  std::unordered_map<size_t, size_t> full_neighbor_map;

  for(auto i: primary) {
    full_neighbor_set.insert({ i, rank });
    full_neighbor_map[i] = rank;
  } // for

//if(rank == 0) {
  for(size_t r(0); r<size; ++r) {
    if(r == rank) {
      continue;
    } // if

    size_t * info = &info_indices[r*max_request_indices];
    std::cout << "result: ";
    for(size_t i(0); i<max_request_indices; ++i) {
      std::cout << info[i] << " ";
      if(info[i] != std::numeric_limits<size_t>::max()) {
        full_neighbor_set.insert({ request_indices_vector[i], info[i] });
        full_neighbor_map[request_indices_vector[i]] = info[i];
      } // if
    } // for
    std::cout << std::endl;
  } // for

  std::cout << "rank " << rank << " closure:" << std::endl;
  for(auto i: full_neighbor_set) {
    std::cout << i.id << " " << i.rank << std::endl;
  } // for
//} // if

  //--------------------------------------------------------------------------//
  // Form the vertex closure.
  //--------------------------------------------------------------------------//

  // This should move into the set utilities

  auto vertex_closure = flecsi::io::vertex_closure(sd, closure);

  if(rank == 0) {
    std::cout << "vertex closure: ";
    for(auto i: vertex_closure) {
      std::cout << i << " ";
    } // for
    std::cout << std::endl;

  //--------------------------------------------------------------------------//
  // Assign vertex ownership.
  //--------------------------------------------------------------------------//

  for(auto i: vertex_closure) {
    auto referencers = flecsi::io::vertex_referencers(sd, i);

    std::cout << "vertex " << i << " is referenced by cells: ";
    for(auto c: referencers) {
      std::cout << c << " ";
    } // for
    std::cout << std::endl;

    size_t min_rank(std::numeric_limits<size_t>::max());
    for(auto c: referencers) {
      min_rank = std::min(min_rank, full_neighbor_map[c]);
    } // for

    std::cout << "vertex " << i << " belongs to rank " << min_rank << std::endl;
  } // for 
  } // if

} // TEST

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
