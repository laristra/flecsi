// Topology components used for storing sizes of other topologies.

#ifndef FLECSI_TOPO_SIZE_HH
#define FLECSI_TOPO_SIZE_HH

#include "flecsi/data/field.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/topo/core.hh"

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
  using accessor = data::accessor_member<field, privilege_pack<ro>>;
};
// Now that resize is complete:
inline const resize::Field::definition<resize> resize::field;

// To control initialization order:
struct with_size {
  with_size(std::size_t n) : sz(n) {}
  auto sizes() {
    return resize::field(sz);
  }
  resize::core sz;
};

} // namespace flecsi::topo

#endif
