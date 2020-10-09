/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Provides a portable random number generator.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <random>

//! \brief A simple, portable random number generator that should yield the
//!        reproduceable results accross all platforms.
class pseudo_random
{
public:
  //! \brief The main constructor.
  //! \param [in] seed  The seed to use on constructon.
  pseudo_random(unsigned seed = 0) : rng_(seed) {}

  //! \brief Generate a new random number with a uniform distribution between
  //!        [0, 1).
  double uniform() {
    return double(rng_()) / (rng_.max)();
  }

  //! \brief Generate a new random number with a uniform distribution between
  //!        [a, b).
  double uniform(double a, double b) {
    return a + (b - a) * uniform();
  }

private:
  //! \brief Use one of the build in C++11 random number generators.
  std::mt19937 rng_;
};
