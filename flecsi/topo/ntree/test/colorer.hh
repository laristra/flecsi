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

#include "flecsi/util/geometry/point.hh"

namespace tree_colorer {

template<typename T, typename KEY, int D>
class colorer
{
  using key_t = KEY;
  using type_t = T;
  static constexpr int dimension = D;
  using point_t = flecsi::util::point<double, dimension>;
  using range_t = std::array<point_t, 2>;

  // Number of octets used communications
  static const size_t noct = 256 * 1024;

  colorer(){};

public:
  /**
   * @brief      Sorting of the input particles or current particles using MPI.
   * This method is composed of several steps to implement the quick sort:
   * - Each process sorts its local particles
   * - Each process generate a subset of particles to fit the byte size limit
   * to send
   * - Each subset if send to the master (Here 0) who generates the pivot for
   * quick sort
   * - Pivot are send to each processes and they create buckets based on the
   * pivot
   * - Each bucket is send to the owner
   * - Each process sorts its local particles again
   * This is the first implementation, it can be long for the first sorting but
   * but then as the particles does not move very fast, the particles on the
   * edge are the only ones shared
   *
   * @param      ents    Local entities for this process
   * @param[in]  nents   The global number of entities
   */
  static void mpi_qsort(std::vector<T> & ents, int nents) {
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Sort types based on key
    std::sort(ents.begin(), ents.end(), [](auto & left, auto & right) {
      if(left.key() < right.key()) {
        return true;
      }
      if(left.key() == right.key()) {
        return left.id() < right.id();
      }
      return false;
    }); // sort

    if(size == 1) {
      return;
    } // if

    // Generate splitters from root node
    std::vector<std::pair<key_t, int64_t>> splitters_;

    splitters_.clear();
    std::vector<int> scount(size);
    generate_splitters_samples_(splitters_, ents, nents);

    int cur_proc = 0;
    assert(static_cast<int>(splitters_.size()) == (size - 1 + 2));

    int64_t nlents = ents.size();
    for(int64_t i = 0L; i < nlents; ++i) {
      if(ents[i].key() >= splitters_[cur_proc].first &&
         ents[i].key() < splitters_[cur_proc + 1].first) {
        scount[cur_proc]++;
      }
      else {
        i--;
        cur_proc++;
      }
    }

    std::vector<type_t> recvbuffer;
    // Direct exchange using point to point
    mpi_alltoallv_p2p_(scount, ents, recvbuffer);

    ents.clear();
    ents = recvbuffer;

    // Sort the bodies after reception
    std::sort(ents.begin(), ents.end(), [](auto & left, auto & right) {
      if(left.key() < right.key()) {
        return true;
      }
      if(left.key() == right.key()) {
        return left.id() < right.id();
      }
      return false;
    }); // sort

#ifdef OUTPUT
    std::vector<int> totalprocbodies;
    totalprocbodies.resize(size);
    int mybodies = rbodies.size();
    // Share the final array size of everybody
    MPI_Allgather(
      &mybodies, 1, MPI_INT, &totalprocbodies[0], 1, MPI_INT, MPI_COMM_WORLD);
#ifdef OUTPUT_TREE_INFO
    std::ostringstream oss;
    oss << "Repartition: ";
    for(auto num : totalprocbodies)
      oss << num << ";";
    clog_one(trace) << oss.str() << std::endl;
#endif
#endif // OUTPUT
  } // mpi_qsort

  /**
   * @brief      Compute the global range coordinates of particles
   *
   * @param      ents           Local entities of this process
   * @param      range          Global range of coordinates
   */
  static void mpi_compute_range(std::vector<type_t> & ents,
    std::array<point_t, 2> & range) {

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Compute the local range
    range_t lrange;

    lrange[1] = ents.back().coordinates();
    lrange[0] = ents.back().coordinates();

    {
      range_t trange;
      trange[1] = ents.back().coordinates();
      trange[0] = ents.back().coordinates();

      for(size_t i = 0; i < ents.size(); ++i) {
        for(size_t d = 0; d < dimension; ++d) {

          if(ents[i].coordinates()[d] + ents[i].radius() > trange[1][d])
            trange[1][d] = ents[i].coordinates()[d] + ents[i].radius();

          if(ents[i].coordinates()[d] - ents[i].radius() < trange[0][d])
            trange[0][d] = ents[i].coordinates()[d] - ents[i].radius();
        }
      }
      for(size_t d = 0; d < dimension; ++d) {
        lrange[1][d] = std::max(lrange[1][d], trange[1][d]);
        lrange[0][d] = std::min(lrange[0][d], trange[0][d]);
      }
    }

    double max[dimension];
    double min[dimension];
    for(size_t i = 0; i < dimension; ++i) {
      max[i] = lrange[1][i];
      min[i] = lrange[0][i];
    }

    // Do the MPI Reduction
    MPI_Allreduce(
      MPI_IN_PLACE, max, dimension, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(
      MPI_IN_PLACE, min, dimension, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);

    for(size_t d = 0; d < dimension; ++d) {
      range[0][d] = min[d];
      range[1][d] = max[d];
    }
  } // mpi_compute_range

private:
  /**
   * @brief      Use in mpi_qsort to generate the splitters to sort the
   * particles in the quick sort algorithm In this function we take some
   * samplers of the total particles and the root determines the splitters This
   * version is based on the sample splitter algorithm but we generate more
   * samples on each process
   *
   * @param      splitters  The splitters used in the qsort in mpi_qsort
   * @param[in]  ents  The local bodies of the process
   */
  static void generate_splitters_samples_(
    std::vector<std::pair<key_t, int64_t>> & splitters,
    std::vector<T> & ents,
    const int64_t & nents) {

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create a vector for the samplers
    std::vector<std::pair<key_t, int64_t>> keys_sample;
    // Number of elements for sampling
    // In this implementation we share up to 256KB to
    // the master.
    size_t maxnsamples = noct / sizeof(std::pair<key_t, int64_t>);
    int64_t nvalues = ents.size();
    size_t nsample = maxnsamples * ((double)nvalues / (double)nents);
    if(nvalues < (int64_t)nsample) {
      nsample = nvalues;
    }

    for(size_t i = 0; i < nsample; ++i) {
      int64_t position = (nvalues / (nsample + 1.)) * (i + 1.);
      keys_sample.push_back(
        std::make_pair(ents[position].key(), ents[position].id()));
    } // for
    assert(keys_sample.size() == (size_t)nsample);

    std::vector<std::pair<key_t, int64_t>> master_keys;
    std::vector<int> master_recvcounts;
    std::vector<int> master_offsets;
    int master_nkeys = 0;

    if(rank == 0) {
      master_recvcounts.resize(size);
    } // if

    // Echange the number of samples
    MPI_Gather(&nsample,
      1,
      MPI_INT,
      &master_recvcounts[0],
      1,
      MPI_INT,
      0,
      MPI_COMM_WORLD);

    // Master
    // Sort the received keys and create the pivots
    if(rank == 0) {
      master_offsets.resize(size);
      master_nkeys = 0;
      for(size_t i = 0; i < master_recvcounts.size(); ++i) {
        master_nkeys += master_recvcounts[i];
      }
      if(nents < master_nkeys) {
        master_nkeys = nents;
      }
      // Number to receiv from each process
      for(int i = 0; i < size; ++i) {
        master_recvcounts[i] *= sizeof(std::pair<key_t, int64_t>);
        if(i == 0)
          master_offsets[i] = master_recvcounts[i];
        else
          master_offsets[i] = master_recvcounts[i] + master_offsets[i - 1];
      } // for
      master_offsets.insert(master_offsets.begin(), 0);
      master_keys.resize(master_nkeys);
    } // if

    MPI_Gatherv(&keys_sample[0],
      nsample * sizeof(std::pair<key_t, int64_t>),
      MPI_BYTE,
      &master_keys[0],
      &master_recvcounts[0],
      &master_offsets[0],
      MPI_BYTE,
      0,
      MPI_COMM_WORLD);

    // Generate the splitters, add zero and max keys
    splitters.resize(size - 1 + 2);
    if(rank == 0) {
      std::sort(
        master_keys.begin(), master_keys.end(), [](auto & left, auto & right) {
          if(left.first < right.first) {
            return true;
          }
          if(left.first == right.first) {
            return left.second < right.second;
          }
          return false;
        });

      splitters[0].first = key_t::min();
      splitters[0].second = 0L;
      splitters[size].first = key_t::max();
      splitters[size].second = LONG_MAX;

      for(int i = 0; i < size - 1; ++i) {
        int64_t position = (master_nkeys / size) * (i + 1);
        splitters[i + 1] = master_keys[position];
        assert(splitters[i + 1].first > splitters[0].first &&
               splitters[i + 1].first < splitters[size].first);
      } // for
    } // if

    // Bradcast the splitters
    MPI_Bcast(&splitters[0],
      (size - 1 + 2) * sizeof(std::pair<key_t, int64_t>),
      MPI_BYTE,
      0,
      MPI_COMM_WORLD);
  } // generate_splitters_samples_

  static void mpi_alltoallv_p2p_(std::vector<int> & sendcount,
    std::vector<type_t> & sendbuffer,
    std::vector<type_t> & recvbuffer) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> recvcount(size), recvoffsets(size), sendoffsets(size);
    // Exchange the send count
    MPI_Alltoall(
      &sendcount[0], 1, MPI_INT, &recvcount[0], 1, MPI_INT, MPI_COMM_WORLD);
    for(int i = 0; i < size; ++i) {
      if(i == 0) {
        recvoffsets[i] = recvcount[i];
        sendoffsets[i] = sendcount[i];
      }
      else {
        recvoffsets[i] = recvcount[i] + recvoffsets[i - 1];
        sendoffsets[i] = sendcount[i] + sendoffsets[i - 1];
      }
    }
    recvoffsets.insert(recvoffsets.begin(), 0);
    sendoffsets.insert(sendoffsets.begin(), 0);
    // Set the recvbuffer to the right size
    recvbuffer.resize(recvoffsets.back());
    // Transform the offsets for bytes
    for(int i = 0; i < size; ++i) {
      sendcount[i] *= sizeof(type_t);
      assert(sendcount[i] >= 0);
      recvcount[i] *= sizeof(type_t);
      assert(recvcount[i] >= 0);
      sendoffsets[i] *= sizeof(type_t);
      assert(sendoffsets[i] >= 0);
      recvoffsets[i] *= sizeof(type_t);
      assert(recvoffsets[i] >= 0);
    } // for
    std::vector<MPI_Status> status(size);
    std::vector<MPI_Request> request(size);
    for(int i = 0; i < size; ++i) {
      if(sendcount[i] != 0) {
        char * start = (char *)&(sendbuffer[0]);
        MPI_Isend(start + sendoffsets[i],
          sendcount[i],
          MPI_BYTE,
          i,
          0,
          MPI_COMM_WORLD,
          &request[i]);
      }
    }
    for(int i = 0; i < size; ++i) {
      if(recvcount[i] != 0) {
        char * start = (char *)&(recvbuffer[0]);
        MPI_Recv(start + recvoffsets[i],
          recvcount[i],
          MPI_BYTE,
          i,
          MPI_ANY_TAG,
          MPI_COMM_WORLD,
          &status[i]);
      }
      if(sendcount[i] != 0) {
        MPI_Wait(&request[i], &status[i]);
      }
    } // for
  } // mpi_alltoallv_p2p

}; // colorer

} // namespace tree_colorer
