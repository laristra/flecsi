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
#include <cassert>
#include <cmath>

#include <flecsi/coloring/box_colorer.h>
#include <flecsi/coloring/coloring_types.h>

#include <type_traits>

namespace flecsi {
namespace coloring {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//
template<size_t D>
struct simple_box_colorer_t : public box_colorer_t<D> {
  //! Default constructor
  simple_box_colorer_t() {}

  //! Copy constructor (disabled)
  simple_box_colorer_t(const simple_box_colorer_t &) = delete;

  //! Assignment operator (disabled)
  simple_box_colorer_t & operator=(const simple_box_colorer_t &) = delete;

  //! Destructor
  ~simple_box_colorer_t() {}

  // Simple partitioning algorithm for structured meshes. Partitions the highest
  // dimensional entities in the  mesh into as many blocks as number of input 
  // ranks.
  box_coloring_t color(
      size_t grid_size[D],
      size_t nhalo,
      size_t nhalo_domain,
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
      std::cerr << "Through dimension (" << thru_dim << ") must less than"
        << " total number of dimensions (" << D  << ")" << std::endl;
    }

    // Obtain indices of the current rank
    size_t idx[D];
    get_indices(ncolors, rank, idx);

    // Step 1: Create bounding boxes for domain with and without halo
    box_t domain(D);
    for (size_t i = 0; i < D; ++i) {
      domain.lowerbnd[i] = nhalo_domain;
      domain.upperbnd[i] = grid_size[i] + nhalo_domain - 1;
    }

    // Compute the strides of the global mesh
    std::vector<size_t> strides(D); 
    for (size_t i = 0; i < D; ++i) {
      strides[i] = grid_size[i] + 2*nhalo_domain;
    }

    // Step 2: Compute the primary box bounds for the current rank
    auto pbox = create_primary_box(domain, ncolors, idx);

    // Step 2: Create colored box type and set its primary box
    // info to the one created in step 1.
    box_coloring_t colbox_cells;
    colbox_cells.primary = true;
    colbox_cells.primary_dim = D;
    colbox_cells.num_boxes = 1; 
    colbox_cells.resize(); 

    colbox_cells.partition[0].box = pbox;
    colbox_cells.partition[0].nhalo = nhalo;
    colbox_cells.partition[0].nhalo_domain = nhalo_domain;
    colbox_cells.partition[0].thru_dim = thru_dim;
    
    colbox_cells.strides.push_back(strides);

    //Set the onbnd vector to false 
    for(size_t i = 0; i < 2*D; ++i)
     colbox_cells.partition[0].onbnd.push_back(false);

    for (size_t i = 0; i < D; ++i) {
      if (pbox.lowerbnd[i] == domain.lowerbnd[i])
        colbox_cells.partition[0].onbnd[2 * i] = true;
      if (pbox.upperbnd[i] == domain.upperbnd[i])
        colbox_cells.partition[0].onbnd[2 * i+1] = true;
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

  // Coloring algorithm for coloring the intermediate entities
  // from an input partition of the cells in the mesh 
  auto color_dependent_entities(box_coloring_t &colored_cells)
  {
     std::vector<box_coloring_t> colored_depents; 
     
     //Reorder ghost and shared boxes for colored cells
     reorder_boxes(colored_cells); 

     //Now color the intermediate entities
     color_dependent_entities_(colored_cells, colored_depents); 
  
     return colored_depents; 
  } //color_intermediate_entities


  // Compute the aggregate information from a colored box. 
  box_aggregate_info_t create_aggregate_info(box_coloring_t &cbox)
  {
    box_aggregate_info_t colinfo; 
 
    //#exclusive entities
    colinfo.exclusive = cbox.exclusive[0].box.size();

    //#shared entities
    colinfo.shared = 0; 
    std::set<size_t> shared_ranks; 
    for (size_t i = 0; i < cbox.shared[0].size(); i++)
    {
      colinfo.shared += cbox.shared[0][i].box.size();
      for (size_t j = 0; j < cbox.shared[0][i].colors.size(); j++)
         shared_ranks.insert(cbox.shared[0][i].colors[j]);
    }
    colinfo.shared_users = shared_ranks; 

    //#ghost entities
    colinfo.ghost = 0; 
    std::set<size_t> ghost_ranks; 
    for (size_t i = 0; i < cbox.ghost[0].size(); i++)
    {
      colinfo.ghost += cbox.ghost[0][i].box.size();
      for (size_t j = 0; j < cbox.ghost[0][i].colors.size(); j++)
         ghost_ranks.insert(cbox.ghost[0][i].colors[j]);
    }
    colinfo.ghost_owners = ghost_ranks;

   return colinfo;  
  }//create_aggregate_color_info 

  


private:
  auto create_primary_box(box_t & domain, size_t ncolors[D], size_t idx[D]) {
    box_t pbox(D);
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
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }

    colbox.exclusive[0].box = ebox;
    colbox.exclusive[0].colors.emplace_back(rank);

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

    box_t sbox(D);
    box_color_t scbox;
    size_t ind[D], idx_new[D], cval;

    {
      for (size_t i = 0; i < 3; ++i) {
        bool flag;
        if ((i == 1))
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

          scbox.box = sbox;
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

            if ((i == 0)) {
              idx_new[0] -= 1;
              cval = idx_new[0];
              scbox.colors.emplace_back(cval);
            } else if ((i == 2)) {
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
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }

    colbox.exclusive[0].box = ebox;
    colbox.exclusive[0].colors.emplace_back(rank);

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

    box_t sbox(D);
    box_color_t scbox;
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

            scbox.box = sbox;
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
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D],
      size_t rank) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t ebox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }

    colbox.exclusive[0].box = ebox;
    colbox.exclusive[0].colors.emplace_back(rank);

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

    box_t sbox(D);
    box_color_t scbox;
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

              scbox.box = sbox;
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
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_t ghbox(D);
    box_color_t gcbox;
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
            gcbox.box = ghbox;
            gcbox.colors.emplace_back(cval);
            colbox.ghost[0].emplace_back(gcbox);
          }
        }
      }
    }
  } // create_ghost_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 2>::type create_ghost_boxes(
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_t ghbox(D);
    box_color_t gcbox;
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
              gcbox.box = ghbox;
              gcbox.colors.emplace_back(cval);
              colbox.ghost[0].emplace_back(gcbox);
            }
          }
        }
    }
  } // create_ghost_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 3>::type create_ghost_boxes(
      box_coloring_t & colbox,
      size_t ncolors[D],
      size_t idx[D]) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo;
    size_t TD = colbox.partition[0].thru_dim;

    // Compute bounds for exclusive box
    box_t gbox = pbox;

    for (size_t i = 0; i < D; ++i) {
      if (!colbox.partition[0].onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if (!colbox.partition[0].onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for (size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_t ghbox(D);
    box_color_t gcbox;
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
                gcbox.box = ghbox;
                gcbox.colors.emplace_back(cval);
                colbox.ghost[0].emplace_back(gcbox);
              }
            }
          }
    }
  } // create_ghost_boxes

  void create_domain_halo_boxes(box_coloring_t & colbox) {
    box_t pbox = colbox.partition[0].box;
    size_t hl = colbox.partition[0].nhalo_domain;

    // Compute bounds for domain with halo
    box_t dbox = pbox;

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

    box_t dhbox(D);

    if (D == 2) {
      for (size_t j = 0; j < 3; ++j)
        for (size_t i = 0; i < 3; ++i) {
          if ((i == 1) && (j == 1))
            continue;
          else if ((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            dhbox.lowerbnd[0] = IM[0][i] + il[i];
            dhbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            dhbox.lowerbnd[1] = IM[1][j] + il[j];
            dhbox.upperbnd[1] = IM[1][j + 1] - iu[j];
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
              dhbox.lowerbnd[0] = IM[0][i] + il[i];
              dhbox.upperbnd[0] = IM[0][i + 1] - iu[i];
              dhbox.lowerbnd[1] = IM[1][j] + il[j];
              dhbox.upperbnd[1] = IM[1][j + 1] - iu[j];
              dhbox.lowerbnd[2] = IM[2][k] + il[k];
              dhbox.upperbnd[2] = IM[2][k + 1] - iu[k];
              colbox.domain_halo[0].emplace_back(dhbox);
            }
          }
    }
  } // create_domain_halo_boxes

  void compute_overlaying_bounding_box(box_coloring_t& colbox)
  {
    for (size_t n = 0; n < colbox.num_boxes; n++){
   
      box_t obb(D); 
      std::vector<size_t> lbnds[D]; 
      std::vector<size_t> ubnds[D]; 

      for (size_t d = 0; d < D; d++){
       for (size_t i = 0; i < colbox.ghost[n].size(); i++)
       {
         lbnds[d].push_back(colbox.ghost[n][i].box.lowerbnd[d]);
         ubnds[d].push_back(colbox.ghost[n][i].box.upperbnd[d]);
       }
      }

      for (size_t d = 0; d < D; d++){
       for (size_t i = 0; i < colbox.domain_halo[n].size(); i++)
       {
         lbnds[d].push_back(colbox.domain_halo[n][i].lowerbnd[d]);
         ubnds[d].push_back(colbox.domain_halo[n][i].upperbnd[d]);
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

 /*******************************************************
 *      Methods for coloring intermediate entities
 ********************************************************/
 
  void reorder_boxes(box_coloring_t& colcells)
  { 
    // Put the shared boxes in a predefined order 
    size_t count = pow(3,D)-1; 
    std::vector<box_color_t> reordered_shared(count);
    std::cout<<"count = "<<count<<std::endl;
    std::cout<<"reordered_shared.size = "<<reordered_shared.size()<<std::endl;
    for (int i = 0; i < colcells.shared[0].size(); i++)
    {
      size_t id = find_box_neighbor_id(colcells.exclusive[0].box, 
                  colcells.shared[0][i].box); 
      std::cout<<"id = "<<id<<std::endl;
      reordered_shared[id] = colcells.shared[0][i];
    }
    colcells.shared[0] = reordered_shared; 


    // Put the ghost boxes in a predefined order 
    std::vector<box_color_t> reordered_ghost; 
    reordered_ghost.resize(count);  
    for (int i = 0; i < colcells.ghost[0].size(); i++)
    {
      size_t id = find_box_neighbor_id(colcells.partition[0].box, 
                  colcells.ghost[0][i].box); 
      std::cout<<"id = "<<id<<std::endl;
      reordered_ghost[id] = colcells.ghost[0][i];
    }
    colcells.ghost[0] = reordered_ghost; 
   
  } //reorder_boxes

  size_t find_box_neighbor_id(box_t& center, box_t& neighbor)
  {
    size_t id; 
    if (D==2) {

      if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
          (neighbor.upperbnd[1]+1 == center.lowerbnd[1]))
         id = 0;
      else if ((neighbor.upperbnd[1]+1 == center.lowerbnd[1]) &&
               (neighbor.lowerbnd[0] == center.lowerbnd[0]) &&
               (neighbor.upperbnd[0] == center.upperbnd[0]))  
         id = 1; 
      else if ((neighbor.upperbnd[1]+1 == center.lowerbnd[1]) &&
               (neighbor.lowerbnd[0]-1 == center.upperbnd[0]))
         id = 2; 
      else if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
               (neighbor.lowerbnd[1] == center.lowerbnd[1]) &&
               (neighbor.upperbnd[1] == center.upperbnd[1]))  
         id = 3; 
      else if ((neighbor.lowerbnd[0]-1 == center.upperbnd[0]) &&
               (neighbor.lowerbnd[1] == center.lowerbnd[1]) &&
               (neighbor.upperbnd[1] == center.upperbnd[1]))  
         id = 4; 
      else if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
         id = 5; 
      else if ((neighbor.lowerbnd[1]-1 == center.upperbnd[1]) &&
               (neighbor.lowerbnd[0] == center.lowerbnd[0]) &&
               (neighbor.upperbnd[0] == center.upperbnd[0]))  
         id = 6; 
      else if ((neighbor.lowerbnd[0]-1 == center.upperbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
         id = 7; 
     }

   return id; 

  } //find_box_neighbor_id 

  void color_dependent_entities_(
       box_coloring_t& col_cells, 
       std::vector<box_coloring_t>& col_depents)
  {
     if (D==2){  
       //Resize input vector
        col_depents.resize(2); 
    
       //Define info needed for setting the correct bounds
       //for the overlays 
       std::vector<int> bnds_info[2] = {{1,1,1},{2,1,0,0,1}};
 
       for (size_t d = 0; d < D; d++)
       {
         box_coloring_t col_ents; 

        //Set primary info and resize vectors  
        col_ents.primary = false; 
        col_ents.primary_dim = D; 
        col_ents.num_boxes = bnds_info[d][0];
        col_ents.resize(D); 

        size_t num_boxes = col_ents.num_boxes; 
        //Compute the overlay bounds 
        for (size_t k = 0; k < num_boxes; k++)
        {
          box_t overlay = col_cells.overlay[0]; 
          for (size_t j = 0; j < D; j++)
          overlay.upperbnd[j] += bnds_info[d][num_boxes*k+j+1]; 
          col_ents.overlay[k] = overlay;  
        }        
   
        //Compute the strides 
        for (size_t k = 0; k < num_boxes; k++)
        {
          std::vector<size_t> strides = col_cells.strides[0]; 
          for (size_t j = 0; j < D; j++)
          strides[j] += bnds_info[d][num_boxes*k+j+1]; 
          col_ents.strides[k] = strides;  
        }        

         //Compute the ghost boxes for entities of dimension d
         for (size_t i = 0; i < 3^D-1; i++)
         {
          if (!col_cells.ghost[0][i].box.isempty())
          { 
              size_t current_rank = col_cells.ghost[0][i].colors[0]; 

              //Compute the bounding boxes for the entities of dimension d from the
              //ghost box of cell.
             box_color_t ghost_boxes[num_boxes]; 

             for (size_t k = 0; k <num_boxes; k++)
             {
               for (size_t j = 0; j < D; j++)
              {
                ghost_boxes[k].box.lowerbnd[j] = col_cells.ghost[0][i].box.lowerbnd[j]; 
                ghost_boxes[k].box.upperbnd[j] = col_cells.ghost[0][i].box.upperbnd[j]
                                             + bnds_info[d][num_boxes*k+j+1]; 
              }
          
              //Loop over each direction
              for (size_t j = 0; j < D; j++)
              {
                //Check the ghost neighbors along each side 
                int opp_box_id_left = ghost2ghost_2d[i][2*j]; 
                if ((opp_box_id_left != -1) && (!col_cells.ghost[0][opp_box_id_left].box.isempty()))
                {
                  size_t opp_rank = col_cells.ghost[0][opp_box_id_left].colors[0];
                  if (opp_rank < current_rank)
                    ghost_boxes[k].box.lowerbnd[j] += 1; 
                } 

                int opp_box_id_right = ghost2ghost_2d[i][2*j+1];
                if ((opp_box_id_right != -1) && (!col_cells.ghost[0][opp_box_id_right].box.isempty()))
                {
                  size_t opp_rank = col_cells.ghost[0][opp_box_id_left].colors[0];
                  if (opp_rank < current_rank)
                    ghost_boxes[k].box.upperbnd[j] -= 1; 
                }
 
                //Check the shared neighbors along each side
                int onpart_left = ghost2partition_2d[i][2*j]; 
                if (onpart_left)
                {
                    size_t opp_rank = col_cells.exclusive[0].colors[0];
                    if (opp_rank < current_rank)
                      ghost_boxes[k].box.lowerbnd[j] += 1; 
                }
 
                int onpart_right = ghost2partition_2d[i][2*j+1]; 
                if (onpart_right)
                {
                    size_t opp_rank = col_cells.exclusive[0].colors[0];
                    if (opp_rank < current_rank)
                      ghost_boxes[k].box.lowerbnd[j] -= 1; 
                } 
              }
   
              //Add ghost rank to the ghost boxes 
              ghost_boxes[k].colors.push_back(current_rank); 
              col_ents.ghost[k][i] = ghost_boxes[k];            
            } // end loop over each sub-box 
          }  
         } // end loop over ghost boxes 
 
       
         //Compute the shared boxes for entities of dimension d
         for (size_t i = 0; i < 3^D-1; i++)
         {
          if (!col_cells.shared[0][i].box.isempty())
          { 
             size_t current_rank = col_cells.exclusive[0].colors[0]; 
             std::vector<size_t> shared_ranks = col_cells.shared[0][i].colors; 

             //Compute the bounding boxes for the entities of dimension d from the
             //ghost box of cell.
             box_color_t shared_boxes[num_boxes]; 

             for (size_t k = 0; k <num_boxes; k++)
             {
               for (size_t j = 0; j < D; j++)
              {
                shared_boxes[k].box.lowerbnd[j] = col_cells.shared[0][i].box.lowerbnd[j]; 
                shared_boxes[k].box.upperbnd[j] = col_cells.shared[0][i].box.upperbnd[j]
                                             + bnds_info[d][num_boxes*k+j+1]; 
              }
          
              //Loop over each direction
              for (size_t j = 0; j < D; j++)
              {
                int opp_box_id_left = shared2ghost_2d[i][2*j]; 
                if (opp_box_id_left != -1)
                {
                  size_t opp_rank = col_cells.ghost[0][opp_box_id_left].colors[0];
                  if (opp_rank < current_rank)
                    shared_boxes[k].box.lowerbnd[j] += 1; 
                } 

                int opp_box_id_right = shared2ghost_2d[i][2*j+1];
                if (opp_box_id_right != -1)
                {
                  size_t opp_rank = col_cells.ghost[0][opp_box_id_left].colors[0];
                  if (opp_rank < current_rank)
                    shared_boxes[k].box.upperbnd[j] -= 1; 
                } 
              }

              //Add shared ranks to the shared boxes
              shared_boxes[k].colors = shared_ranks; 
              col_ents.shared[k][i] = shared_boxes[k];            
            } // end loop over each sub-box 
          } 
         } // end loop over shared boxes

       //Compute the exclusive box for entities of dimension d  
       size_t current_rank = col_cells.exclusive[0].colors[0]; 
       box_color_t exclusive_boxes[num_boxes]; 

       for (size_t k = 0; k <num_boxes; k++)
       {
          for (size_t j = 0; j < D; j++)
          {
            exclusive_boxes[k].box.lowerbnd[j] = col_cells.exclusive[0].box.lowerbnd[j];
            exclusive_boxes[k].box.upperbnd[j] = col_cells.exclusive[0].box.upperbnd[j]
                                                 + bnds_info[d][num_boxes*k+j+1];

            if (!col_cells.partition[0].onbnd[2*j] )
              exclusive_boxes[k].box.lowerbnd[j] += col_cells.partition[0].nhalo; 
            if (!col_cells.partition[0].onbnd[2*j+1] )
              exclusive_boxes[k].box.upperbnd[j]  -= col_cells.partition[0].nhalo; 
           }

         exclusive_boxes[k].colors.push_back(current_rank); 
         col_ents.exclusive[k] = exclusive_boxes[k]; 
        } // end loop for exclusive boxes

       //Compute the domain halo boxes for entities of dimension d  
       for (size_t i = 0 ; i < col_cells.domain_halo[0].size(); i++){
         box_t halo_boxes[num_boxes]; 

         for (size_t k = 0; k <num_boxes; k++)
         {
            for (size_t j = 0; j < D; j++)
            {
              halo_boxes[k].lowerbnd[j] = col_cells.domain_halo[0][i].lowerbnd[j]; 
              halo_boxes[k].upperbnd[j] = col_cells.domain_halo[0][i].upperbnd[j]
                                            + bnds_info[d][num_boxes*k+j+1]; 
            }

            col_ents.domain_halo[k].push_back(halo_boxes[k]); 
          }
       } // end loop for domain halo boxes

       col_depents[d] = col_ents;       
 
       } // end loop over intermediate entities

  } //dimension check
  } //color_intermediate_entities_

   int shared2ghost_2d[8][4] = {{-1,1,-1,3},{0,2,-1,-1},{1,-1,-1,4},{-1,-1,0,5},
                                {-1,-1,2,7},{-1,6,3,-1},{5,7,-1,-1},{6,-1,4,-1}}; 

   int ghost2ghost_2d[8][4] = {{3,-1,1,-1},{-1,-1,1,-1},{-1,4,1,-1},{3,-1,-1,-1},
                               {-1,4,-1,-1},{3,-1,-1,6},{-1,-1,-1,6},{-1,4,-1,6}}; 

   int ghost2partition_2d[8][4] = {{0,0,0,0},{0,0,0,1},{0,0,0,0},{0,1,0,0},
                                   {1,0,0,0},{0,0,0,0},{0,0,1,0},{0,0,0,0}};
 

}; // class simple_box_colorer_t

} // namespace coloring
} // namespace flecsi
#endif // simple_box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
