/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef simple_box_colorer_h
#define simple_box_colorer_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include "mpi.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>

#include "flecsi/topo/structured/box_colorer.hh"

namespace flecsi {
namespace topo {
namespace structured_impl {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//
template<size_t D>
struct simple_box_colorer : public box_colorer<D> {
  //! Default constructor
  simple_box_colorer() {}

  //! Copy constructor (disabled)
  simple_box_colorer(const simple_box_colorer &) = delete;

  //! Assignment operator (disabled)
  simple_box_colorer & operator=(const simple_box_colorer &) = delete;

  //! Destructor
  ~simple_box_colorer() {}

  // Simple partitioning algorithm for structured meshes. Partitions the highest
  // dimensional entities in the  mesh into as many blocks as number of input 
  // ranks.
  box_coloring color(
      size_t grid_size[D],
      size_t nghost_layers,
      size_t ndomain_layers,
      size_t thru_dim,
      size_t ncolors[D]
      ) override {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Assert that the number of partitions is equal to number of ranks
    int count = 1;
    for (size_t nc = 0; nc < D; ++nc)
      count = static_cast<int>(count * ncolors[nc]);

    // Dont assert, abort.  Because assertions are removed with -DNDEBUG, and
    // this does not provided any performance benefit.
    // Note: could use clog_err, but its behaviour is not as expected (i.e. does
    // not always show output).
    if ( count != size ) {
      std::cerr << "Number of processors (" << size << ") must equal the"
        << " total number of domains (" << count << ")" << std::endl;
    }
    if ( thru_dim >= D ) {
      std::cerr << "Through dimension (" << thru_dim << ") must be less than"
        << " total number of dimensions (" << D  << ")" << std::endl;
    }

    // Obtain indices of the current rank
    size_t idx[D];
    get_indices(ncolors, rank, idx);

    // Step 1: Create global bounding boxes for domain including domain halo layers
    box_core domain(D);
    for (size_t i = 0; i < D; ++i) {
      domain.lowerbnd[i] = ndomain_layers;
      domain.upperbnd[i] = grid_size[i] + ndomain_layers - 1;
    }

    // Compute the strides of the global mesh
    std::vector<size_t> strides(D); 
    for (size_t i = 0; i < D; ++i) {
      strides[i] = grid_size[i] + 2*ndomain_layers;
    }

    // Step 2: Compute the primary box bounds for the current rank
    auto pbox = create_primary_box(domain, ncolors, idx);

    // Step 2: Create colored box type and set its primary box
    // info to the one created in step 1.
    box_coloring colbox_cells;
    colbox_cells.mesh_dim = D;
    colbox_cells.entity_dim = D;
    colbox_cells.primary_dim = D;
    colbox_cells.num_boxes = 1; 
    colbox_cells.resize(); 

    colbox_cells.partition[0].box = pbox;
    colbox_cells.partition[0].nghost_layers = nghost_layers;
    colbox_cells.partition[0].ndomain_layers = ndomain_layers;
    colbox_cells.partition[0].thru_dim = thru_dim;
    
    colbox_cells.strides[0] = strides;

    //Set the onbnd vector to false 
    for(size_t i = 0; i < 2*D; ++i)
     colbox_cells.partition[0].onbnd.push_back(false);

    for (size_t i = 0; i < D; ++i) {
      if (pbox.lowerbnd[i] == domain.lowerbnd[i])
        colbox_cells.partition[0].onbnd[2*i] = true;
      if (pbox.upperbnd[i] == domain.upperbnd[i])
        colbox_cells.partition[0].onbnd[2*i+1] = true;
    }
  
    // Step 3: Compute exclusive and shared boxes from primary box info.
    create_exclusive_and_shared_boxes(colbox_cells, ncolors, idx, rank);

    // Step 4: Compute ghost boxes
    create_ghost_boxes(colbox_cells, ncolors, idx);

    // Step 5: Compute domain halo boxes
    create_domain_halo_boxes(colbox_cells);

    //Step 6: Compute the overlaying bounding box
    compute_overlaying_bounding_box(colbox_cells);

    return colbox_cells;
  } // color


  // Compute the aggregate information from a colored box. 
  box_aggregate_info create_aggregate_info(box_coloring &cbox)
  {
    box_aggregate_info colinfo; 
 
    //#exclusive entities
    colinfo.exclusive = cbox.exclusive[0].domain.size();

    //#shared entities
    colinfo.shared = 0; 
    std::set<size_t> shared_ranks; 
    for (size_t i = 0; i < cbox.shared[0].size(); i++)
    {
      colinfo.shared += cbox.shared[0][i].domain.size();
      for (size_t j = 0; j < cbox.shared[0][i].colors.size(); j++)
         shared_ranks.insert(cbox.shared[0][i].colors[j]);
    }
    colinfo.shared_users = shared_ranks; 

    //#ghost entities
    colinfo.ghost = 0; 
    std::set<size_t> ghost_ranks; 
    for (size_t i = 0; i < cbox.ghost[0].size(); i++)
    {
      colinfo.ghost += cbox.ghost[0][i].domain.size();
      for (size_t j = 0; j < cbox.ghost[0][i].colors.size(); j++)
         ghost_ranks.insert(cbox.ghost[0][i].colors[j]);
    }
    colinfo.ghost_owners = ghost_ranks;

    //Send the overlay boxes on the current rank to all the shared ranks
    size_t sz = 2*D*cbox.num_boxes; 
    int sbuf[sz], count = 0; 
    for (size_t i = 0; i < cbox.num_boxes; i++){
      for (size_t d = 0; d < D; d++){
        sbuf[count] = cbox.overlay[i].lowerbnd[d];
        count += 1; 
      }
      for (size_t d = 0; d < D; d++){
        sbuf[count] = cbox.overlay[i].upperbnd[d];
        count += 1; 
      }
    }

    //Post receives
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> rbuf[size]; 
  
    std::vector<MPI_Request> requests;
    for (size_t i = 0; i < size; i++)
    {
        if (i != rank){
          MPI_Request request; 
          MPI_Irecv((void*)&(rbuf[i]), sz, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
          requests.push_back(request);
       } 
    }

    //Post isends 
    std::set<size_t>::iterator it; 
    for (it = shared_ranks.begin(); it != shared_ranks.end(); ++it)
    {
        MPI_Request request; 
        MPI_Isend((void*)&(sbuf), sz, MPI_INT, *it, 0, MPI_COMM_WORLD, &request);
    }

    // Wait for all receives to complete
    if (requests.size() > 0)
    {
      std::vector<MPI_Status> statuses(requests.size());
      MPI_Waitall(requests.size(), &(requests[0]), &(statuses[0]));
    }

    //Add received data to map
    for (size_t i = 0; i < size; ++i)
    {
      if (rbuf[i].size() > 0){
       for (size_t n = 0; n < cbox.num_boxes; ++n){
          box_core ob(D); 
 
          for (size_t d = 0; d < D; d++){
           ob.lowerbnd[d]= rbuf[i][2*D*n+d];
           ob.upperbnd[d]= rbuf[i][2*D*n+d+D];
        }
        colinfo.ghost_overlays[i].push_back(ob);      
      }
     }
    }
 
   return colinfo;  
  }//create_aggregate_color_info 

  
private:
  auto create_primary_box(box_core & domain, size_t ncolors[D], size_t idx[D]) {
    box_core pbox(D);
    for (size_t i = 0; i < D; ++i) {
      size_t N = domain.upperbnd[i] - domain.lowerbnd[i] + 1;
      size_t ne = N / ncolors[i];

      pbox.lowerbnd[i] = domain.lowerbnd[i] + ne * idx[i];

      if (idx[i] != ncolors[i] - 1)
        pbox.upperbnd[i] = domain.lowerbnd[i] + ne * (idx[i] + 1) - 1;
      else
        pbox.upperbnd[i] = domain.upperbnd[i];
    }

    return pbox;
  } // create_primary_box

  template<size_t D_ = D>
  typename std::enable_if<D_ == 1>::type create_exclusive_and_shared_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    size_t TD = colbox.partition[0].thru_dim;
    std::cout<<"ncolors = "<<ncolors[0]<<std::endl;

    // Compute bounds for exclusive box
    box_color ecbox(D);
    box_core ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2*i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2*i + 1])
        ebox.upperbnd[i] -= hl;
    }
    ecbox.domain = ebox; 
    ecbox.colors.emplace_back(rank); 
    colbox.exclusive[0] = ecbox;

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = pbox.lowerbnd[i];
      IM[i][1] = ebox.lowerbnd[i];
      IM[i][2] = ebox.upperbnd[i];
      IM[i][3] = pbox.upperbnd[i];
    }

    box_core sbox(D);
    box_color scbox(D);
    size_t ind[D], idx_new[D], cval;

    {
      for (size_t i = 0; i < 3; ++i) {
        bool flag;
        if (i == 1)
          flag = false;
        else if (IM[0][i] != IM[0][i + 1])
          flag = true;
        else if (
            (i == 1) && (IM[0][i] == IM[0][i + 1]))
          flag = true;
        else
          flag = false;

        if (flag) {
          sbox.lowerbnd[0] = IM[0][i] + il[i];
          sbox.upperbnd[0] = IM[0][i + 1] - iu[i];

          scbox.domain = sbox;
          scbox.colors.clear();

          ind[0] = i;

          // Add edge shared ranks
          for (size_t d = 0; d < D; ++d) {
            idx_new[0] = idx[0];
            if (ind[d] == 0) {
              idx_new[d] -= 1;
              cval = idx_new[0];
              scbox.colors.emplace_back(cval);
            } else if (ind[d] == 2) {
              idx_new[d] += 1;
              cval = idx_new[0];
              scbox.colors.emplace_back(cval);
            }
          }

          // Add corner vertex shared ranks
          if (TD == 0) {
            idx_new[0] = idx[0];

            if (i == 0) {
              idx_new[0] -= 1;
              cval = idx_new[0];
              scbox.colors.emplace_back(cval);
            } else if (i == 2) {
              idx_new[0] += 1;
              cval = idx_new[0];
              scbox.colors.emplace_back(cval);
            }
          }
          colbox.shared[0].emplace_back(scbox);
        }
      }
    }
  } // create_exclusive_and_shared_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 2>::type create_exclusive_and_shared_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_color ecbox(D);
    box_core ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }
    ecbox.domain = ebox;
    ecbox.colors.emplace_back(rank); 
    colbox.exclusive[0] = ecbox;

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = pbox.lowerbnd[i];
      IM[i][1] = ebox.lowerbnd[i];
      IM[i][2] = ebox.upperbnd[i];
      IM[i][3] = pbox.upperbnd[i];
    }

    box_core sbox(D);
    box_color scbox(D);
    size_t ind[D], idx_new[D], cval;

    {
      for (size_t j = 0; j < 3; ++j)
        for (size_t i = 0; i < 3; ++i) {
          bool flag;
          if ((i == 1) && (j == 1))
            flag = false;
          else if ((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
            flag = true;
          else if (
              (i == 1) && (IM[0][i] == IM[0][i + 1]) &&
              (IM[1][j] != IM[1][j + 1]))
            flag = true;
          else if (
              (j == 1) && (IM[0][i] != IM[0][i + 1]) &&
              (IM[1][j] == IM[1][j + 1]))
            flag = true;
          else
            flag = false;

          if (flag) {
            sbox.lowerbnd[0] = IM[0][i] + il[i];
            sbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            sbox.lowerbnd[1] = IM[1][j] + il[j];
            sbox.upperbnd[1] = IM[1][j + 1] - iu[j];

            scbox.domain = sbox;
            scbox.colors.clear();

            ind[0] = i;
            ind[1] = j;

            // Add edge shared ranks
            for (size_t d = 0; d < D; ++d) {
              idx_new[0] = idx[0];
              idx_new[1] = idx[1];
              if (ind[d] == 0) {
                idx_new[d] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              } else if (ind[d] == 2) {
                idx_new[d] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
            }

            // Add corner vertex shared ranks
            if (TD == 0) {
              idx_new[0] = idx[0];
              idx_new[1] = idx[1];

              if ((i == 0) && (j == 0)) {
                idx_new[0] -= 1;
                idx_new[1] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              } else if ((i == 2) && (j == 0)) {
                idx_new[0] += 1;
                idx_new[1] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              } else if ((i == 0) && (j == 2)) {
                idx_new[0] -= 1;
                idx_new[1] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              } else if ((i == 2) && (j == 2)) {
                idx_new[0] += 1;
                idx_new[1] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
            }
            colbox.shared[0].emplace_back(scbox);
          }
        }
    }
  } // create_exclusive_and_shared_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 3>::type create_exclusive_and_shared_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_color ecbox(D);
    box_core ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }
    ecbox.domain = ebox;
    ecbox.colors.emplace_back(rank); 
    colbox.exclusive[0] = ecbox;

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = pbox.lowerbnd[i];
      IM[i][1] = ebox.lowerbnd[i];
      IM[i][2] = ebox.upperbnd[i];
      IM[i][3] = pbox.upperbnd[i];
    }

    box_core sbox(D);
    box_color scbox(D);
    size_t ind[D], idx_new[D], cval;

    {
      for (size_t k = 0; k < 3; ++k)
        for (size_t j = 0; j < 3; ++j)
          for (size_t i = 0; i < 3; ++i) {
            bool flag;
            if ((i == 1) && (j == 1) && (k == 1))
              flag = false;
            else if (
                (IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if (
                (i == 1) && (IM[0][i] == IM[0][i + 1]) &&
                (IM[1][j] != IM[1][j + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if (
                (j == 1) && (IM[1][j] == IM[1][j + 1]) &&
                (IM[0][i] != IM[0][i + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if (
                (k == 1) && (IM[2][k] == IM[2][k + 1]) &&
                (IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
              flag = true;
            else if (
                (i == 1) && (IM[0][i] == IM[0][i + 1]) && (j == 1) &&
                (IM[1][j] == IM[1][j + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if (
                (j == 1) && (IM[1][j] == IM[1][j + 1]) && (k == 1) &&
                (IM[2][k] == IM[2][k + 1]) && (IM[0][i] != IM[0][i + 1]))
              flag = true;
            else if (
                (k == 1) && (IM[2][k] == IM[2][k + 1]) && (i == 1) &&
                (IM[0][i] == IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
              flag = true;
            else
              flag = false;

            if (flag) {
              sbox.lowerbnd[0] = IM[0][i] + il[i];
              sbox.upperbnd[0] = IM[0][i + 1] - iu[i];
              sbox.lowerbnd[1] = IM[1][j] + il[j];
              sbox.upperbnd[1] = IM[1][j + 1] - iu[j];
              sbox.lowerbnd[2] = IM[2][k] + il[k];
              sbox.upperbnd[2] = IM[2][k + 1] - iu[k];

              scbox.domain = sbox;
              scbox.colors.clear();
              ind[0] = i;
              ind[1] = j;
              ind[2] = k;

              // Add face shared ranks
              for (size_t d = 0; d < D; ++d) {
                idx_new[0] = idx[0];
                idx_new[1] = idx[1];
                idx_new[2] = idx[2];
                if (ind[d] == 0) {
                  idx_new[d] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if (ind[d] == 2) {
                  idx_new[d] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
              }

              // Add edge shared ranks
              if (TD <= 1) {
                for (size_t d = 0; d < D; ++d) {
                  idx_new[0] = idx[0];
                  idx_new[1] = idx[1];
                  idx_new[2] = idx[2];

                  if ((ind[im[d][0]] == 0) && (ind[im[d][1]] == 0)) {
                    idx_new[im[d][0]] -= 1;
                    idx_new[im[d][1]] -= 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if ((ind[im[d][0]] == 2) && (ind[im[d][1]] == 0)) {
                    idx_new[im[d][0]] += 1;
                    idx_new[im[d][1]] -= 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if ((ind[im[d][0]] == 0) && (ind[im[d][1]] == 2)) {
                    idx_new[im[d][0]] -= 1;
                    idx_new[im[d][1]] += 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if ((ind[im[d][0]] == 2) && (ind[im[d][1]] == 2)) {
                    idx_new[im[d][0]] += 1;
                    idx_new[im[d][1]] += 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                }
              }

              // Add corner vertex shared ranks
              if (TD == 0) {
                idx_new[0] = idx[0];
                idx_new[1] = idx[1];
                idx_new[2] = idx[2];

                if ((i == 0) && (j == 0) && (k == 0)) {
                  idx_new[0] -= 1;
                  idx_new[1] -= 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 2) && (j == 0) && (k == 0)) {
                  idx_new[0] += 1;
                  idx_new[1] -= 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 0) && (j == 2) && (k == 0)) {
                  idx_new[0] -= 1;
                  idx_new[1] += 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 2) && (j == 2) && (k == 0)) {
                  idx_new[0] += 1;
                  idx_new[1] += 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 0) && (j == 0) && (k == 2)) {
                  idx_new[0] -= 1;
                  idx_new[1] -= 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 2) && (j == 0) && (k == 2)) {
                  idx_new[0] += 1;
                  idx_new[1] -= 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 0) && (j == 2) && (k == 2)) {
                  idx_new[0] -= 1;
                  idx_new[1] += 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                } else if ((i == 2) && (j == 2) && (k == 2)) {
                  idx_new[0] += 1;
                  idx_new[1] += 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
              }
              colbox.shared[0].emplace_back(scbox);
            }
          }
    }

  } // create_exclusive_and_shared_boxes
  
  template<size_t D_ = D>
  typename std::enable_if<D_ == 1>::type create_ghost_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    int TD = colbox.partition[0].thru_dim;

    std::cout<<"ncolors = "<<ncolors[0]<<std::endl;
    // Compute bounds for exclusive box
    box_core gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_core ghbox(D);
    box_color gcbox(D);
    size_t idx_new[D], cval;

    {
      int rankmap[3][2] = {{0, -1}, {1, 0}, {0, 1}};

      for (size_t i = 0; i < 3; ++i) {
        if (i == 1)
          continue;
        else if (IM[0][i] != IM[0][i + 1]) {
          ghbox.lowerbnd[0] = IM[0][i] + il[i];
          ghbox.upperbnd[0] = IM[0][i + 1] - iu[i];

          gcbox.colors.clear();
          size_t id = i;
          if (TD <= rankmap[id][0]) {
            idx_new[0] = idx[0] + rankmap[id][1];

            cval = idx_new[0];
            gcbox.domain = ghbox;
            gcbox.colors.emplace_back(cval);
            colbox.ghost[0].emplace_back(gcbox);
          }
        }
      }
    }
  } // create_ghost_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 2>::type create_ghost_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    int TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_core gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_core ghbox(D);
    box_color gcbox(D);
    size_t idx_new[D], cval;

    {
      int rankmap[9][3] = {{0, -1, -1}, {1, 0, -1}, {0, 1, -1},
                           {1, -1, 0},  {2, 0, 0},  {1, 1, 0},
                           {0, -1, 1},  {1, 0, 1},  {0, 1, 1}};

      for (size_t j = 0; j < 3; ++j)
        for (size_t i = 0; i < 3; ++i) {
          if ((i == 1) && (j == 1))
            continue;
          else if ((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            ghbox.lowerbnd[0] = IM[0][i] + il[i];
            ghbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            ghbox.lowerbnd[1] = IM[1][j] + il[j];
            ghbox.upperbnd[1] = IM[1][j + 1] - iu[j];

            gcbox.colors.clear();
            size_t id = i + 3 * j;
            if (TD <= rankmap[id][0]) {
              idx_new[0] = idx[0] + rankmap[id][1];
              idx_new[1] = idx[1] + rankmap[id][2];

              cval = idx_new[0] + ncolors[0] * idx_new[1];
              gcbox.domain = ghbox;
              gcbox.colors.emplace_back(cval);
              colbox.ghost[0].emplace_back(gcbox);
            }
          }
        }
    }
  } // create_ghost_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 3>::type create_ghost_boxes(
      box_coloring & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nghost_layers;
    int TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_core gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_core ghbox(D);
    box_color gcbox(D);
    size_t idx_new[D], cval;

    {
      int rankmap[27][4] = {
          {0, -1, -1, -1}, {1, 0, -1, -1}, {0, 1, -1, -1}, {1, -1, 0, -1},
          {2, 0, 0, -1},   {1, 1, 0, -1},  {0, -1, 1, -1}, {1, 0, 1, -1},
          {0, 1, 1, -1},   {1, -1, -1, 0}, {2, 0, -1, 0},  {1, 1, -1, 0},
          {2, -1, 0, 0},   {3, 0, 0, 0},   {2, 1, 0, 0},   {1, -1, 1, 0},
          {2, 0, 1, 0},    {1, 1, 1, 0},   {0, -1, -1, 1}, {1, 0, -1, 1},
          {0, 1, -1, 1},   {1, -1, 0, 1},  {2, 0, 0, 1},   {1, 1, 0, 1},
          {0, -1, 1, 1},   {1, 0, 1, 1},   {0, 1, 1, 1}};

      for (size_t k = 0; k < 3; ++k)
        for (size_t j = 0; j < 3; ++j)
          for (size_t i = 0; i < 3; ++i) {
            if ((i == 1) && (j == 1) && (k == 1))
              continue;
            else if (
                (IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                (IM[2][k] != IM[2][k + 1])) {
              ghbox.lowerbnd[0] = IM[0][i] + il[i];
              ghbox.upperbnd[0] = IM[0][i + 1] - iu[i];
              ghbox.lowerbnd[1] = IM[1][j] + il[j];
              ghbox.upperbnd[1] = IM[1][j + 1] - iu[j];
              ghbox.lowerbnd[2] = IM[2][k] + il[k];
              ghbox.upperbnd[2] = IM[2][k + 1] - iu[k];

              gcbox.colors.clear();
              size_t id = i + 3 * j + 9 * k;
              if (TD <= rankmap[id][0]) {
                idx_new[0] = idx[0] + rankmap[id][1];
                idx_new[1] = idx[1] + rankmap[id][2];
                idx_new[2] = idx[2] + rankmap[id][3];

                cval = idx_new[0] + ncolors[0] * idx_new[1] +
                       ncolors[0] * ncolors[1] * idx_new[2];
                gcbox.domain = ghbox;
                gcbox.colors.emplace_back(cval);
                colbox.ghost[0].emplace_back(gcbox);
              }
            }
          }
    }
  } // create_ghost_boxes

  void create_domain_halo_boxes(box_coloring & colbox) {
    box_core pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].ndomain_layers;

    // Compute bounds for domain with halo
    box_core dbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (colbox.partition[0].onbnd[2 * i])
        dbox.lowerbnd[i] -= hl;
      if (colbox.partition[0].onbnd[2 * i + 1])
        dbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = dbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = dbox.upperbnd[i];
    }

    box_color dhbox(D);

    if (D == 1) {
        for (size_t i = 0; i < 3; ++i) {
          if ((i == 1))
            continue;
          else if ((IM[0][i] != IM[0][i + 1])) {
            dhbox.domain.lowerbnd[0] = IM[0][i] + il[i];
            dhbox.domain.upperbnd[0] = IM[0][i + 1] - iu[i];
            colbox.domain_halo[0].emplace_back(dhbox);
          }
        }
   } else if (D == 2) {
      for (size_t j = 0; j < 3; ++j)
        for (size_t i = 0; i < 3; ++i) {
          if ((i == 1) && (j == 1))
            continue;
          else if ((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            dhbox.domain.lowerbnd[0] = IM[0][i] + il[i];
            dhbox.domain.upperbnd[0] = IM[0][i + 1] - iu[i];
            dhbox.domain.lowerbnd[1] = IM[1][j] + il[j];
            dhbox.domain.upperbnd[1] = IM[1][j + 1] - iu[j];
            colbox.domain_halo[0].emplace_back(dhbox);
          }
        }
    } else if (D == 3) {
      for (size_t k = 0; k < 3; ++k)
        for (size_t j = 0; j < 3; ++j)
          for (size_t i = 0; i < 3; ++i) {
            if ((i == 1) && (j == 1) && (k == 1))
              continue;
            else if (
                (IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                (IM[2][k] != IM[2][k + 1])) {
              dhbox.domain.lowerbnd[0] = IM[0][i] + il[i];
              dhbox.domain.upperbnd[0] = IM[0][i + 1] - iu[i];
              dhbox.domain.lowerbnd[1] = IM[1][j] + il[j];
              dhbox.domain.upperbnd[1] = IM[1][j + 1] - iu[j];
              dhbox.domain.lowerbnd[2] = IM[2][k] + il[k];
              dhbox.domain.upperbnd[2] = IM[2][k + 1] - iu[k];
              colbox.domain_halo[0].emplace_back(dhbox);
            }
          }
    }
  } // create_domain_halo_boxes

  void compute_overlaying_bounding_box(box_coloring& colbox)
  {
    for (size_t n = 0; n < colbox.num_boxes; n++){
   
      box_core obb(D); 
      std::vector<size_t> lbnds[D]; 
      std::vector<size_t> ubnds[D]; 

      for (size_t d = 0; d < D; d++){
       for (size_t i = 0; i < colbox.ghost[n].size(); i++)
       {
         lbnds[d].push_back(colbox.ghost[n][i].domain.lowerbnd[d]);
         ubnds[d].push_back(colbox.ghost[n][i].domain.upperbnd[d]);
       }
      }

      for (size_t d = 0; d < D; d++){
       for (size_t i = 0; i < colbox.domain_halo[n].size(); i++)
       {
         lbnds[d].push_back(colbox.domain_halo[n][i].domain.lowerbnd[d]);
         ubnds[d].push_back(colbox.domain_halo[n][i].domain.upperbnd[d]);
       }
      }
      
      for (size_t d = 0; d < D; d++){
       std::sort(lbnds[d].begin(), lbnds[d].end());
       std::sort(ubnds[d].begin(), ubnds[d].end()); 
      }

       for (size_t d = 0; d < D; d++){
        obb.lowerbnd[d] = *lbnds[d].begin();
        obb.upperbnd[d] = *(ubnds[d].end()-1);
       }

      colbox.overlay[n] = obb; 
   }

  } // compute_overlaying_bounding_box

  void get_indices(size_t cobnds[D], int rank, size_t id[D]) {
    size_t rem, factor, value;

    rem = rank;
    for (size_t i = 0; i < D; ++i) {
      factor = 1;
      for (size_t j = 0; j < D - i - 1; ++j)
        factor *= cobnds[j];

      value = rem / factor;
      id[D - i - 1] = value;
      rem -= value * factor;
    }
  } // get_indices

}; // class simple_box_colorer

} // namespace structured_impl
} // namespace topo
} // namespace flecsi
#endif // simple_box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
