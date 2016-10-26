class Vertex_$DOMAIN : public mesh_entity_t<0, $NUM_DOMAINS>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  Vertex_$DOMAIN() = default;
  Vertex_$DOMAIN(mesh_topology_base_t &) {}
};

