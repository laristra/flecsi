class Cell_$DOMAIN : public mesh_entity_t<2, $NUM_DOMAINS>{
public:
  Cell_$DOMAIN(mesh_topology_base_t& mesh)
  : mesh_(mesh){}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(flecsi::id_t cell_id, size_t dim, domain_connectivity<$NUM_DIMENSIONS> & c, flecsi::id_t * e){
    flecsi::id_t* v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[2];
    
    e[2] = v[1];
    e[3] = v[3];
    
    e[4] = v[0];
    e[5] = v[1];
    
    e[6] = v[2];
    e[7] = v[3];

    return {2, 2, 2, 2};
  }

private:
  mesh_topology_base_t& mesh_;
};

