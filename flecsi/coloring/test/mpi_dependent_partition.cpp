//
// Created by ollie on 4/10/18.
//

#include <cinchdevel.h>

#include <mpi.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>
#include <flecsi/coloring/mpi_utils.h>

using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;
using ::testing::ElementsAreArray;

using rank_t = std::size_t;
using cell_id_t = std::size_t;

template <typename T>
std::vector<std::vector<T>>
shuffle(const std::vector<std::vector<T>>& send_buffers)
{
  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  std::vector<int> send_counts;
  for (const auto& buf : send_buffers) {
    send_counts.push_back(buf.size());
  }

  std::vector<int> recv_counts(comm_size, 0);
  MPI_Alltoall(send_counts.data(), 1, MPI_INT,
    recv_counts.data(), 1, MPI_INT,
    MPI_COMM_WORLD);

  std::vector<std::vector<T>> recv_buffers(comm_size);
  std::vector<MPI_Request> requests;
  for (int i = 0; i < comm_size; i++) {
    if (recv_counts[i]) {
      recv_buffers[i].resize(recv_counts[i]);
      requests.push_back({});
      MPI_Irecv(recv_buffers[i].data(), recv_counts[i],
        flecsi::coloring::mpi_typetraits__<T>::type(), //MPI_INT,
        i, 66, MPI_COMM_WORLD, &requests.back());
    }
  }

  for (int i = 0; i < comm_size; i++) {
    if (send_counts[i]) {
      MPI_Send(send_buffers[i].data(), send_counts[i],
        flecsi::coloring::mpi_typetraits__<T>::type(),//MPI_INT,
        i, 66, MPI_COMM_WORLD);
    }
  }

  std::vector<MPI_Status> status(requests.size());
  MPI_Waitall(requests.size(), requests.data(), status.data());

  return recv_buffers;
}

template <typename T>
std::vector<T>
partition_by_field(const std::vector<T>& cells, const std::vector<rank_t>& colors) {
  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  std::vector<std::vector<T>> send_buffers(comm_size);

  // Essentially a sort by key
  for (int i = 0; i < colors.size(); i++) {
    send_buffers[colors[i]].push_back(cells[i]);
  }

  auto recv_buffers = shuffle(send_buffers);

  std::vector<T> result;
  for (int i = 0; i < comm_size; i++) {
    result.insert(result.end(),
      recv_buffers[i].begin(),
      recv_buffers[i].end());
  }

  return result;
}

TEST(partition_by_field, shuffle_primary) {
  // cell:  0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  // color: 0 0 1 1 0 0 1 1 2 2  3  3  2  2  3  3
  int cells[][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};
  int colors[][4] = {{0, 0, 1, 1}, {0, 0, 1, 1}, {2, 2, 3, 3}, {2, 2, 3, 3}};

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::vector<int> my_cells(&cells[rank][0], &cells[rank][4]);
  std::vector<rank_t> my_colors(&colors[rank][0], &colors[rank][4]);

  std::vector<int> colored_cells = partition_by_field(my_cells, my_colors);

  // cell:    0 1 4 5 2 3 6 7 8 9 12 13 10 11 14 15
  // [color]: 0 0 0 0 1 1 1 1 2 2  2  2  3  3  3  3
  switch (rank) {
    case 0:
      EXPECT_THAT(colored_cells, ElementsAre(0, 1, 4, 5));
      break;
    case 1:
      EXPECT_THAT(colored_cells, ElementsAre(2, 3, 6, 7));
      break;
    case 2:
      EXPECT_THAT(colored_cells, ElementsAre(8, 9, 12, 13));
      break;
    case 3:
      EXPECT_THAT(colored_cells, ElementsAre(10, 11, 14, 15));
      break;
  }
}

template <typename T0, typename T1>
std::vector<T1>
partition_by_image(const std::vector<T0>& cells,
                   const std::map<T0, std::vector<T1>>& conn) {
  // Essentially a natural join followed by distinct.
  // Insertion to std::set will make sure that duplication is
  // removed thus no additional std::unique is required.
  std::set<T1> local_results;
  std::vector<T0> nonlocal_lookups;
  for (auto cell : cells) {
    auto iter = conn.find(cell);
    if (iter != conn.end())
      // lookups that can be resolved locally.
      local_results.insert((*iter).second.begin(), (*iter).second.end());
    else {
      // the lookup can not be resolved locally. we need to ask other ranks.
      nonlocal_lookups.push_back(cell);
    }
  }

  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  // Alltoall communication for each rank to know how many nonlocal_lookups
  // lookups other ranks have.
  std::vector<std::vector<T0>> requests_for_remote(comm_size, nonlocal_lookups);

  auto requests_from_remote = shuffle(requests_for_remote);

  std::vector<std::set<T1>> results_for_remote(comm_size);
  for (int r = 0; r < comm_size; r++) {
    for (auto key : requests_from_remote[r]) {
      auto iter = conn.find(key);
      if (iter != conn.end()) {
        results_for_remote[r].insert((*iter).second.begin(),
          (*iter).second.end());
      }
    }
  }

  // An other round of alltoall to send back results_for_remote.
  // copy results from std::set to std::vector first
  std::vector<std::vector<T1>> send_buffers(comm_size);
  for (int r = 0; r < comm_size; r++) {
    send_buffers[r] = std::vector<T1>(results_for_remote[r].begin(),
      results_for_remote[r].end());
  }

  auto results_from_remote = shuffle(send_buffers);

  // merge both locally and remotely resolved lookups.
  for (int r = 0; r < comm_size; r++) {
    local_results.insert(results_from_remote[r].begin(),
      results_from_remote[r].end());
  }

  return std::vector<T1>{local_results.begin(), local_results.end()};
}

TEST(partition_by_image, local_connectivity) {
  // primary cells: 0 1 4 5
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell {
    // R0
    {{0, {1, 4, 5}}, {1, {0, 4, 5, 2, 6}},
    {4, {0, 1, 5, 8, 9}}, {5, {0, 1, 2, 4, 6, 8, 9, 10}}},
    // R1
    {{2, {1, 3, 5, 6, 7}}, {3, {2, 6, 7}},
    {6, {1, 2, 3, 5, 7, 9, 10, 11}}, {7, {2, 3, 6, 10, 11}}},
    // R2
    {{8, {4, 5, 9, 12, 13}}, {9, {4, 5, 6, 8, 10, 12, 13, 14}},
    {12, {8, 9, 13}}, {13, {8, 9, 10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9, 11, 13, 14, 15}}, {11, {6, 7, 10, 14, 15}},
    {14, {9, 10, 11, 13, 15}}, {15, {10, 11, 14}}},
  };

  // Union({from -> to})
  // May need some kind of std::unique
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // only using local portion of the global cell to cell connectivity.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // closure: 0 1 4 5 2 6 8 9 10
  switch (rank) {
    case 0:
      EXPECT_THAT(closure, ElementsAre(0, 1, 2, 4, 5, 6, 8, 9, 10));
      break;
    case 1:
      EXPECT_THAT(closure, ElementsAre(1, 2, 3, 5, 6, 7, 9, 10, 11));
      break;
    case 2:
      EXPECT_THAT(closure, ElementsAre(4, 5, 6, 8, 9, 10, 12, 13, 14));
      break;
    case 3:
      EXPECT_THAT(closure, ElementsAre(5, 6, 7, 9, 10, 11, 13, 14, 15));
      break;
  }
}

template <typename T0, typename T1>
std::map<T0, std::vector<T1>>
partition_by_image_grouped(const std::vector<T0>& cells,
                           const std::map<T0, std::vector<T1>>& conn) {
  // Essentially a natural join followed by a groupbykey.
  // Insertion to std::set will make sure that duplication is
  // removed thus no additional std::unique is required.
  //std::set<T1> local_results;
  std::map<T0, std::vector<T1>> local_results;
  std::vector<T0> nonlocal_lookups;
  for (auto cell : cells) {
    auto iter = conn.find(cell);
    if (iter != conn.end()) {
      // lookups that can be resolved locally.
      local_results.insert({cell, {(*iter).second.begin(), (*iter).second.end()}});
    } else {
      // the lookup can not be resolved locally. we need to ask other ranks.
      nonlocal_lookups.push_back(cell);
    }
  }

  // Alltoall communication for each rank to know how many nonlocal
  // lookups other ranks have.
  int comm_size;
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  std::vector<std::vector<T0>> requests_for_remote(comm_size, nonlocal_lookups);

  auto request_from_remote = shuffle(requests_for_remote);

  std::vector<std::vector<T0>> result_keys(comm_size);
  std::vector<std::vector<T1>> result_values(comm_size);

  for (int r = 0; r < comm_size; r++) {
    for (auto key : request_from_remote[r]) {
      auto iter = conn.find(key);
      if (iter != conn.end()) {
        for (auto value : iter->second) {
          result_keys[r].push_back(key);
          result_values[r].push_back(value);
        }
      }
    }
  }

  auto remote_keys = shuffle(result_keys);
  auto remote_values = shuffle(result_values);

  for (int r = 0; r < comm_size; r++) {
    for (int i = 0; i < remote_keys[r].size(); i++) {
      auto key = remote_keys[r][i];
      auto value = remote_values[r][i];
      local_results[key].push_back(value);
    }
  }

  return local_results;
}


template <typename T>
std::vector<T>
partition_by_difference(const std::vector<T>& left, const std::vector<T>& right)
{
  std::set<T> setA(left.begin(), left.end());
  std::set<T> setB(right.begin(), right.end());

  std::set<T> result;
  std::set_difference(setA.begin(), setA.end(),
    setB.begin(), setB.end(),
    std::inserter(result, result.begin()));

  return std::vector<T>{result.begin(), result.end()};
}

TEST(partition_by_difference, ghost_cell) {
  // ghost cells = closure - primary
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell {
    // R0
    {{0, {1, 4, 5}}, {1, {0, 4, 5, 2, 6}},
    {4, {0, 1, 5, 8, 9}}, {5, {0, 1, 2, 4, 6, 8, 9, 10}}},
    // R1
    {{2, {1, 3, 5, 6, 7}}, {3, {2, 6, 7}},
    {6, {1, 2, 3, 5, 7, 9, 10, 11}}, {7, {2, 3, 6, 10, 11}}},
    // R2
    {{8, {4, 5, 9, 12, 13}}, {9, {4, 5, 6, 8, 10, 12, 13, 14}},
    {12, {8, 9, 13}}, {13, {8, 9, 10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9, 11, 13, 14, 15}}, {11, {6, 7, 10, 14, 15}},
    {14, {9, 10, 11, 13, 15}}, {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // only using local portion of the global cell to cell connectivity.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // ghost: 2 6 8 9 10
  auto ghost = partition_by_difference(closure, primary[rank]);

  switch (rank) {
    case 0:
      EXPECT_THAT(ghost, ElementsAre(2, 6, 8, 9, 10));
      break;
    case 1:
      EXPECT_THAT(ghost, ElementsAre(1, 5, 9, 10, 11));
      break;
    case 2:
      EXPECT_THAT(ghost, ElementsAre(4, 5, 6, 10, 14));
      break;
    case 3:
      EXPECT_THAT(ghost, ElementsAre(5, 6, 7, 9, 13));
      break;
  }
}

TEST(partition_by_image, shared_cells) {
  // ghost_cells: 2 6 8 9 10
  // cell to cell => We need global cell to cell connectivity
  // This requires either to be broadcasted or an all to all
  // communication.

  // ghost cells = closure - primary
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell {
    // R0
    {{0, {1, 4, 5}}, {1, {0, 4, 5, 2, 6}},
      {4, {0, 1, 5, 8, 9}}, {5, {0, 1, 2, 4, 6, 8, 9, 10}}},
    // R1
    {{2, {1, 3, 5, 6, 7}}, {3, {2, 6, 7}},
      {6, {1, 2, 3, 5, 7, 9, 10, 11}}, {7, {2, 3, 6, 10, 11}}},
    // R2
    {{8, {4, 5, 9, 12, 13}}, {9, {4, 5, 6, 8, 10, 12, 13, 14}},
      {12, {8, 9, 13}}, {13, {8, 9, 10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9, 11, 13, 14, 15}}, {11, {6, 7, 10, 14, 15}},
      {14, {9, 10, 11, 13, 15}}, {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // only using local portion of the global cell to cell connectivity.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // ghost: 2 6 8 9 10
  auto ghost = partition_by_difference(closure, primary[rank]);

  // neighbor of ghost cells
  auto neighbor_of_ghost = partition_by_image(ghost, cell2cell[rank]);
  switch (rank) {
    case 0:
      EXPECT_THAT(neighbor_of_ghost, ElementsAreArray({1, 2, 3, 4 ,5, 6, 7, 8, 9,
                                                       10, 11, 12, 13, 14, 15}));
      break;
    case 1:
      EXPECT_THAT(neighbor_of_ghost, ElementsAreArray({0, 1, 2, 4 ,5, 6, 7, 8, 9,
                                                       10, 11, 12, 13, 14, 15}));
      break;
    case 2:
      EXPECT_THAT(neighbor_of_ghost, ElementsAreArray({0, 1, 2, 3, 4, 5, 6, 7, 8,
                                                       9, 10, 11, 13, 14, 15}));
      break;
    case 3:
      EXPECT_THAT(neighbor_of_ghost, ElementsAreArray({0, 1, 2, 3, 4 ,5, 6, 7, 8, 9,
                                                       10, 11, 12, 13, 14}));

      break;
  }
}

template <typename T>
std::vector<T>
partition_by_intersection(const std::vector<T>& left, const std::vector<T>& right)
{
  std::set<T> setA(left.begin(), left.end());
  std::set<T> setB(right.begin(), right.end());

  std::set<T> result;
  std::set_intersection(setA.begin(), setA.end(),
    setB.begin(), setB.end(),
    std::inserter(result, result.begin()));

  return std::vector<T>{result.begin(), result.end()};
}

TEST(partition_by_intersection, shared_cells) {
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell{
    // R0
    { {0,  {1, 4, 5}},
      {1,  {0, 4, 5,  2,  6}},
      {4,  {0, 1,  5,  8,  9}},
      {5,  {0,  1,  2,  4,  6, 8, 9, 10}}},
    // R1
    {{2,  {1, 3, 5, 6,  7}},
      {3,  {2, 6, 7}},
      {6,  {1, 2,  3,  5,  7, 9, 10, 11}},
      {7,  {2,  3,  6,  10, 11}}},
    // R2
    {{8,  {4, 5, 9, 12, 13}},
      {9,  {4, 5, 6,  8,  10, 12, 13, 14}},
      {12, {8, 9,  13}},
      {13, {8,  9,  10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9,  11, 13, 14, 15}},
      {11, {6, 7, 10, 14, 15}},
      {14, {9, 10, 11, 13, 15}},
      {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Compute the dependency closure of the primary cells.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. They are also ghost cells.
  auto ghost = partition_by_difference(closure, primary[rank]);

  // nearest neighbor closure of ghost cells
  auto neighbor_of_ghost = partition_by_image(ghost, cell2cell[rank]);

  // primary intersect neighbors of ghost cells = shared
  auto shared = partition_by_intersection(primary[rank], neighbor_of_ghost);
  // shared cells: 1 4 5
  switch (rank) {
    case 0:
      EXPECT_THAT(shared, ElementsAre(1, 4, 5));
      break;
    case 1:
      EXPECT_THAT(shared, ElementsAre(2, 6, 7));
      break;
    case 2:
      EXPECT_THAT(shared, ElementsAre(8, 9, 13));
      break;
    case 3:
      EXPECT_THAT(shared, ElementsAre(10, 11, 14));
      break;
  }
}

TEST(partition_by_difference, exclusive_cells) {
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell{
    // R0
    { {0,  {1, 4, 5}},
      {1,  {0, 4, 5,  2,  6}},
      {4,  {0, 1,  5,  8,  9}},
      {5,  {0,  1,  2,  4,  6, 8, 9, 10}}},
    // R1
    {{2,  {1, 3, 5, 6,  7}},
      {3,  {2, 6, 7}},
      {6,  {1, 2,  3,  5,  7, 9, 10, 11}},
      {7,  {2,  3,  6,  10, 11}}},
    // R2
    {{8,  {4, 5, 9, 12, 13}},
      {9,  {4, 5, 6,  8,  10, 12, 13, 14}},
      {12, {8, 9,  13}},
      {13, {8,  9,  10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9,  11, 13, 14, 15}},
      {11, {6, 7, 10, 14, 15}},
      {14, {9, 10, 11, 13, 15}},
      {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Compute the dependency closure of the primary cells.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. They are also ghost cells.
  auto ghost = partition_by_difference(closure, primary[rank]);

  // nearest neighbor closure of ghost cells
  auto neighbor_of_ghost = partition_by_image(ghost, cell2cell[rank]);

  // primary intersect neighbors of ghost cells = shared
  auto shared = partition_by_intersection(primary[rank], neighbor_of_ghost);

  auto exclusive = partition_by_difference(primary[rank], shared);
  // shared cells: 1 4 5
  switch (rank) {
    case 0:
      EXPECT_THAT(exclusive, ElementsAre(0));
      break;
    case 1:
      EXPECT_THAT(exclusive, ElementsAre(3));
      break;
    case 2:
      EXPECT_THAT(exclusive, ElementsAre(12));
      break;
    case 3:
      EXPECT_THAT(exclusive, ElementsAre(15));
      break;
  }
}

TEST(partition_by_image, ghost_owner) {
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell{
    // R0
    { {0,  {1, 4, 5}},
      {1,  {0, 4, 5,  2,  6}},
      {4,  {0, 1,  5,  8,  9}},
      {5,  {0,  1,  2,  4,  6, 8, 9, 10}}},
    // R1
    {{2,  {1, 3, 5, 6,  7}},
      {3,  {2, 6, 7}},
      {6,  {1, 2,  3,  5,  7, 9, 10, 11}},
      {7,  {2,  3,  6,  10, 11}}},
    // R2
    {{8,  {4, 5, 9, 12, 13}},
      {9,  {4, 5, 6,  8,  10, 12, 13, 14}},
      {12, {8, 9,  13}},
      {13, {8,  9,  10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9,  11, 13, 14, 15}},
      {11, {6, 7, 10, 14, 15}},
      {14, {9, 10, 11, 13, 15}},
      {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Compute the dependency closure of the primary cells.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. They are also ghost cells.
  auto ghost = partition_by_difference(closure, primary[rank]);

  // nearest neighbor closure of ghost cells
  auto neighbor_of_ghost = partition_by_image(ghost, cell2cell[rank]);

  // primary intersect neighbors of ghost cells = shared
  auto shared = partition_by_intersection(primary[rank], neighbor_of_ghost);

  std::map<cell_id_t, std::vector<rank_t>> shared_rank;
  for (auto cell : shared) {
    std::vector<rank_t> r;
    r.push_back(rank);
    shared_rank.insert({cell, r});
  }

  auto ghost_owners = partition_by_image(ghost, shared_rank);

  switch (rank) {
    case 0:
      EXPECT_THAT(ghost_owners, ElementsAre(1, 2, 3));
      break;
    case 1:
      EXPECT_THAT(ghost_owners, ElementsAre(0, 2, 3));
      break;
    case 2:
      EXPECT_THAT(ghost_owners, ElementsAre(0, 1, 3));
      break;
    case 3:
      EXPECT_THAT(ghost_owners, ElementsAre(0, 1, 2));
      break;
  }
}


TEST(partition_by_image_grouped, shared_users) {
  std::vector<std::vector<cell_id_t>> primary{
    {0,  1,  4,  5},
    {2,  3,  6,  7},
    {8,  9,  12, 13},
    {10, 11, 14, 15}
  };

  // global cell 2 cell connectivity
  std::vector<std::map<cell_id_t, std::vector<cell_id_t>>> cell2cell{
    // R0
    { {0,  {1, 4, 5}},
      {1,  {0, 4, 5,  2,  6}},
      {4,  {0, 1,  5,  8,  9}},
      {5,  {0,  1,  2,  4,  6, 8, 9, 10}}},
    // R1
    {{2,  {1, 3, 5, 6,  7}},
      {3,  {2, 6, 7}},
      {6,  {1, 2,  3,  5,  7, 9, 10, 11}},
      {7,  {2,  3,  6,  10, 11}}},
    // R2
    {{8,  {4, 5, 9, 12, 13}},
      {9,  {4, 5, 6,  8,  10, 12, 13, 14}},
      {12, {8, 9,  13}},
      {13, {8,  9,  10, 12, 14}}},
    // R3
    {{10, {5, 6, 7, 9,  11, 13, 14, 15}},
      {11, {6, 7, 10, 14, 15}},
      {14, {9, 10, 11, 13, 15}},
      {15, {10, 11, 14}}},
  };

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Compute the dependency closure of the primary cells.
  auto closure = partition_by_image(primary[rank], cell2cell[rank]);

  // Subtracting out the initial set leaves just the nearest
  // neighbors. They are also ghost cells.
  auto ghost = partition_by_difference(closure, primary[rank]);

  // nearest neighbor closure of ghost cells
  auto neighbor_of_ghost = partition_by_image(ghost, cell2cell[rank]);

  // primary intersect neighbors of ghost cells = shared
  auto shared = partition_by_intersection(primary[rank], neighbor_of_ghost);

  // TODO: we probably need to define a partition_by_image_grouped
  // TODO: do we also want another overload that take std::map<cell_id_t, rank_t>?
  // TODO: what we want is to shared.join(ghost_rank).groupby(shared)
  std::map<cell_id_t, std::vector<rank_t>> ghost_rank;
  for (auto cell : ghost) {
    std::vector<rank_t> r;
    r.push_back(rank);
    ghost_rank.insert({cell, r});
  }

  auto shared_users = partition_by_image_grouped(shared, ghost_rank);

  switch (rank) {
    case 0:
      EXPECT_THAT(shared_users[1], ElementsAre(1));
      EXPECT_THAT(shared_users[4], ElementsAre(2));
      EXPECT_THAT(shared_users[5], ElementsAre(1, 2, 3));
      break;
    case 1:
      EXPECT_THAT(shared_users[2], ElementsAre(0));
      EXPECT_THAT(shared_users[6], ElementsAre(0, 2, 3));
      EXPECT_THAT(shared_users[7], ElementsAre(3));
      break;
    case 2:
      EXPECT_THAT(shared_users[8], ElementsAre(0));
      EXPECT_THAT(shared_users[9], ElementsAre(0, 1, 3));
      EXPECT_THAT(shared_users[13], ElementsAre(3));
      break;
    case 3:
      EXPECT_THAT(shared_users[10], ElementsAre(0, 1, 2));
      EXPECT_THAT(shared_users[11], ElementsAre(1));
      EXPECT_THAT(shared_users[14], ElementsAre(2));
      break;
  }
}
