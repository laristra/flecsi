/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */
#include <flecsi/utils/geometry/point.hh>

//----------------------------------------------------------------------------//
//! @file space_curve.h
//! @date 02/27/2019
//----------------------------------------------------------------------------//

namespace flecsi {

/*----------------------------------------------------------------------------*
 * class filling_curve_u
 * @brief Basic functionality for a space filling curve.
 * The filling_curves are built using a CRTP pattern.
 * This base class features the generic function to generate filling curves
 * and is specialized but \ref hilbert_curve_u and \ref morton_curve_u
 *----------------------------------------------------------------------------*/
template<size_t DIM, typename T, class DERIVED>
class filling_curve_u
{
  static constexpr size_t dimension = DIM;
  using int_t = T;
  using point_t = point_u<double, dimension>;
  using range_t = std::array<point_t, 2>;

protected:
  static constexpr size_t bits_ = sizeof(int_t) * 8; //! Maximum number of bits
  static constexpr size_t max_depth_ =
    (bits_ - 1) /
    dimension; //! Maximum
               //! depth reachable regarding the size of the memory word used
  static constexpr size_t max_value_ = int_t(1) << (max_depth_ - 1);

  static double min_, scale_;
  static range_t range_;
  int_t value_;
  filling_curve_u(int_t value) : value_(value) {}

public:
  filling_curve_u() : value_(0) {}
  filling_curve_u(const filling_curve_u & key) : value_(key) {}
  ~filling_curve_u() {
    value_ = 0;
  }

  static size_t max_depth() {
    return max_depth_;
  }

  //! Smallest value possible at max_depth considering the root
  static constexpr DERIVED min() {
    return DERIVED(int_t(1) << max_depth_ * dimension);
  }
  //! Biggest value possible at max_depth considering the root
  static constexpr DERIVED max() {
    int_t id = ~static_cast<int_t>(0);
    int_t remove = int_t(1) << max_depth_ * dimension;
    for(size_t i = max_depth_ * dimension + 1; i < bits_; ++i) {
      id ^= int_t(1) << i;
    } // for
    return DERIVED(id);
  }
  /*! Get the root id (depth 0) */
  static constexpr DERIVED root() {
    return DERIVED(int_t(1));
  }
  /*! Get the null id. */
  static constexpr DERIVED null() {
    return DERIVED(0);
  }
  /*! Check if value_ is null. */
  constexpr bool is_null() const {
    return value_ == int_t(0);
  }
  /*! Find the depth of this key */
  size_t depth() const {
    int_t id = value_;
    size_t d = 0;
    while(id >>= dimension)
      ++d;
    return d;
  }
  /*! Push bits onto the end of this id. */
  void push(int_t bits) {
    assert(bits < int_t(1) << dimension);
    value_ <<= dimension;
    value_ |= bits;
  }
  /*! Pop the bits of greatest depth off this id. */
  void pop() {
    assert(depth() > 0);
    value_ >>= dimension;
  }

  //! Search for the depth were two keys are in conflict
  int conflict_depth(filling_curve_u key_a, filling_curve_u key_b) {
    int conflict = max_depth_;
    while(key_a != key_b) {
      key_a.pop();
      key_b.pop();
      --conflict;
    } // while
    return conflict;
  }
  //! Pop and return the digit popped
  int pop_value() {
    assert(depth() > 0);
    int poped = 0;
    poped = value_ & ((1 << (dimension)) - 1);
    assert(poped < (1 << dimension));
    value_ >>= dimension;
    return poped;
  }
  //! Return the last digit of the key
  int last_value() {
    int poped = 0;
    poped = value_ & ((1 << (dimension)) - 1);
    return poped;
  }
  //! Pop the depth d bits from the end of this key.
  void pop(size_t d) {
    assert(d >= depth());
    value_ >>= d * dimension;
  }

  //! Return the parent of this key (depth - 1)
  constexpr filling_curve_u parent() const {
    return DERIVED(value_ >> dimension);
  }

  //! Truncate this key until it is of depth to_depth.
  void truncate(size_t to_depth) {
    size_t d = depth();
    if(d < to_depth) {
      return;
    }
    value_ >>= (d - to_depth) * dimension;
  }
  //! Output a key using oct in 3d and poping values for 2 and 1D
  void output_(std::ostream & ostr) const {
    if constexpr(dimension == 3) {
      ostr << std::oct << value_ << std::dec;
    }
    else {
      std::string output;
      filling_curve_u id = *this;
      int poped;
      while(id != root()) {
        poped = id.pop_value();
        output.insert(0, std::to_string(poped));
      } // while
      output.insert(output.begin(), '1');
      ostr << output.c_str();
    } // if else
  }
  //! Get the value associated to this key
  int_t value() const {
    return value_;
  }
  //! Convert this key to coordinates in range.
  virtual void coordinates(point_t & p) {}

  // Operators
  filling_curve_u & operator=(const filling_curve_u & bid) {
    value_ = bid.value_;
    return *this;
  }

  constexpr bool operator==(const filling_curve_u & bid) const {
    return value_ == bid.value_;
  }

  constexpr bool operator<=(const filling_curve_u & bid) const {
    return value_ <= bid.value_;
  }

  constexpr bool operator>=(const filling_curve_u & bid) const {
    return value_ >= bid.value_;
  }

  constexpr bool operator>(const filling_curve_u & bid) const {
    return value_ > bid.value_;
  }

  constexpr bool operator<(const filling_curve_u & bid) const {
    return value_ < bid.value_;
  }

  constexpr bool operator!=(const filling_curve_u & bid) const {
    return value_ != bid.value_;
  }

  operator int_t() const {
    return value_;
  }

}; // class filling_curve_u

//! output for filling_curve_u using output_ function defined in the class
template<size_t D, typename T, class DER>
std::ostream &
operator<<(std::ostream & ostr, const filling_curve_u<D, T, DER> & k) {
  k.output_(ostr);
  return ostr;
}

/*----------------------------------------------------------------------------*
 * class hilbert_curve_u
 * @brief Implementation of the hilbert peano space filling curve in
 * 1, 2 and 3d
 *----------------------------------------------------------------------------*/
template<size_t DIM, typename T>
class hilbert_curve_u : public filling_curve_u<DIM, T, hilbert_curve_u<DIM, T>>
{
  using int_t = T;
  static constexpr size_t dimension = DIM;
  using coord_t = std::array<int_t, dimension>;
  using point_t = point_u<double, dimension>;
  using range_t = std::array<point_t, 2>;

  using filling_curve_u<DIM, T, hilbert_curve_u>::value_;
  using filling_curve_u<DIM, T, hilbert_curve_u>::max_depth_;
  using filling_curve_u<DIM, T, hilbert_curve_u>::max_value_;

  using filling_curve_u<DIM, T, hilbert_curve_u>::min_;
  using filling_curve_u<DIM, T, hilbert_curve_u>::scale_;
  using filling_curve_u<DIM, T, hilbert_curve_u>::range_;

public:
  hilbert_curve_u() : filling_curve_u<DIM, T, hilbert_curve_u>() {}
  hilbert_curve_u(const int_t & id)
    : filling_curve_u<DIM, T, hilbert_curve_u>(id) {}
  hilbert_curve_u(const hilbert_curve_u & bid)
    : filling_curve_u<DIM, T, hilbert_curve_u>(bid.value_) {}
  hilbert_curve_u(const point_t & p)
    : hilbert_curve_u(p, filling_curve_u<DIM, T, hilbert_curve_u>::max_depth_) {
  }

  //! Hilbert key is always generated to the max_depth_ and then truncated
  //! otherwise the keys will not be the same
  hilbert_curve_u(const point_t & p, const size_t depth) {
    *this = filling_curve_u<DIM, T, hilbert_curve_u>::min();
    assert(depth <= max_depth_);
    std::array<int_t, dimension> coords;
    // Convert the position to integer
    for(int i = 0; i < dimension; ++i) {
      coords[i] = (p[i] - min_) / scale_ * max_value_;
    }
    // Handle 1D case
    if constexpr(dimension == 1) {
      assert(value_ & 1UL << max_depth_);
      value_ |= coords[0] >> dimension;
      value_ >>= (max_depth_ - depth);
      return;
    }
    int shift = max_depth_ - 1;
    for(int_t mask = max_value_ >> 1; mask > 0; mask >>= 1, --shift) {
      std::array<bool, dimension> r;
      if constexpr(dimension == 2) {
        r = {!!(mask & coords[0]), !!(mask & coords[1])};
        int_t bits = ((3 * r[0]) ^ r[1]);
        value_ |= bits << (shift * 2);
        rotation2d(mask, coords, r);
      }
      if constexpr(dimension == 3) {
        r = {!!(mask & coords[0]), !!(mask & coords[1]), !!(mask & coords[2])};
        int_t bits = (7 * r[0]) ^ (3 * r[1]) ^ r[2];
        value_ |= bits << (shift * 3);
        unrotation3d(mask, coords, r);
      }
    }
    assert(value_ & int_t(1) << (max_depth_ * dimension));
    // Then truncate the key to the depth
    value_ >>= (max_depth_ - depth) * dimension;
  }

  static void set_range(const range_t & range) {
    range_ = range;
    for(int i = 0; i < dimension; ++i) {
      min_ = range_[0][i];
      scale_ = range_[1][i] - min_;
    }
  }

  /*! Convert this id to coordinates in range. */
  void coordinates(point_t & p) {
    int_t key = value_;
    std::array<int_t, dimension> coords = {};
    int i = 0;
    for(int_t mask = int_t(1); mask <= max_value_; mask <<= 1) {
      std::array<bool, dimension> r = {};
      if constexpr(dimension == 3) {
        r[0] = (!!(key & 4));
        r[1] = (!!(key & 2)) ^ r[0];
        r[2] = (!!(key & 1)) ^ r[0] ^ r[1];
        rotation3d(mask, coords, r);
        coords[0] += r[0] * mask;
        coords[1] += r[1] * mask;
        coords[2] += r[2] * mask;
      }
      if constexpr(dimension == 2) {
        r[0] = (!!(key & 2));
        r[1] = (!!(key & 1)) ^ r[0];
        // r[0] = r[0] ^ r[1];
        rotation2d(mask, coords, r);
        coords[0] += r[0] * mask;
        coords[1] += r[1] * mask;
      }
      key >>= dimension;
    }

    assert(key == int_t(1));
    for(int j = 0; j < dimension; ++j) {
      p[j] = min_ + scale_ * static_cast<double>(coords[j]) / max_value_ / 2;
    } // for
  }

private:
  void rotation2d(const int_t & n,
    std::array<int_t, dimension> & coords,
    const std::array<bool, dimension> & bits) {
    if(bits[1] == 0) {
      if(bits[0] == 1) {
        coords[0] = n - 1 - coords[0];
        coords[1] = n - 1 - coords[1];
      }
      // Swap X-Y or Z
      std::swap(coords[0], coords[1]);
    }
  }

  void rotate_90_x(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {tmp[0], n - tmp[2] - 1, tmp[1]};
  }
  void rotate_90_y(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {tmp[2], tmp[1], n - tmp[0] - 1};
  }
  void rotate_90_z(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {n - tmp[1] - 1, tmp[0], tmp[2]};
  }
  void rotate_180_x(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {tmp[0], n - tmp[1] - 1, n - tmp[2] - 1};
  }
  void rotate_270_x(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {tmp[0], tmp[2], n - tmp[1] - 1};
  }
  void rotate_270_y(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {n - tmp[2] - 1, tmp[1], tmp[0]};
  }
  void rotate_270_z(const int_t & n, coord_t & coords) {
    coord_t tmp = coords;
    coords = {tmp[1], n - tmp[0] - 1, tmp[2]};
  }

  void rotation3d(const int_t & n,
    coord_t & coords,
    const std::array<bool, dimension> & r) {
    if(!r[0] && !r[1] && !r[2]) {
      // Left front bottom
      rotate_270_z(n, coords);
      rotate_270_x(n, coords);
    }
    else if(!r[0] && r[2]) {
      // Left top
      rotate_90_z(n, coords);
      rotate_90_y(n, coords);
    }
    else if(r[1] && !r[2]) {
      // Back bottom
      rotate_180_x(n, coords);
    }
    else if(r[0] && r[2]) {
      // Right top
      rotate_270_z(n, coords);
      rotate_270_y(n, coords);
    }
    else if(r[0] && !r[2] && !r[1]) {
      // Right front bottom
      rotate_90_y(n, coords);
      rotate_90_z(n, coords);
    }
  }

  void unrotation3d(const int_t & n,
    coord_t & coords,
    const std::array<bool, dimension> & r) {
    if(!r[0] && !r[1] && !r[2]) {
      // Left front bottom
      rotate_90_x(n, coords);
      rotate_90_z(n, coords);
    }
    else if(!r[0] && r[2]) {
      // Left top
      rotate_270_y(n, coords);
      rotate_270_z(n, coords);
    }
    else if(r[1] && !r[2]) {
      // Back bottom
      rotate_180_x(n, coords);
    }
    else if(r[0] && r[2]) {
      // Right top
      rotate_90_y(n, coords);
      rotate_90_z(n, coords);
    }
    else if(r[0] && !r[2] && !r[1]) {
      // Right front bottom
      rotate_270_z(n, coords);
      rotate_270_y(n, coords);
    }
  }
}; // class hilbert

/*----------------------------------------------------------------------------*
 * class morton_curve_u
 * @brief Implementation of the Morton space filling curve (Z ordering)
 *----------------------------------------------------------------------------*/
template<size_t DIM, typename T>
class morton_curve_u : public filling_curve_u<DIM, T, morton_curve_u<DIM, T>>
{

  using int_t = T;
  static constexpr size_t dimension = DIM;
  using coord_t = std::array<int_t, dimension>;
  using point_t = point_u<double, dimension>;
  using range_t = std::array<point_t, 2>;

  using filling_curve_u<DIM, T, morton_curve_u>::value_;
  using filling_curve_u<DIM, T, morton_curve_u>::max_depth_;
  using filling_curve_u<DIM, T, morton_curve_u>::max_value_;

  using filling_curve_u<DIM, T, morton_curve_u>::bits_;
  using filling_curve_u<DIM, T, morton_curve_u>::min_;
  using filling_curve_u<DIM, T, morton_curve_u>::scale_;
  using filling_curve_u<DIM, T, morton_curve_u>::range_;

public:
  morton_curve_u() : filling_curve_u<DIM, T, morton_curve_u>() {}
  morton_curve_u(const int_t & id)
    : filling_curve_u<DIM, T, morton_curve_u>(id) {}
  morton_curve_u(const morton_curve_u & bid)
    : filling_curve_u<DIM, T, morton_curve_u>(bid.value_) {}
  morton_curve_u(const point_t & p)
    : morton_curve_u(p, filling_curve_u<DIM, T, morton_curve_u>::max_depth_) {}

  //! Morton key can be generated directly up to the right depth
  morton_curve_u(const point_t & p, const size_t depth) {
    *this = filling_curve_u<DIM, T, morton_curve_u>::min();
    assert(depth <= max_depth_);
    std::array<int_t, dimension> coords;
    for(size_t i = 0; i < dimension; ++i) {
      coords[i] =
        (p[i] - min_) / scale_ * (int_t(1) << (bits_ - 1) / dimension);
    }
    size_t k = 0;
    for(size_t i = max_depth_ - depth; i < max_depth_; ++i) {
      for(size_t j = 0; j < dimension; ++j) {
        int_t bit = (coords[j] & int_t(1) << i) >> i;
        value_ |= bit << (k * dimension + j);
      } // for
      ++k;
    } // for
  } // morton_curve_u

  static void set_range(const range_t & range) {
    range_ = range;
    for(int i = 0; i < dimension; ++i) {
      min_ = range_[0][i];
      scale_ = range_[1][i] - min_;
    }
  }

  /*! Convert this id to coordinates in range. */
  void coordinates(point_t & p) {
    std::array<int_t, dimension> coords;
    coords.fill(int_t(0));
    int_t id = value_;
    size_t d = 0;
    while(id >> dimension != int_t(0)) {
      for(size_t j = 0; j < dimension; ++j) {
        coords[j] |= (((int_t(1) << j) & id) >> j) << d;
      } // for
      id >>= dimension;
      ++d;
    } // while
    constexpr int_t m = (int_t(1) << max_depth_) - 1;
    for(size_t j = 0; j < dimension; ++j) {
      coords[j] <<= max_depth_ - d;
      p[j] = min_ + scale_ * static_cast<double>(coords[j]) / m;
    } // for
  } //  coordinates
}; // class morton

template<size_t DIM, typename T, class DERIVED>
double filling_curve_u<DIM, T, DERIVED>::min_ = 0;
template<size_t DIM, typename T, class DERIVED>
typename filling_curve_u<DIM, T, DERIVED>::range_t
  filling_curve_u<DIM, T, DERIVED>::range_ = {};
template<size_t DIM, typename T, class DERIVED>
double filling_curve_u<DIM, T, DERIVED>::scale_ = 0;

} // namespace flecsi
