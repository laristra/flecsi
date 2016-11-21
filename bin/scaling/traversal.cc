CINCH_CAPTURE() << "------------- forall cells, vertices" << endl;

for(auto cell : mesh->entities<2, $DOMAIN>()) {
  CINCH_CAPTURE() << "------------- cell id: " << cell.id() << endl;
  for(auto vertex : mesh->entities<0, $DOMAIN>(cell)) {
    CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    for(auto cell2 : mesh->entities<2, $DOMAIN>(vertex)) {
      CINCH_CAPTURE() << "+ cell2 id: " << cell2.id() << endl;
    }
  }
}

CINCH_CAPTURE() << "------------- forall cells, edges" << endl;

for(auto cell : mesh->entities<2, $DOMAIN>()) {
  CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
  for(auto edge : mesh->entities<1, $DOMAIN>(cell)) {
    CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
  }
}

CINCH_CAPTURE() << "------------- forall vertices, edges" << endl;

for(auto vertex : mesh->entities<0, $DOMAIN>()) {
  CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
  for(auto edge : mesh->entities<1, $DOMAIN>(vertex)) {
    CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
  }
}

CINCH_CAPTURE() << "------------- forall vertices, cells" << endl;

for(auto vertex : mesh->entities<0, $DOMAIN>()) {
  CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
  for(auto cell : mesh->entities<2, $DOMAIN>(vertex)) {
    CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
  }
}

CINCH_CAPTURE() << "------------- forall edges, cells" << endl;

for(auto edge : mesh->entities<1, $DOMAIN>()) {
  CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
  for(auto cell : mesh->entities<2, $DOMAIN>(edge)) {
    CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
  }
}

CINCH_CAPTURE() << "------------- forall edges, vertices" << endl;

for(auto edge : mesh->entities<1, $DOMAIN>()) {
  CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
  for(auto vertex : mesh->entities<0, $DOMAIN>(edge)) {
    CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
  }
}
