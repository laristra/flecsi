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

  // Coloring algo
  box_coloring_info_t<D> color(size_t grid_size[D],
    size_t nhalo,
    size_t nhalo_domain,
    size_t thru_dim,
    size_t ncolors[D]) override {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Assert that the number of partitions is equal to number of ranks
    int count = 1;
    for(size_t nc = 0; nc < D; ++nc)
      count = static_cast<int>(count * ncolors[nc]);
    assert(count == size);
    assert(thru_dim < D);

    // Obtain indices of the current rank
    size_t idx[D];
    get_indices(ncolors, rank, idx);

    // Step 1: Create bounding boxes for domain with and without halo
    box_t<D> domain;
    for(size_t i = 0; i < D; ++i) {
      domain.lowerbnd[i] = nhalo_domain;
      domain.upperbnd[i] = grid_size[i] + nhalo_domain - 1;
    }

    // Step 2: Compute the primary box bounds for the current rank
    auto pbox = create_primary_box(domain, ncolors, idx);

    // Step 2: Create colored box type and set its primary box
    // info to the one created in step 1.
    box_coloring_info_t<D> colbox;
    colbox.primary.box = pbox;
    colbox.primary.nhalo = nhalo;
    colbox.primary.nhalo_domain = nhalo_domain;
    colbox.primary.thru_dim = thru_dim;
    colbox.primary.onbnd.reset(0);

    for(size_t i = 0; i < D; ++i) {
      if(pbox.lowerbnd[i] == domain.lowerbnd[i])
        colbox.primary.onbnd.set(2 * i, 1);
      if(pbox.upperbnd[i] == domain.upperbnd[i])
        colbox.primary.onbnd.set(2 * i + 1, 1);
    }

    // Step 3: Compute exclusive and shared boxes from primary box info.
    create_exclusive_and_shared_boxes(colbox, ncolors, idx, rank);

    // Step 4: Compute ghost boxes
    create_ghost_boxes(colbox, ncolors, idx);

    // Step 5: Compute domain halo boxes
    create_domain_halo_boxes(colbox);

    return colbox;
  } // color

  auto create_primary_box(box_t<D> & domain, size_t ncolors[D], size_t idx[D]) {
    box_t<D> pbox;
    for(size_t i = 0; i < D; ++i) {
      size_t N = domain.upperbnd[i] - domain.lowerbnd[i] + 1;
      size_t ne = N / ncolors[i];

      pbox.lowerbnd[i] = domain.lowerbnd[i] + ne * idx[i];

      if(idx[i] != ncolors[i] - 1)
        pbox.upperbnd[i] = domain.lowerbnd[i] + ne * (idx[i] + 1) - 1;
      else
        pbox.upperbnd[i] = domain.upperbnd[i];
    }
    return pbox;
  } // create_primary_box

  template<size_t D_ = D>
  typename std::enable_if<D_ == 2>::type create_exclusive_and_shared_boxes(
    box_coloring_info_t<D> & colbox,
    size_t ncolors[D],
    size_t idx[D],
    size_t rank) {
    box_t<D> pbox = colbox.primary.box;
    size_t hl = colbox.primary.nhalo;
    size_t TD = colbox.primary.thru_dim;

    // Compute bounds for exclusive box
    box_t<D> ebox = pbox;

    for(size_t i = 0; i < D; ++i) {
      if(!colbox.primary.onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if(!colbox.primary.onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }

    colbox.exclusive.box = ebox;
    colbox.exclusive.colors.emplace_back(rank);

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for(size_t i = 0; i < D; ++i) {
      IM[i][0] = pbox.lowerbnd[i];
      IM[i][1] = ebox.lowerbnd[i];
      IM[i][2] = ebox.upperbnd[i];
      IM[i][3] = pbox.upperbnd[i];
    }

    box_t<D> sbox;
    box_color_t<D> scbox;
    size_t ind[D], idx_new[D], cval;

    {
      for(size_t j = 0; j < 3; ++j)
        for(size_t i = 0; i < 3; ++i) {
          bool flag;
          if((i == 1) && (j == 1))
            flag = false;
          else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
            flag = true;
          else if((i == 1) && (IM[0][i] == IM[0][i + 1]) &&
                  (IM[1][j] != IM[1][j + 1]))
            flag = true;
          else if((j == 1) && (IM[0][i] != IM[0][i + 1]) &&
                  (IM[1][j] == IM[1][j + 1]))
            flag = true;
          else
            flag = false;

          if(flag) {
            sbox.lowerbnd[0] = IM[0][i] + il[i];
            sbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            sbox.lowerbnd[1] = IM[1][j] + il[j];
            sbox.upperbnd[1] = IM[1][j + 1] - iu[j];

            scbox.box = sbox;
            scbox.colors.clear();

            ind[0] = i;
            ind[1] = j;

            // Add edge shared ranks
            for(size_t d = 0; d < D; ++d) {
              idx_new[0] = idx[0];
              idx_new[1] = idx[1];
              if(ind[d] == 0) {
                idx_new[d] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
              else if(ind[d] == 2) {
                idx_new[d] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
            }

            // Add corner vertex shared ranks
            if(TD == 0) {
              idx_new[0] = idx[0];
              idx_new[1] = idx[1];

              if((i == 0) && (j == 0)) {
                idx_new[0] -= 1;
                idx_new[1] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
              else if((i == 2) && (j == 0)) {
                idx_new[0] += 1;
                idx_new[1] -= 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
              else if((i == 0) && (j == 2)) {
                idx_new[0] -= 1;
                idx_new[1] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
              else if((i == 2) && (j == 2)) {
                idx_new[0] += 1;
                idx_new[1] += 1;
                cval = idx_new[0] + ncolors[0] * idx_new[1];
                scbox.colors.emplace_back(cval);
              }
            }
            colbox.shared.emplace_back(scbox);
          }
        }
    }
  } // create_exclusive_and_shared_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 3>::type create_exclusive_and_shared_boxes(
    box_coloring_info_t<D> & colbox,
    size_t ncolors[D],
    size_t idx[D],
    size_t rank) {
    box_t<D> pbox = colbox.primary.box;
    size_t hl = colbox.primary.nhalo;
    size_t TD = colbox.primary.thru_dim;

    // Compute bounds for exclusive box
    box_t<D> ebox = pbox;

    for(size_t i = 0; i < D; ++i) {
      if(!colbox.primary.onbnd[2 * i])
        ebox.lowerbnd[i] += hl;
      if(!colbox.primary.onbnd[2 * i + 1])
        ebox.upperbnd[i] -= hl;
    }

    colbox.exclusive.box = ebox;
    colbox.exclusive.colors.emplace_back(rank);

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for(size_t i = 0; i < D; ++i) {
      IM[i][0] = pbox.lowerbnd[i];
      IM[i][1] = ebox.lowerbnd[i];
      IM[i][2] = ebox.upperbnd[i];
      IM[i][3] = pbox.upperbnd[i];
    }

    box_t<D> sbox;
    box_color_t<D> scbox;
    size_t ind[D], idx_new[D], cval;

    {
      for(size_t k = 0; k < 3; ++k)
        for(size_t j = 0; j < 3; ++j)
          for(size_t i = 0; i < 3; ++i) {
            bool flag;
            if((i == 1) && (j == 1) && (k == 1))
              flag = false;
            else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                    (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if((i == 1) && (IM[0][i] == IM[0][i + 1]) &&
                    (IM[1][j] != IM[1][j + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if((j == 1) && (IM[1][j] == IM[1][j + 1]) &&
                    (IM[0][i] != IM[0][i + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if((k == 1) && (IM[2][k] == IM[2][k + 1]) &&
                    (IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
              flag = true;
            else if((i == 1) && (IM[0][i] == IM[0][i + 1]) && (j == 1) &&
                    (IM[1][j] == IM[1][j + 1]) && (IM[2][k] != IM[2][k + 1]))
              flag = true;
            else if((j == 1) && (IM[1][j] == IM[1][j + 1]) && (k == 1) &&
                    (IM[2][k] == IM[2][k + 1]) && (IM[0][i] != IM[0][i + 1]))
              flag = true;
            else if((k == 1) && (IM[2][k] == IM[2][k + 1]) && (i == 1) &&
                    (IM[0][i] == IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]))
              flag = true;
            else
              flag = false;

            if(flag) {
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
              for(size_t d = 0; d < D; ++d) {
                idx_new[0] = idx[0];
                idx_new[1] = idx[1];
                idx_new[2] = idx[2];
                if(ind[d] == 0) {
                  idx_new[d] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if(ind[d] == 2) {
                  idx_new[d] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
              }

              // Add edge shared ranks
              if(TD <= 1) {
                for(size_t d = 0; d < D; ++d) {
                  idx_new[0] = idx[0];
                  idx_new[1] = idx[1];
                  idx_new[2] = idx[2];

                  if((ind[im[d][0]] == 0) && (ind[im[d][1]] == 0)) {
                    idx_new[im[d][0]] -= 1;
                    idx_new[im[d][1]] -= 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if((ind[im[d][0]] == 2) && (ind[im[d][1]] == 0)) {
                    idx_new[im[d][0]] += 1;
                    idx_new[im[d][1]] -= 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if((ind[im[d][0]] == 0) && (ind[im[d][1]] == 2)) {
                    idx_new[im[d][0]] -= 1;
                    idx_new[im[d][1]] += 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                  if((ind[im[d][0]] == 2) && (ind[im[d][1]] == 2)) {
                    idx_new[im[d][0]] += 1;
                    idx_new[im[d][1]] += 1;
                    cval = idx_new[0] + ncolors[0] * idx_new[1] +
                           ncolors[0] * ncolors[1] * idx_new[2];
                    scbox.colors.emplace_back(cval);
                  }
                }
              }

              // Add corner vertex shared ranks
              if(TD == 0) {
                idx_new[0] = idx[0];
                idx_new[1] = idx[1];
                idx_new[2] = idx[2];

                if((i == 0) && (j == 0) && (k == 0)) {
                  idx_new[0] -= 1;
                  idx_new[1] -= 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 2) && (j == 0) && (k == 0)) {
                  idx_new[0] += 1;
                  idx_new[1] -= 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 0) && (j == 2) && (k == 0)) {
                  idx_new[0] -= 1;
                  idx_new[1] += 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 2) && (j == 2) && (k == 0)) {
                  idx_new[0] += 1;
                  idx_new[1] += 1;
                  idx_new[2] -= 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 0) && (j == 0) && (k == 2)) {
                  idx_new[0] -= 1;
                  idx_new[1] -= 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 2) && (j == 0) && (k == 2)) {
                  idx_new[0] += 1;
                  idx_new[1] -= 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 0) && (j == 2) && (k == 2)) {
                  idx_new[0] -= 1;
                  idx_new[1] += 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
                else if((i == 2) && (j == 2) && (k == 2)) {
                  idx_new[0] += 1;
                  idx_new[1] += 1;
                  idx_new[2] += 1;
                  cval = idx_new[0] + ncolors[0] * idx_new[1] +
                         ncolors[0] * ncolors[1] * idx_new[2];
                  scbox.colors.emplace_back(cval);
                }
              }
              colbox.shared.emplace_back(scbox);
            }
          }
    }

  } // create_exclusive_and_shared_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 2>::type create_ghost_boxes(
    box_coloring_info_t<D> & colbox,
    size_t ncolors[D],
    size_t idx[D]) {
    box_t<D> pbox = colbox.primary.box;
    size_t hl = colbox.primary.nhalo;
    size_t TD = colbox.primary.thru_dim;

    // Compute bounds for exclusive box
    box_t<D> gbox = pbox;

    for(size_t i = 0; i < D; ++i) {
      if(!colbox.primary.onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if(!colbox.primary.onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for(size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_t<D> ghbox;
    box_color_t<D> gcbox;
    size_t idx_new[D], cval;

    {
      int rankmap[9][3] = {{0, -1, -1}, {1, 0, -1}, {0, 1, -1}, {1, -1, 0},
        {2, 0, 0}, {1, 1, 0}, {0, -1, 1}, {1, 0, 1}, {0, 1, 1}};

      for(size_t j = 0; j < 3; ++j)
        for(size_t i = 0; i < 3; ++i) {
          if((i == 1) && (j == 1))
            continue;
          else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            ghbox.lowerbnd[0] = IM[0][i] + il[i];
            ghbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            ghbox.lowerbnd[1] = IM[1][j] + il[j];
            ghbox.upperbnd[1] = IM[1][j + 1] - iu[j];

            gcbox.colors.clear();
            size_t id = i + 3 * j;
            if(TD <= rankmap[id][0]) {
              idx_new[0] = idx[0] + rankmap[id][1];
              idx_new[1] = idx[1] + rankmap[id][2];

              cval = idx_new[0] + ncolors[0] * idx_new[1];
              gcbox.box = ghbox;
              gcbox.colors.emplace_back(cval);
              colbox.ghost.emplace_back(gcbox);
            }
          }
        }
    }
  } // create_ghost_boxes

  template<size_t D_ = D>
  typename std::enable_if<D_ == 3>::type create_ghost_boxes(
    box_coloring_info_t<D> & colbox,
    size_t ncolors[D],
    size_t idx[D]) {
    box_t<D> pbox = colbox.primary.box;
    size_t hl = colbox.primary.nhalo;
    size_t TD = colbox.primary.thru_dim;

    // Compute bounds for exclusive box
    box_t<D> gbox = pbox;

    for(size_t i = 0; i < D; ++i) {
      if(!colbox.primary.onbnd[2 * i])
        gbox.lowerbnd[i] -= hl;
      if(!colbox.primary.onbnd[2 * i + 1])
        gbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};
    size_t im[3][2] = {{0, 1}, {1, 2}, {2, 0}};

    size_t IM[D][4];
    for(size_t i = 0; i < D; ++i) {
      IM[i][0] = gbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = gbox.upperbnd[i];
    }

    box_t<D> ghbox;
    box_color_t<D> gcbox;
    size_t idx_new[D], cval;

    {
      int rankmap[27][4] = {{0, -1, -1, -1}, {1, 0, -1, -1}, {0, 1, -1, -1},
        {1, -1, 0, -1}, {2, 0, 0, -1}, {1, 1, 0, -1}, {0, -1, 1, -1},
        {1, 0, 1, -1}, {0, 1, 1, -1}, {1, -1, -1, 0}, {2, 0, -1, 0},
        {1, 1, -1, 0}, {2, -1, 0, 0}, {3, 0, 0, 0}, {2, 1, 0, 0}, {1, -1, 1, 0},
        {2, 0, 1, 0}, {1, 1, 1, 0}, {0, -1, -1, 1}, {1, 0, -1, 1},
        {0, 1, -1, 1}, {1, -1, 0, 1}, {2, 0, 0, 1}, {1, 1, 0, 1}, {0, -1, 1, 1},
        {1, 0, 1, 1}, {0, 1, 1, 1}};

      for(size_t k = 0; k < 3; ++k)
        for(size_t j = 0; j < 3; ++j)
          for(size_t i = 0; i < 3; ++i) {
            if((i == 1) && (j == 1) && (k == 1))
              continue;
            else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                    (IM[2][k] != IM[2][k + 1])) {
              ghbox.lowerbnd[0] = IM[0][i] + il[i];
              ghbox.upperbnd[0] = IM[0][i + 1] - iu[i];
              ghbox.lowerbnd[1] = IM[1][j] + il[j];
              ghbox.upperbnd[1] = IM[1][j + 1] - iu[j];
              ghbox.lowerbnd[2] = IM[2][k] + il[k];
              ghbox.upperbnd[2] = IM[2][k + 1] - iu[k];

              gcbox.colors.clear();
              size_t id = i + 3 * j + 9 * k;
              if(TD <= rankmap[id][0]) {
                idx_new[0] = idx[0] + rankmap[id][1];
                idx_new[1] = idx[1] + rankmap[id][2];
                idx_new[2] = idx[2] + rankmap[id][3];

                cval = idx_new[0] + ncolors[0] * idx_new[1] +
                       ncolors[0] * ncolors[1] * idx_new[2];
                gcbox.box = ghbox;
                gcbox.colors.emplace_back(cval);
                colbox.ghost.emplace_back(gcbox);
              }
            }
          }
    }
  } // create_ghost_boxes

  void create_domain_halo_boxes(box_coloring_info_t<D> & colbox) {
    box_t<D> pbox = colbox.primary.box;
    size_t hl = colbox.primary.nhalo_domain;

    // Compute bounds for domain with halo
    box_t<D> dbox = pbox;

    for(size_t i = 0; i < D; ++i) {
      if(colbox.primary.onbnd[2 * i])
        dbox.lowerbnd[i] -= hl;
      if(colbox.primary.onbnd[2 * i + 1])
        dbox.upperbnd[i] += hl;
    }

    size_t il[3] = {0, 0, 1};
    size_t iu[3] = {1, 0, 0};

    size_t IM[D][4];
    for(size_t i = 0; i < D; ++i) {
      IM[i][0] = dbox.lowerbnd[i];
      IM[i][1] = pbox.lowerbnd[i];
      IM[i][2] = pbox.upperbnd[i];
      IM[i][3] = dbox.upperbnd[i];
    }

    box_t<D> dhbox;

    if(D == 2) {
      for(size_t j = 0; j < 3; ++j)
        for(size_t i = 0; i < 3; ++i) {
          if((i == 1) && (j == 1))
            continue;
          else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1])) {
            dhbox.lowerbnd[0] = IM[0][i] + il[i];
            dhbox.upperbnd[0] = IM[0][i + 1] - iu[i];
            dhbox.lowerbnd[1] = IM[1][j] + il[j];
            dhbox.upperbnd[1] = IM[1][j + 1] - iu[j];
            colbox.domain_halo.emplace_back(dhbox);
          }
        }
    }
    else if(D == 3) {
      for(size_t k = 0; k < 3; ++k)
        for(size_t j = 0; j < 3; ++j)
          for(size_t i = 0; i < 3; ++i) {
            if((i == 1) && (j == 1) && (k == 1))
              continue;
            else if((IM[0][i] != IM[0][i + 1]) && (IM[1][j] != IM[1][j + 1]) &&
                    (IM[2][k] != IM[2][k + 1])) {
              dhbox.lowerbnd[0] = IM[0][i] + il[i];
              dhbox.upperbnd[0] = IM[0][i + 1] - iu[i];
              dhbox.lowerbnd[1] = IM[1][j] + il[j];
              dhbox.upperbnd[1] = IM[1][j + 1] - iu[j];
              dhbox.lowerbnd[2] = IM[2][k] + il[k];
              dhbox.upperbnd[2] = IM[2][k + 1] - iu[k];
              colbox.domain_halo.emplace_back(dhbox);
            }
          }
    }
  } // create_domain_halo_boxes

  void get_indices(size_t cobnds[D], int rank, size_t id[D]) {
    size_t rem, factor, value;

    rem = rank;
    for(size_t i = 0; i < D; ++i) {
      factor = 1;
      for(size_t j = 0; j < D - i - 1; ++j)
        factor *= cobnds[j];

      value = rem / factor;
      id[D - i - 1] = value;
      rem -= value * factor;
    }
  } // get_indices

}; // class simple_box_colorer_t

} // namespace coloring
} // namespace flecsi
#endif // simple_box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
