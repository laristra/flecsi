
#pragma once

// void
// compute_cofm(node * cofm, std::vector<body *> ents, std::vector<node *>
// nodes) {
/*  // Then compute the CoFM
  point_t coordinates = point_t{};
  double radius = 0; // bmax
  double mass = 0;
  size_t sub_entities = 0;
  double lap = 0;
  point_t bmin, bmax;
  for(int i = 0; i < gdimension; ++i) {
    bmax[i] = -DBL_MAX;
    bmin[i] = DBL_MAX;
  }
  // Compute the center of mass and mass
  for(int i = 0; i < ents.size(); ++i) {
    body * ent = ents[i];
    // This correspond to a body
    coordinates += ent->mass() * ent->coordinates();
    mass += ent->mass();
    ++sub_entities;
    for(int d = 0; d < gdimension; ++d) {
      bmin[d] = std::min(bmin[d], ent->coordinates()[d] - ent->radius() / 2.);
      bmax[d] = std::max(bmax[d], ent->coordinates()[d] + ent->radius() / 2.);
    } // for
  }
  for(int i = 0; i < nodes.size(); ++i) {
    // This correspond to another node
    node * c = nodes[i];
    coordinates += c->mass() * c->coordinates();
    mass += c->mass();
    sub_entities += c->sub_entities();
    for(int d = 0; d < gdimension; ++d) {
      bmin[d] = std::min(bmin[d], c->bmin()[d]);
      bmax[d] = std::max(bmax[d], c->bmax()[d]);
    } // for
  } // for
  assert(mass != 0.);
  // Compute the radius
  coordinates /= mass;
  for(int i = 0; i < ents.size(); ++i) {
    body * ent = ents[i];
    double dist = distance(coordinates, ent->coordinates());
    radius = std::max(radius, dist);
    lap = std::max(lap, dist + ent->radius());
  }
  for(int i = 0; i < nodes.size(); ++i) {
    node * c = nodes[i];
    double dist = distance(coordinates, c->coordinates());
    radius = std::max(radius, dist + c->radius());
    lap = std::max(lap, dist + c->radius() + c->lap());
  } // for
  // Register and quit this node
  cofm->set_coordinates(coordinates);
  cofm->set_radius(radius);
  cofm->set_mass(mass);
  cofm->set_sub_entities(sub_entities);
  cofm->set_lap(lap);
  cofm->set_bmin(bmin);
  cofm->set_bmax(bmax);

// Compute multipole mass moments
#ifdef fmm_order
  if constexpr(gdimension == 3) {
    fmm::compute_moments(cofm, ents, nodes);
  }
#endif*/
//}
