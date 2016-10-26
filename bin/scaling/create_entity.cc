case $DOMAIN:{
  switch(D){
    case 1:
      return mesh->make<Edge_$DOMAIN>(*mesh);
    default:
      assert(false && "invalid topological dimension");
  }
  break;
}
