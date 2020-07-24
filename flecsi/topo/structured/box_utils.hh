/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef box_utils_hh
#define box_utils_hh

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>

namespace flecsi {
namespace topo {
namespace structured_impl {

void
print_boxes(const box_coloring & colored_cells,
  const std::vector<box_coloring> & colored_depents) {
  int dim = colored_cells.exclusive[0].domain.dim;
  int nbids = pow(3, dim);

  // Print colored cells: overlay, exclusive, shared, ghost, domain-halos
  auto ebox = colored_cells.exclusive[0];
  auto shboxes = colored_cells.shared[0];
  auto ghboxes = colored_cells.ghost[0];
  auto dhboxes = colored_cells.domain_halo[0];
  auto obox = colored_cells.overlay[0];
  auto strides = colored_cells.strides[0];

  std::cout << "CELL COLORING" << std::endl;
  std::cout << "   ----->Overlay:";
  for(int i = 0; i < dim; ++i)
    std::cout << " dim " << i << " : " << obox.lowerbnd[i] << ", "
              << obox.upperbnd[i] << std::endl;

  std::cout << "   ----->Strides:";
  for(int i = 0; i < dim; ++i)
    std::cout << " dim " << i << " : " << strides[i] << std::endl;

  std::cout << "   ----->Exclusive:";
  for(int i = 0; i < dim; ++i)
    std::cout << " dim " << i << " : " << ebox.domain.lowerbnd[i] << ", "
              << ebox.domain.upperbnd[i] << std::endl;

  std::cout << " bid tags = [";
  for(int b = 0; b < nbids; ++b)
    std::cout << ebox.tag[b] << " ";
  std::cout << "]" << std::endl;

  std::cout << " colors = [";
  for(size_t c = 0; c < ebox.colors.size(); ++c)
    std::cout << ebox.colors[c] << " ";
  std::cout << "]" << std::endl;

  std::cout << "   ----->Shared:";
  for(size_t s = 0; s < shboxes.size(); ++s) {
    std::cout << "   ---------->Shared Box " << s << ":";
    for(int i = 0; i < dim; ++i)
      std::cout << " dim " << i << " : " << shboxes[s].domain.lowerbnd[i]
                << ", " << shboxes[s].domain.upperbnd[i] << std::endl;

    std::cout << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      std::cout << shboxes[s].tag[b] << " ";
    std::cout << "]" << std::endl;

    std::cout << " colors = [";
    for(size_t c = 0; c < shboxes[s].colors.size(); ++c)
      std::cout << shboxes[s].colors[c] << " ";
    std::cout << "]" << std::endl;
  } // shared

  std::cout << "   ----->Ghost:";
  for(size_t g = 0; g < ghboxes.size(); ++g) {
    std::cout << "   ---------->Ghost Box " << g << ":";
    for(int i = 0; i < dim; ++i)
      std::cout << " dim " << i << " : " << ghboxes[g].domain.lowerbnd[i]
                << ", " << ghboxes[g].domain.upperbnd[i] << std::endl;

    std::cout << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      std::cout << ghboxes[g].tag[b] << " ";
    std::cout << "]" << std::endl;

    std::cout << " colors = [";
    for(size_t c = 0; c < ghboxes[g].colors.size(); ++c)
      std::cout << ghboxes[g].colors[c] << " ";
    std::cout << "]" << std::endl;
  } // ghost

  std::cout << "   ----->Domain Halo:";
  for(size_t h = 0; h < dhboxes.size(); ++h) {
    std::cout << "   ---------->Domain Halo Box " << h << ":";
    for(int i = 0; i < dim; ++i)
      std::cout << " dim " << i << " : " << dhboxes[h].domain.lowerbnd[i]
                << ", " << dhboxes[h].domain.upperbnd[i] << std::endl;

    std::cout << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      std::cout << dhboxes[h].tag[b] << " ";
    std::cout << "]" << std::endl;

    std::cout << " colors = [";
    for(size_t c = 0; c < dhboxes[h].colors.size(); ++c)
      std::cout << dhboxes[h].colors[c] << " ";
    std::cout << "]" << std::endl;
  } // domain_halo

  // Print colored depents: overlay, exclusive, shared, ghost, domain-halos
  for(size_t edim = 0; edim < colored_depents.size(); ++edim) {

    size_t de_nboxes = colored_depents[edim].num_boxes;
    ;
    auto de_ebox = colored_depents[edim].exclusive;
    auto de_shboxes = colored_depents[edim].shared;
    auto de_ghboxes = colored_depents[edim].ghost;
    auto de_dhboxes = colored_depents[edim].domain_halo;
    auto de_obox = colored_depents[edim].overlay;
    auto de_strides = colored_depents[edim].strides;

    std::cout << "DEPENDENT ENTITY OF DIM " << edim << " COLORING" << std::endl;

    std::cout << "   ----->Overlay:";
    for(size_t n = 0; n < de_nboxes; ++n) {
      std::cout << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        std::cout << " dim " << i << " : " << de_obox[n].lowerbnd[i] << ", "
                  << de_obox[n].upperbnd[i] << std::endl;
    }

    std::cout << "   ----->Strides:";
    for(size_t n = 0; n < de_nboxes; ++n) {
      std::cout << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        std::cout << " dim " << i << " : " << de_strides[n][i] << std::endl;
    }

    std::cout << "   ----->Exclusive:";
    for(size_t n = 0; n < de_nboxes; ++n) {
      std::cout << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        std::cout << " dim " << i << " : " << de_ebox[n].domain.lowerbnd[i]
                  << ", " << de_ebox[n].domain.upperbnd[i] << std::endl;

      std::cout << " bid tags = [";
      for(int b = 0; b < nbids; ++b)
        std::cout << de_ebox[n].tag[b] << " ";
      std::cout << "]" << std::endl;

      std::cout << " colors = [";
      for(size_t c = 0; c < de_ebox[n].colors.size(); ++c)
        std::cout << de_ebox[n].colors[c] << " ";
      std::cout << "]" << std::endl;
    } // de_nboxes

    std::cout << "   ----->Shared:";
    size_t nsh = de_shboxes[0].size();
    for(size_t s = 0; s < nsh; ++s) {
      std::cout << "   ---------->Shared Box " << s << ":";
      for(size_t n = 0; n < de_nboxes; ++n) {
        std::cout << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          std::cout << " dim " << i << " : "
                    << de_shboxes[n][s].domain.lowerbnd[i] << ", "
                    << de_shboxes[n][s].domain.upperbnd[i] << std::endl;

        std::cout << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          std::cout << de_shboxes[n][s].tag[b] << " ";
        std::cout << "]" << std::endl;

        std::cout << " colors = [";
        for(size_t c = 0; c < de_shboxes[n][s].colors.size(); ++c)
          std::cout << de_shboxes[n][s].colors[c] << " ";
        std::cout << "]" << std::endl;
      } // shared
    } // de_nboxes

    std::cout << "   ----->Ghost:";
    size_t ngh = de_ghboxes[0].size();
    for(size_t g = 0; g < ngh; ++g) {
      std::cout << "   ---------->Ghost Box " << g << ":";
      for(size_t n = 0; n < de_nboxes; ++n) {
        std::cout << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          std::cout << " dim " << i << " : "
                    << de_ghboxes[n][g].domain.lowerbnd[i] << ", "
                    << de_ghboxes[n][g].domain.upperbnd[i] << std::endl;

        std::cout << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          std::cout << de_ghboxes[n][g].tag[b] << " ";
        std::cout << "]" << std::endl;

        std::cout << " colors = [";
        for(size_t c = 0; c < de_ghboxes[n][g].colors.size(); ++c)
          std::cout << de_ghboxes[n][g].colors[c] << " ";
        std::cout << "]" << std::endl;
      } // ghost
    } // de_nboxes

    std::cout << "   ----->Domain Halo:";
    size_t ndh = de_dhboxes[0].size();
    for(size_t h = 0; h < ndh; ++h) {
      std::cout << "   ---------->Domain Halo Box " << h << ":";
      for(size_t n = 0; n < de_nboxes; ++n) {
        std::cout << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          std::cout << " dim " << i << " : "
                    << de_dhboxes[n][h].domain.lowerbnd[i] << ", "
                    << de_dhboxes[n][h].domain.upperbnd[i] << std::endl;

        std::cout << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          std::cout << de_dhboxes[n][h].tag[b] << " ";
        std::cout << "]" << std::endl;

        std::cout << " colors = [";
        for(size_t c = 0; c < de_dhboxes[n][h].colors.size(); ++c)
          std::cout << de_dhboxes[n][h].colors[c] << " ";
        std::cout << "]" << std::endl;
      } // domain halo
    } // de_nboxes
  } // edim

} // print

auto
bid2dim(int dim) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      map = std::vector<int>{0, 1, 0};
      break;
    case 2:
      map = std::vector<int>{0, 1, 0, 1, 2, 1, 0, 1, 0};
      break;
    case 3:
      map = std::vector<int>{0,
        1,
        0,
        1,
        2,
        1,
        0,
        1,
        0,
        1,
        2,
        1,
        2,
        3,
        2,
        1,
        2,
        1,
        0,
        1,
        0,
        1,
        2,
        1,
        0,
        1,
        0};
      break;
  }
  return map;
} // bid2dim

auto
dim2bid(int dim, int edim) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      if(edim == 0) {
        map = std::vector<int>{0, 2};
      }
      break;
    case 2:
      if(edim == 0) {
        map = std::vector<int>{0, 2, 6, 8};
      }
      else if(edim == 1) {
        map = std::vector<int>{1, 3, 5, 7};
      }
      break;
    case 3:
      if(edim == 0) {
        map = std::vector<int>{0, 2, 6, 8, 18, 20, 24, 26};
      }
      else if(edim == 1) {
        map = std::vector<int>{1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25};
      }
      else if(edim == 2) {
        map = std::vector<int>{4, 10, 12, 14, 16, 22};
      }
      break;
  }
  return map;
} // dim2bid

auto
bid2dir(int dim, int dir) {
  std::vector<int> map;
  switch(dim) {
    case 1:
      if(dir == 0) {
        map = std::vector<int>{0, 2, 1};
      }
      break;
    case 2:
      if(dir == 0) {
        map = std::vector<int>{0, 2, 1, 0, 2, 1, 0, 2, 1};
      }
      else if(dir == 1) {
        map = std::vector<int>{0, 0, 0, 2, 2, 2, 1, 1, 1};
      }
      break;
    case 3:
      if(dir == 0) {
        map = std::vector<int>{0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1,
          0,
          2,
          1};
      }
      else if(dir == 1) {
        map = std::vector<int>{0,
          0,
          0,
          2,
          2,
          2,
          1,
          1,
          1,
          0,
          0,
          0,
          2,
          2,
          2,
          1,
          1,
          1,
          0,
          0,
          0,
          2,
          2,
          2,
          1,
          1,
          1};
      }
      else if(dir == 2) {
        map = std::vector<int>{0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          2,
          2,
          2,
          2,
          2,
          2,
          2,
          2,
          2,
          1,
          1,
          1,
          1,
          1,
          1,
          1,
          1,
          1};
      }
      break;
  }

  return map;
} // bid2dir

auto
dim2bounds(int dim, int edim) {
  std::vector<std::vector<int>> map;
  switch(dim) {
    case 1:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1, 0, 2}};
      }
      break;
    case 2:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1, 1, 0, 2, 6, 8, 1, 3, 5, 7}};
      }
      else if(edim == 1) {
        map = std::vector<std::vector<int>>{{1, 0, 3, 5}, {0, 1, 1, 7}};
      }
      break;
    case 3:
      if(edim == 0) {
        map = std::vector<std::vector<int>>{{1,
          1,
          1,
          0,
          2,
          6,
          8,
          18,
          20,
          24,
          26,
          1,
          3,
          5,
          7,
          9,
          11,
          15,
          17,
          19,
          21,
          23,
          25,
          4,
          10,
          12,
          14,
          16,
          22}};
      }
      else if(edim == 1) {
        map = std::vector<std::vector<int>>{
          {1, 1, 0, 9, 11, 15, 17, 10, 12, 14, 16},
          {0, 1, 1, 1, 7, 19, 25, 4, 10, 16, 22},
          {1, 0, 1, 3, 5, 21, 23, 4, 12, 14, 22}};
      }
      else if(edim == 2) {
        map = std::vector<std::vector<int>>{
          {1, 0, 0, 12, 14}, {0, 1, 0, 10, 16}, {0, 0, 1, 4, 22}};
      }
      break;
  }

  return map;
} // dim2bounds

} // namespace structured_impl
} // namespace topo
} // namespace flecsi
#endif // box_utils_hh
