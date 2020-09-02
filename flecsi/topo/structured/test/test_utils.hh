/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef test_utils_hh
#define test_utils_hh

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include "flecsi/util/unit.hh"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>

namespace flecsi {
namespace topo {
namespace structured_impl {

void
print_part_primary_entity(const box_coloring & colored_cells) {

  int dim = colored_cells.mesh_dim;
  int nbids = pow(3, dim);

  // Print colored cells: overlay, exclusive, shared, ghost, domain-halos
  auto & ebox = colored_cells.exclusive[0];
  auto & shboxes = colored_cells.shared[0];
  auto & ghboxes = colored_cells.ghost[0];
  auto & dhboxes = colored_cells.domain_halo[0];
  auto & obox = colored_cells.overlay[0];
  auto & strides = colored_cells.strides[0];

  UNIT_CAPTURE() << "CELL COLORING" << std::endl;
  UNIT_CAPTURE() << "MESH DIM " << dim << std::endl;
  UNIT_CAPTURE() << "   ----->Overlay:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << obox.lowerbnd[i] << ", "
                   << obox.upperbnd[i] << std::endl;

  UNIT_CAPTURE() << "   ----->Strides:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << strides[i] << std::endl;

  UNIT_CAPTURE() << "   ----->Exclusive:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << ebox.domain.lowerbnd[i] << ", "
                   << ebox.domain.upperbnd[i] << std::endl;

  UNIT_CAPTURE() << " bid tags = [";
  for(int b = 0; b < nbids; ++b)
    UNIT_CAPTURE() << ebox.tag[b] << " ";
  UNIT_CAPTURE() << "]" << std::endl;

  UNIT_CAPTURE() << " colors = [";
  for(std::size_t c = 0; c < ebox.colors.size(); ++c)
    UNIT_CAPTURE() << ebox.colors[c] << " ";
  UNIT_CAPTURE() << "]" << std::endl;

  UNIT_CAPTURE() << "   ----->Shared:" << std::endl;
  for(std::size_t s = 0; s < shboxes.size(); ++s) {
    UNIT_CAPTURE() << "   ---------->Shared Box " << s << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << shboxes[s].domain.lowerbnd[i]
                     << ", " << shboxes[s].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << shboxes[s].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < shboxes[s].colors.size(); ++c)
      UNIT_CAPTURE() << shboxes[s].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // shared

  UNIT_CAPTURE() << "   ----->Ghost:" << std::endl;
  for(std::size_t g = 0; g < ghboxes.size(); ++g) {
    UNIT_CAPTURE() << "   ---------->Ghost Box " << g << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << ghboxes[g].domain.lowerbnd[i]
                     << ", " << ghboxes[g].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << ghboxes[g].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < ghboxes[g].colors.size(); ++c)
      UNIT_CAPTURE() << ghboxes[g].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // ghost

  UNIT_CAPTURE() << "   ----->Domain Halo:" << std::endl;
  for(std::size_t h = 0; h < dhboxes.size(); ++h) {
    UNIT_CAPTURE() << "   ---------->Domain Halo Box " << h << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << dhboxes[h].domain.lowerbnd[i]
                     << ", " << dhboxes[h].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << dhboxes[h].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < dhboxes[h].colors.size(); ++c)
      UNIT_CAPTURE() << dhboxes[h].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // domain_halo

} // print

void
print_part_all_entities(const box_coloring & colored_cells,
  const std::vector<box_coloring> & colored_depents) {
  int dim = colored_cells.exclusive[0].domain.dim;
  int nbids = pow(3, dim);

  // Print colored cells: overlay, exclusive, shared, ghost, domain-halos
  auto & ebox = colored_cells.exclusive[0];
  auto & shboxes = colored_cells.shared[0];
  auto & ghboxes = colored_cells.ghost[0];
  auto & dhboxes = colored_cells.domain_halo[0];
  auto & obox = colored_cells.overlay[0];
  auto & strides = colored_cells.strides[0];

  UNIT_CAPTURE() << "CELL COLORING" << std::endl;
  UNIT_CAPTURE() << "   ----->Overlay:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << obox.lowerbnd[i] << ", "
                   << obox.upperbnd[i] << std::endl;

  UNIT_CAPTURE() << "   ----->Strides:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << strides[i] << std::endl;

  UNIT_CAPTURE() << "   ----->Exclusive:" << std::endl;
  for(int i = 0; i < dim; ++i)
    UNIT_CAPTURE() << " dim " << i << " : " << ebox.domain.lowerbnd[i] << ", "
                   << ebox.domain.upperbnd[i] << std::endl;

  UNIT_CAPTURE() << " bid tags = [";
  for(int b = 0; b < nbids; ++b)
    UNIT_CAPTURE() << ebox.tag[b] << " ";
  UNIT_CAPTURE() << "]" << std::endl;

  UNIT_CAPTURE() << " colors = [";
  for(std::size_t c = 0; c < ebox.colors.size(); ++c)
    UNIT_CAPTURE() << ebox.colors[c] << " ";
  UNIT_CAPTURE() << "]" << std::endl;

  UNIT_CAPTURE() << "   ----->Shared:" << std::endl;
  for(std::size_t s = 0; s < shboxes.size(); ++s) {
    UNIT_CAPTURE() << "   ---------->Shared Box " << s << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << shboxes[s].domain.lowerbnd[i]
                     << ", " << shboxes[s].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << shboxes[s].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < shboxes[s].colors.size(); ++c)
      UNIT_CAPTURE() << shboxes[s].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // shared

  UNIT_CAPTURE() << "   ----->Ghost:" << std::endl;
  for(std::size_t g = 0; g < ghboxes.size(); ++g) {
    UNIT_CAPTURE() << "   ---------->Ghost Box " << g << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << ghboxes[g].domain.lowerbnd[i]
                     << ", " << ghboxes[g].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << ghboxes[g].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < ghboxes[g].colors.size(); ++c)
      UNIT_CAPTURE() << ghboxes[g].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // ghost

  UNIT_CAPTURE() << "   ----->Domain Halo:" << std::endl;
  for(std::size_t h = 0; h < dhboxes.size(); ++h) {
    UNIT_CAPTURE() << "   ---------->Domain Halo Box " << h << ":" << std::endl;
    for(int i = 0; i < dim; ++i)
      UNIT_CAPTURE() << " dim " << i << " : " << dhboxes[h].domain.lowerbnd[i]
                     << ", " << dhboxes[h].domain.upperbnd[i] << std::endl;

    UNIT_CAPTURE() << " bid tags = [";
    for(int b = 0; b < nbids; ++b)
      UNIT_CAPTURE() << dhboxes[h].tag[b] << " ";
    UNIT_CAPTURE() << "]" << std::endl;

    UNIT_CAPTURE() << " colors = [";
    for(std::size_t c = 0; c < dhboxes[h].colors.size(); ++c)
      UNIT_CAPTURE() << dhboxes[h].colors[c] << " ";
    UNIT_CAPTURE() << "]" << std::endl;
  } // domain_halo

  // Print colored depents: overlay, exclusive, shared, ghost, domain-halos
  for(std::size_t edim = 0; edim < colored_depents.size(); ++edim) {

    std::size_t de_nboxes = colored_depents[edim].num_boxes;
    ;
    auto & de_ebox = colored_depents[edim].exclusive;
    auto & de_shboxes = colored_depents[edim].shared;
    auto & de_ghboxes = colored_depents[edim].ghost;
    auto & de_dhboxes = colored_depents[edim].domain_halo;
    auto & de_obox = colored_depents[edim].overlay;
    auto & de_strides = colored_depents[edim].strides;

    UNIT_CAPTURE() << "DEPENDENT ENTITY OF DIM " << edim << " COLORING"
                   << std::endl;

    UNIT_CAPTURE() << "   ----->Overlay:" << std::endl;
    for(std::size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << " dim " << i << " : " << de_obox[n].lowerbnd[i]
                       << ", " << de_obox[n].upperbnd[i] << std::endl;
    }

    UNIT_CAPTURE() << "   ----->Strides:" << std::endl;
    for(std::size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << " dim " << i << " : " << de_strides[n][i]
                       << std::endl;
    }

    UNIT_CAPTURE() << "   ----->Exclusive:" << std::endl;
    for(std::size_t n = 0; n < de_nboxes; ++n) {
      UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
      for(int i = 0; i < dim; ++i)
        UNIT_CAPTURE() << " dim " << i << " : " << de_ebox[n].domain.lowerbnd[i]
                       << ", " << de_ebox[n].domain.upperbnd[i] << std::endl;

      UNIT_CAPTURE() << " bid tags = [";
      for(int b = 0; b < nbids; ++b)
        UNIT_CAPTURE() << de_ebox[n].tag[b] << " ";
      UNIT_CAPTURE() << "]" << std::endl;

      UNIT_CAPTURE() << " colors = [";
      for(std::size_t c = 0; c < de_ebox[n].colors.size(); ++c)
        UNIT_CAPTURE() << de_ebox[n].colors[c] << " ";
      UNIT_CAPTURE() << "]" << std::endl;
    } // de_nboxes

    UNIT_CAPTURE() << "   ----->Shared:" << std::endl;
    std::size_t nsh = de_shboxes[0].size();
    for(std::size_t s = 0; s < nsh; ++s) {
      UNIT_CAPTURE() << "   ---------->Shared Box " << s << ":" << std::endl;
      for(std::size_t n = 0; n < de_nboxes; ++n) {
        UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          UNIT_CAPTURE() << " dim " << i << " : "
                         << de_shboxes[n][s].domain.lowerbnd[i] << ", "
                         << de_shboxes[n][s].domain.upperbnd[i] << std::endl;

        UNIT_CAPTURE() << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          UNIT_CAPTURE() << de_shboxes[n][s].tag[b] << " ";
        UNIT_CAPTURE() << "]" << std::endl;

        UNIT_CAPTURE() << " colors = [";
        for(std::size_t c = 0; c < de_shboxes[n][s].colors.size(); ++c)
          UNIT_CAPTURE() << de_shboxes[n][s].colors[c] << " ";
        UNIT_CAPTURE() << "]" << std::endl;
      } // shared
    } // de_nboxes

    UNIT_CAPTURE() << "   ----->Ghost:" << std::endl;
    std::size_t ngh = de_ghboxes[0].size();
    for(std::size_t g = 0; g < ngh; ++g) {
      UNIT_CAPTURE() << "   ---------->Ghost Box " << g << ":" << std::endl;
      for(std::size_t n = 0; n < de_nboxes; ++n) {
        UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          UNIT_CAPTURE() << " dim " << i << " : "
                         << de_ghboxes[n][g].domain.lowerbnd[i] << ", "
                         << de_ghboxes[n][g].domain.upperbnd[i] << std::endl;

        UNIT_CAPTURE() << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          UNIT_CAPTURE() << de_ghboxes[n][g].tag[b] << " ";
        UNIT_CAPTURE() << "]" << std::endl;

        UNIT_CAPTURE() << " colors = [";
        for(std::size_t c = 0; c < de_ghboxes[n][g].colors.size(); ++c)
          UNIT_CAPTURE() << de_ghboxes[n][g].colors[c] << " ";
        UNIT_CAPTURE() << "]" << std::endl;
      } // ghost
    } // de_nboxes

    UNIT_CAPTURE() << "   ----->Domain Halo:" << std::endl;
    std::size_t ndh = de_dhboxes[0].size();
    for(std::size_t h = 0; h < ndh; ++h) {
      UNIT_CAPTURE() << "   ---------->Domain Halo Box " << h << ":"
                     << std::endl;
      for(std::size_t n = 0; n < de_nboxes; ++n) {
        UNIT_CAPTURE() << "  ------->box_id " << n << " " << std::endl;
        for(int i = 0; i < dim; ++i)
          UNIT_CAPTURE() << " dim " << i << " : "
                         << de_dhboxes[n][h].domain.lowerbnd[i] << ", "
                         << de_dhboxes[n][h].domain.upperbnd[i] << std::endl;

        UNIT_CAPTURE() << " bid tags = [";
        for(int b = 0; b < nbids; ++b)
          UNIT_CAPTURE() << de_dhboxes[n][h].tag[b] << " ";
        UNIT_CAPTURE() << "]" << std::endl;

        UNIT_CAPTURE() << " colors = [";
        for(std::size_t c = 0; c < de_dhboxes[n][h].colors.size(); ++c)
          UNIT_CAPTURE() << de_dhboxes[n][h].colors[c] << " ";
        UNIT_CAPTURE() << "]" << std::endl;
      } // domain halo
    } // de_nboxes
  } // edim

} // print

} // namespace structured_impl
} // namespace topo
} // namespace flecsi
#endif // test_utils_hh
