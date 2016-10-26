{
  vector<Vertex_$DOMAIN*> vs;

  size_t id = 0;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = mesh->make<Vertex_$DOMAIN>();
      mesh->add_entity<0, $DOMAIN>(v);
      vs.push_back(v); 
    }
  }

  id = 0;
  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = mesh->make<Cell_$DOMAIN>(*mesh);

      mesh->add_entity<2, $DOMAIN>(c);

      mesh->init_cell<$DOMAIN>(c,
                         {vs[i + j * width1],
                         vs[i + (j + 1) * width1],
                         vs[i + 1 + j * width1],
                         vs[i + 1 + (j + 1) * width1]}
                        );


    }
  }

  mesh->init<$DOMAIN>();
}
