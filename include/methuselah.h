#pragma once

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace methuselah {

// Exceptions
// ==========------------------------------------------------------------------
class NotImplementedException : public std::runtime_error {
 public:
  NotImplementedException(const std::string& arg) : std::runtime_error(arg) {}
  NotImplementedException() : NotImplementedException("") {}
};

// Cell
// ====------------------------------------------------------------------------
template <typename T>
class Cell {
 public:
  Cell(std::unique_ptr<T> value, bool onBorder)
      : value(std::move(value)), onBorder(onBorder) {}
  Cell(T value, bool onBorder)
      : value(std::make_unique<T>(value)), onBorder(onBorder) {}
  Cell() : value(std::make_unique<T>()) {}

  T* getValue() { return value.get(); }

 private:
  std::unique_ptr<T> value;
  bool const onBorder;
};

// Grid
// ====------------------------------------------------------------------------
std::vector<short> const VON_NEUMANN_2D_NEIGHBORHOOD{-3, -1, 1, 3};
std::vector<short> const MOORE_2D_NEIGHBORHOOD{-4, -3, -2, -1, 1, 2, 3, 4};
std::vector<short> const MOORE_1D_NEIGHBORHOOD{-1, 1};

enum BoundaryConditions { BOUNDED, TOROIDAL };

template <typename T>
class Grid {
 public:
  Grid(size_t size, unsigned short int dimensions,
       BoundaryConditions boundaryConditions, std::vector<short> neighborhood)
      : size(size),
        dimensions(dimensions),
        boundaryConditions(boundaryConditions),
        neighborhood(neighborhood),
  {}

 private:
  size_t const size;
  size_t const padding;
  unsigned short int const dimensions;
  std::vector<short> const neighborhood;
  BoundaryConditions const boundaryConditions;
  std::vector<std::unique_ptr<Cell<T>>> cells;

  // static size_t determinePadding(size_t size, short maxNeighborDistance,
  //                                BoundaryConditions boundaryConditions) {
  //   switch (boundaryConditions) {
  //     case Geometry.BOUNDED:
  //       return (int)(std::pow(2, dimensions)) * ;

  //     default:
  //       break;
  //   }
  // }
};

}  // namespace methuselah