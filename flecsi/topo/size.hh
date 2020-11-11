// Topology components used for storing sizes of other topologies.

#ifndef FLECSI_TOPO_SIZE_HH
#define FLECSI_TOPO_SIZE_HH

#include "flecsi/data/field.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/topo/core.hh"

#include <cmath> // ceil

namespace flecsi::topo {

struct index_base {
  struct coloring {
    coloring(size_t size) : size_(size) {}

    size_t size() const {
      return size_;
    }

  private:
    size_t size_;
  };
};

// A topology with one index point per color.
// Suitable for the single layout but not the ragged layout, which is
// implemented in terms of this topology.
template<class P>
struct color_category : index_base, data::partitioned<data::partition> {
  color_category(const coloring & c)
    : partitioned(data::make_region<P>({c.size(), 1})) {}
};
template<>
struct detail::base<color_category> {
  using type = index_base;
};

// A subtopology for storing/updating row sizes of a partition.
struct resize : specialization<color_category, resize> {
  // cslot is useless without a color function, but we don't need it.
  using Field = flecsi::field<data::partition::row, data::single>;
  static const Field::definition<resize> field;
  template<partition_privilege_t P>
  using accessor = data::accessor_member<field, privilege_pack<P>>;

  struct policy {
    // Smaller s favors monotonic size changes at the expense of oscillations.
    policy(std::size_t m = 0, // minimum total slots
      std::size_t e = 0, // minimum extra slots
      float l = 0, // lowest fill fraction allowed
      float h = 1, // highest
      float s = 0) // speed control on [0,1]
      : min(m), extra(e), lo(l), hi(h), slow(s ? std::pow(h / l, s) : 1) {}

    std::size_t operator()(std::size_t n, std::size_t cap) const {
      // n on [floor(lo*c),hi*c] preserves cap (although we have no mechanism
      // for a task to indicate to its caller the choice not to change it).
      const auto div = [n](float d) -> std::size_t {
        return std::nearbyint((n + .5f) / d);
      };
      return std::max(min,
        std::max(n + extra,
          n > hi * cap ? div(lo * slow)
                       : n >= std::size_t(lo * cap) ? cap : div(hi / slow)));
    }

  private:
    std::size_t min, extra;
    float lo, hi, slow;
  };
};
// Now that resize is complete:
inline const resize::Field::definition<resize> resize::field;

// To control initialization order:
struct with_size {
  with_size(std::size_t n, const resize::policy & p = {}) : sz(n), growth(p) {}
  auto sizes() {
    return resize::field(sz);
  }
  resize::core sz;
  resize::policy growth;
};

} // namespace flecsi::topo

#endif
