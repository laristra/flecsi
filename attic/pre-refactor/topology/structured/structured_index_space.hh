#ifndef flecsi_topology_structured_index_space_h
#define flecsi_topology_structured_index_space_h

#include "flecsi/topo/structured/box_types.hh"
#include "flecsi/topo/structured/unit_box.hh"
#include <vector>

namespace flecsi {
namespace topology {
namespace structured_impl {

template<size_t MESH_DIMENSION, size_t ENTITY_DIMENSION>
struct structured_index_space_u {

  template<bool BTAG>
  using ubox_u = unit_box_u<MESH_DIMENSION, ENTITY_DIMENSION, BTAG>;

  using box_t = flecsi::topology::structured_impl::box_t;
  using box_color_t = flecsi::topology::structured_impl::box_color_t;
  using box_coloring_t = flecsi::topology::structured_impl::box_coloring_t;

  void initialize(box_coloring_t & colored_is) {
    primary_ = (colored_is.entity_dim == colored_is.primary_dim);

    size_t nboxes = colored_is.num_boxes;
    int nbids = pow(3, MESH_DIMENSION);
    bool btag = true;
    std::vector<size_t> lbnds(nboxes * MESH_DIMENSION);
    std::vector<size_t> ubnds(nboxes * MESH_DIMENSION);
    std::vector<size_t> strs(nboxes * MESH_DIMENSION);
    std::vector<bool> tags(nboxes * nbids);
    std::cout << "nboxes = " << nboxes << std::endl;
    // Overlay boxes
    auto obox = colored_is.overlay;
    for(size_t i = 0; i < nboxes; i++)
      for(size_t j = 0; j < MESH_DIMENSION; j++) {
        lbnds[MESH_DIMENSION * i + j] = obox[i].lowerbnd[j];
        ubnds[MESH_DIMENSION * i + j] = obox[i].upperbnd[j];
        strs[MESH_DIMENSION * i + j] = colored_is.strides[i][j];
      }

    overlay_.initialize(lbnds, ubnds, strs, primary_);

    // Exclusive boxes
    auto ebox = colored_is.exclusive;
    for(size_t i = 0; i < nboxes; i++) {
      for(size_t j = 0; j < MESH_DIMENSION; j++) {
        lbnds[MESH_DIMENSION * i + j] =
          ebox[i].domain.lowerbnd[j] - obox[i].lowerbnd[j];
        ubnds[MESH_DIMENSION * i + j] =
          ebox[i].domain.upperbnd[j] - obox[i].lowerbnd[j];
        strs[MESH_DIMENSION * i + j] =
          obox[i].upperbnd[j] - obox[i].lowerbnd[j] + 1;
      }

      for(int b = 0; b < nbids; ++b)
        tags[nbids * i + b] = ebox[i].tag[b];
    }

    exclusive_.initialize(lbnds, ubnds, strs, btag, tags, primary_);

    // Shared boxes
    auto sbox = colored_is.shared;
    shared_.resize(sbox[0].size());
    for(size_t k = 0; k < sbox[0].size(); ++k) {
      for(size_t i = 0; i < nboxes; ++i) {
        for(size_t j = 0; j < MESH_DIMENSION; ++j) {
          lbnds[MESH_DIMENSION * i + j] =
            sbox[i][k].domain.lowerbnd[j] - obox[i].lowerbnd[j];
          ubnds[MESH_DIMENSION * i + j] =
            sbox[i][k].domain.upperbnd[j] - obox[i].lowerbnd[j];
          strs[MESH_DIMENSION * i + j] =
            obox[i].upperbnd[j] - obox[i].lowerbnd[j] + 1;
        }

        for(int b = 0; b < nbids; ++b)
          tags[nbids * i + b] = sbox[i][k].tag[b];
      }

      shared_[k].initialize(lbnds, ubnds, strs, btag, tags, primary_);
    }

    // Ghost boxes
    auto gbox = colored_is.ghost;
    ghost_.resize(gbox[0].size());
    for(size_t k = 0; k < gbox[0].size(); ++k) {
      for(size_t i = 0; i < nboxes; ++i) {
        for(size_t j = 0; j < MESH_DIMENSION; ++j) {
          lbnds[MESH_DIMENSION * i + j] =
            gbox[i][k].domain.lowerbnd[j] - obox[i].lowerbnd[j];
          ubnds[MESH_DIMENSION * i + j] =
            gbox[i][k].domain.upperbnd[j] - obox[i].lowerbnd[j];
          strs[MESH_DIMENSION * i + j] =
            obox[i].upperbnd[j] - obox[i].lowerbnd[j] + 1;
        }

        for(int b = 0; b < nbids; ++b)
          tags[nbids * i + b] = gbox[i][k].tag[b];
      }

      ghost_[k].initialize(lbnds, ubnds, strs, btag, tags, primary_);
    }

    // Domain halo boxes
    auto dbox = colored_is.domain_halo;
    domain_halo_.resize(dbox[0].size());
    for(size_t k = 0; k < dbox[0].size(); ++k) {
      for(size_t i = 0; i < nboxes; ++i) {
        for(size_t j = 0; j < MESH_DIMENSION; ++j) {
          lbnds[MESH_DIMENSION * i + j] =
            dbox[i][k].domain.lowerbnd[j] - obox[i].lowerbnd[j];
          ubnds[MESH_DIMENSION * i + j] =
            dbox[i][k].domain.upperbnd[j] - obox[i].lowerbnd[j];
          strs[MESH_DIMENSION * i + j] =
            obox[i].upperbnd[j] - obox[i].lowerbnd[j] + 1;
        }

        for(int b = 0; b < nbids; ++b)
          tags[nbids * i + b] = dbox[i][k].tag[b];
      }

      domain_halo_[k].initialize(lbnds, ubnds, strs, btag, tags, primary_);
    }

  } // initialize

  void convert_bounds_to_local(box_t & outer,
    box_t & inner,
    std::vector<size_t> strides) {
    strides.resize(MESH_DIMENSION);
    for(size_t j = 0; j < MESH_DIMENSION; ++j) {
      inner.lowerbnd[j] -= outer.lowerbnd[j];
      inner.upperbnd[j] -= outer.lowerbnd[j];
      strides[j] = outer.upperbnd[j] - outer.lowerbnd[j];
    }
  } // convert_bounds_to_local

  void convert_bounds_to_global(box_t & outer, box_t & inner) {
    for(size_t j = 0; j < MESH_DIMENSION; ++j) {
      inner.lowerbnd[j] += outer.lowerbnd[j];
      inner.upperbnd[j] += outer.lowerbnd[j];
    }
  } // convert_bounds_to_global

  auto overlay() {
    return overlay_;
  }

  auto exclusive() {
    return exclusive_;
  }

  auto shared() {
    return shared_;
  }

  auto ghost() {
    return ghost_;
  }

  auto domain_halo() {
    return domain_halo_;
  }

private:
  bool primary_ = false;
  ubox_u<false> overlay_;
  ubox_u<true> exclusive_;
  std::vector<ubox_u<true>> shared_;
  std::vector<ubox_u<true>> ghost_;
  std::vector<ubox_u<true>> domain_halo_;

}; // structured_index_space_storage_u
} // namespace structured_impl
} // namespace topology
} // namespace flecsi
#endif
