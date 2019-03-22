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
      std::cerr << "Through dimension (" << thru_dim << ") must be less than"
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

     //Tag box bounds
     compute_bounding_box_tags(colbox_cells); 

     //Now color the intermediate entities
     color_dependent_entities(colored_cells, colored_depents); 
  
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

    colbox.exclusive[0].domain.box = ebox;
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

          scbox.domain.box = sbox;
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

    colbox.exclusive[0].domain.box = ebox;
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

            scbox.domain.box = sbox;
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

    colbox.exclusive[0].domain.box = ebox;
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

              scbox.domain.box = sbox;
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
            gcbox.domain.box = ghbox;
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
              gcbox.domain.box = ghbox;
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
                gcbox.domain.box = ghbox;
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

    box_tag_t dhbox(D);

    if (D == 2) {
      for (size_t j = 0; j < 3; ++j)
        for (size_t i = 0; i < 3; ++i) {
          if ((i == 1) && (j == 1))
            continue;
          else if ((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            dhbox.box.lowerbnd[0] = IM[0][i] + il[i];
            dhbox.box.upperbnd[0] = IM[0][i + 1] - iu[i];
            dhbox.box.lowerbnd[1] = IM[1][j] + il[j];
            dhbox.box.upperbnd[1] = IM[1][j + 1] - iu[j];
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
              dhbox.box.lowerbnd[0] = IM[0][i] + il[i];
              dhbox.box.upperbnd[0] = IM[0][i + 1] - iu[i];
              dhbox.box.lowerbnd[1] = IM[1][j] + il[j];
              dhbox.box.upperbnd[1] = IM[1][j + 1] - iu[j];
              dhbox.box.lowerbnd[2] = IM[2][k] + il[k];
              dhbox.box.upperbnd[2] = IM[2][k + 1] - iu[k];
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
         lbnds[d].push_back(colbox.ghost[n][i].domain.box.lowerbnd[d]);
         ubnds[d].push_back(colbox.ghost[n][i].domain.box.upperbnd[d]);
       }
      }

      for (size_t d = 0; d < D; d++){
       for (size_t i = 0; i < colbox.domain_halo[n].size(); i++)
       {
         lbnds[d].push_back(colbox.domain_halo[n][i].box.lowerbnd[d]);
         ubnds[d].push_back(colbox.domain_halo[n][i].box.upperbnd[d]);
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
    size_t count = pow(3,D); 
    std::vector<box_color_t> reordered_shared(count);
    std::cout<<"count = "<<count<<std::endl;
    std::cout<<"reordered_shared.size = "<<reordered_shared.size()<<std::endl;
    for (int i = 0; i < colcells.shared[0].size(); i++)
    {
      size_t id = find_box_neighbor_id(colcells.exclusive[0].domain.box, 
                  colcells.shared[0][i].domain.box); 
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
                  colcells.ghost[0][i].domain.box); 
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
         id = 5; 
      else if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
         id = 6; 
      else if ((neighbor.lowerbnd[1]-1 == center.upperbnd[1]) &&
               (neighbor.lowerbnd[0] == center.lowerbnd[0]) &&
               (neighbor.upperbnd[0] == center.upperbnd[0]))  
         id = 7; 
      else if ((neighbor.lowerbnd[0]-1 == center.upperbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
         id = 8; 
     }

   return id; 

  } //find_box_neighbor_id 

/*  auto find_box_boundary_ids(box_t& center, box_t& neighbor)
  {
    std::vector<int> bids; 
    if (D==2) {
      if ((neighbor.lowerbnd[0]-1 == center.upperbnd[0]) &&
               (neighbor.lowerbnd[1] == center.lowerbnd[1]) &&
               (neighbor.upperbnd[1] == center.upperbnd[1]))  
          bids.push_back(0); 

      if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
               (neighbor.lowerbnd[1] == center.lowerbnd[1]) &&
               (neighbor.upperbnd[1] == center.upperbnd[1]))  
          bids.push_back(1); 

      if ((neighbor.lowerbnd[1]-1 == center.upperbnd[1]) &&
               (neighbor.lowerbnd[0] == center.lowerbnd[0]) &&
               (neighbor.upperbnd[0] == center.upperbnd[0]))  
          bids.push_back(2);  

      if ((neighbor.upperbnd[1]+1 == center.lowerbnd[1]) &&
               (neighbor.lowerbnd[0] == center.lowerbnd[0]) &&
               (neighbor.upperbnd[0] == center.upperbnd[0]))  
         bids.push_back(3);  


      if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
          (neighbor.upperbnd[1]+1 == center.lowerbnd[1]))
         bids.push_back(7);
      
      if ((neighbor.upperbnd[1]+1 == center.lowerbnd[1]) &&
               (neighbor.lowerbnd[0]-1 == center.upperbnd[0]))
          bids.push_back(6); 
     
      
      if ((neighbor.upperbnd[0]+1 == center.lowerbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
          bids.push_back(3); 
      
      if ((neighbor.lowerbnd[0]-1 == center.upperbnd[0]) &&
               (neighbor.lowerbnd[1]-1 == center.upperbnd[1]))
          bids.push_back(4);
     }

   return id; 

  } //find_box_neighbor_id 
  */

  void compute_bounding_box_tags(box_coloring_t& col_cells)
  {
    
    size_t num_bids[2] = {8,26}; 
    size_t owner_rank = col_cells.exclusive[0].colors[0]; 
   
    //Compute the boundary tags for the ghost boxes 
    size_t count = pow(3,D); 
    for (size_t i = 0; i < count; i++)
    {
      if (!col_cells.ghost[0][i].domain.box.isempty())
      { 
        size_t current_rank = col_cells.ghost[0][i].colors[0]; 

        //Loop over all the boundary types 
        for (size_t bid = 0; bid <num_bids[D-2]; bid++)
        {
          std::set<int> ngbranks; 
          if (ghost2partition(i,bid))
            ngbranks.insert(owner_rank);

          std::vector<int> gids = ghost2ghost(i, bid); 
          if (gids.size() > 0){                
            for (size_t j = 0; j<gids.size(); j++)
            {
              if (!col_cells.ghost[0][gids[j]].domain.box.isempty())
               ngbranks.insert(col_cells.ghost[0][gids[j]].colors[0]);
            }
          }

          if (*ngbranks.begin() != current_rank)
              col_cells.ghost[0][i].domain.tag[bid] = false; 
        }
      }
    }

   //Compute the boundary tags for the shared boxes 
    size_t count = pow(3,D); 
    for (size_t i = 0; i < count; i++)
    {
      if (!col_cells.shared[0][i].domain.box.isempty())
      { 
        //Loop over all the boundary types 
        for (size_t bid = 0; bid <num_bids[D-2]; bid++)
        {
          std::vector<int> gids = shared2ghost(i, bid); 
          if (gids.size() > 0){
            std::set<int> ngbranks; 
            for (size_t j = 0; j<gids.size(); j++)
            {
              if (!col_cells.ghost[0][gids[j]].domain.box.isempty())
               ngbranks.insert(col_cells.ghost[0][gids[j]].colors[0]);
            }
          }
          if (*ngbranks.begin() != owner_rank)
              col_cells.shared[0][i].domain.tag[bid] = false; 
        }
      }
    }
 
    //Compute the boundary tags for the exclusive box
    //Loop over all the boundary types 
    for (size_t bid = 0; bid <num_bids[D-2]; bid++)
    {
      std::vector<int> gids = exclusive2shared(bid);  
      for (size_t j = 0; j<gids.size(); j++)
      {
        if (!col_cells.shared[0][gids[j]].domain.box.isempty())
          col_cells.exclusive[0].domain.tag[bid] = false; 
      }     
    }

    //Compute the boundary tags for the domain halo boxes 
   // box_t pbox = col_cells.partition[0].box; 
   // for (size_t i = 0; i < col_cells.domain_halo[0].size(); i++)
   //   compute_bounding_box_tags(pbox, col_cells.domain_halo[0][i]);
  }//compute_bounding_box_tags

  void color_dependent_entities_(
       box_coloring_t& col_cells, 
       std::vector<box_coloring_t>& col_depents)
  {
     if (D==2){  
       //Resize input vector
        col_depents.resize(2); 
    
       //Define info needed for setting the correct bounds
       //for the overlays 
       std::vector<int> bnds_info[2] = {{2,1,0,0,1},{1,1,1}};
       int bid_bnd[2] = {4,8};
 
       //Loop over intermediate dimensions
       for (size_t d = D-1; d >= 0 D; d--)
       {
        size_t owner_rank = col_cells.exclusive[0].colors[0]; 
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
        size_t count = pow(3,D); 
        for (size_t i = 0; i < count; i++)
        {
          if (!col_cells.ghost[0][i].domain.box.isempty())
          { 
            box_color_t ghosts[num_boxes]; 
            for (size_t k = 0; k <num_boxes; k++)
            {
              //Compute the bounding boxes for the entities of dimension d from the
             //ghost cell box.
              ghosts[k].domain.box.resize(D); 

              for (size_t j = 0; j < D; j++)
              {
                ghosts[k].domain.box.lowerbnd[j] = col_cells.ghost[0][i].domain.box.lowerbnd[j]; 
                ghosts[k].domain.box.upperbnd[j] = col_cells.ghost[0][i].domain.box.upperbnd[j]
                                           + bnds_info[d][num_boxes*k+j+1]; 
               }
        
              //Obtain the tags from the ghost cell box to tag the boundary entities
              //this ghost sub-box covers. 
              std::vector<int> cbids = depent2cellbids(d,k);
              for (size_t b = 0; b < cbids.size(); b++)
                ghosts[k].domain.tag[cbids[b]] = col_cells.ghost[0][i].domain.tag[cbids[b]];

              //Add rank of ghost rank to the intermediate ghost domain
              ghosts[k].colors = col_cells.ghost[0][i].colors[0];  
              col_ents.ghost[k][i] = ghost_boxes[k]; 
            }
          }// end loop over each sub-box 
        } // end loop over ghost boxes 

       //Compute the shared boxes for entities of dimension d
        size_t count = pow(3,D); 
        for (size_t i = 0; i < count; i++)
        {
          if (!col_cells.shared[0][i].domain.box.isempty())
          {             
            box_color_t shared[num_boxes]; 
            for (size_t k = 0; k <num_boxes; k++)
            {
              //Compute the bounding boxes for the entities of dimension d from the
              //shared cell box.
              shared[k].domain.box.resize(D); 

              for (size_t j = 0; j < D; j++)
              {
                shared[k].domain.box.lowerbnd[j] = col_cells.shared[0][i].domain.box.lowerbnd[j]; 
                shared[k].domain.box.upperbnd[j] = col_cells.shared[0][i].domain.box.upperbnd[j]
                                           + bnds_info[d][num_boxes*k+j+1]; 
               }

              //Obtain the tags from the shared cell box to tag the boundary entities
              //this shared sub-box covers. 
              std::vector<int> cbids = depent2cellbids(d,k);
              for (size_t b = 0; b < cbids.size(); b++)
                shared[k].domain.tag[cbids[b]] = col_cells.shared[0][i].domain.tag[cbids[b]];
        
              //Add rank of this ghost to the intermediate ghost domain
              shared[k].colors = col_cells.shared[0][i].colors; 
              col_ents.shared[k][i] = shared[k]; 
            }
          }
        }

       //Compute the exclusive box for entities of dimension d   
       box_color_t exclusive[num_boxes]; 
       for (size_t k = 0; k <num_boxes; k++)
       {
          exclusive_boxes[k].box.resize(D);  
   
          for (size_t j = 0; j < D; j++)
          {
            exclusive[k].box.lowerbnd[j] = col_cells.exclusive[0].box.lowerbnd[j];
            exclusive[k].box.upperbnd[j] = col_cells.exclusive[0].box.upperbnd[j]
                                                 + bnds_info[d][num_boxes*k+j+1];
          }

          //Obtain the tags from the exclusive cell box to tag the boundary entities
          //this exclusive sub-box covers. 
          std::vector<int> cbids = depent2cellbids(d,k);
          for (size_t b = 0; b < cbids.size(); b++)
            exclusive[k].domain.tag[cbids[b]] = col_cells.exclusive[0].domain.tag[cbids[b]];      

         exclusive[k].colors = col_cells.exclusive[0].colors; 
         col_ents.exclusive[k] = exclusive[k]; 
        } // end loop for exclusive boxes

       //Compute the domain halo boxes for entities of dimension d  
       /for (size_t i = 0 ; i < col_cells.domain_halo[0].size(); i++){
         box_tag_t halos[num_boxes]; 

         for (size_t k = 0; k <num_boxes; k++)
         {
            halos[k].resize(D); 

            for (size_t j = 0; j < D; j++)
            {
              halos[k].box.lowerbnd[j] = col_cells.domain_halo[0][i].box.lowerbnd[j]; 
              halos[k].box.upperbnd[j] = col_cells.domain_halo[0][i].box.upperbnd[j]
                                            + bnds_info[d][num_boxes*k+j+1]; 
            }

           //Obtain the tags from the domain halo cell box to tag the boundary entities
           //this domain halo sub-box covers. 
           std::vector<int> cbids = depent2cellbids(d,k);
           for (size_t b = 0; b < cbids.size(); b++)
             halos[k].tag[cbids[b]] = col_cells.domain_halo[0][i].tag[cbids[b]];      
 
           col_ents.domain_halo[k].push_back(halos[k]); 
          }
       } // end loop for domain halo boxes
       

       col_depents[d] = col_ents;       
 
      } // end loop over intermediate entities
    } //dimension check
  } //color_intermediate_entities_



   //Utility functions
   std::vector<int> G2G_bid_map_2d[8] = {{-1,0},{1,0},{0,-1},{0,1},
                                         {-1,-1,-1,0,0,-1},{0,-1,1,-1,1,0},
                                         {-1,1,-1,0,0,1},{1,0,1,1,0,1}};


   int S2G_bid_map_2d[8][2] = {{0,-1},{2,-1},{-1,0},{-1,2},
                                {0,0},{2,0},{0,2},{2,2}};


   std::vector<int> E2S_bid_map_2d[8] = {{3},{4},{1},{6},{0,1,3},,{1,2,4},{3,5,6},{4,6,7}};

   std::vector<int> D2CBIDS_2d[2] = {{2,2,0,1,2,3},{1,8,0,1,2,3,4,5,6,7}};

   auto ghost2ghost(int index, int bnd_id)
   {
    std::vector<int> gids; 
    if (D==2)
    {
      std::vector<int> vals = G2G_bid_map_2d[bnd_id];
      size_t nc = vals.size()/2; 
      size_t I, J, Ig, Jg;
      
      Jg = index % 3;
      Ig = index - 3*Jg;
      
      for (size_t i = 0; i < nc; i++)
      {
        I = Ig+vals[i*2];
        J = Jg+vals[i*2+1];
        if ((I>0 && I<3) && (J>0 && J<3))
          gids.push_back(3*J+I);
      }
      
    }
    return gids; 
   }      

   void ghost2ghost(int index, int bnd_id, std::vector<int> &gids)
   {

    gids.clear();
   
    if (D==2)
    {
      std::vector<int> vals = G2G_bid_map_2d[bnd_id];
      size_t nc = vals.size()/2; 
      size_t I, J, Ig, Jg;
      
      Jg = index % 3;
      Ig = index - 3*Jg;
      
      for (size_t i = 0; i < nc; i++)
      {
        I = Ig+vals[i*2];
        J = Jg+vals[i*2+1];
        if ((I>0 && I<3) && (J>0 && J<3))
          gids.push_back(3*J+I);
      }
    }
   }      

   auto shared2ghost(int index, int bnd_id)
   {
    std::vector<int> gids;
    if (D==2)
    {
      size_t I, J, Is, Js; 

      I = S2G_bid_map_2d[bnd_id][0];
      J = S2G_bid_map_2d[bnd_id][1];

      Js = index % 3;
      Is = index - 3*Js;

      if ((I==Is && J==-1) || (I==-1 && J==Js) || (I==Is && J==Js))
         ghost2ghost(4, bnd_id, gids);
    }
    return gids; 
   }  

   auto ghost2partition(int index, int bnd_id)
   {
    bool onpart = false; 

    if (D==2)
    {
      std::vector<int> vals = G2G_bid_map_2d[bnd_id];
      size_t nc = vals.size()/2; 
      size_t I, J, Ig, Jg;
      
      Jg = index % 3;
      Ig = index - 3*Jg;
      
      for (size_t i = 0; i < nc; i++)
      {
        I = Ig+vals[i*2];
        J = Jg+vals[i*2+1];
        if (I==1 && J==1)
        {
          onpart = true;
          break;
        }
      }
    }
    return onpart; 
   }  
                          
   auto exclusive2shared(int bnd_id)
   {
    return E2S_bid_map_2d[bnd_id]; 
   } 

   auto depent2cellbids(int dim, int sub_bid)
   {
    std::vector<int> ids; 
    int nb = D2CBIDS_2d[d][0];
    int cnt = D2CBIDS_2d[d][1];
    for (size_t i = 0; i < cnt; i++)
      ids.push_back(D2CBIDS_2d[d][cnt*sub_bid+i+2])
    return ids; 
   }

}; // class simple_box_colorer_t

} // namespace coloring
} // namespace flecsi
#endif // simple_box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
