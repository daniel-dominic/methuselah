#pragma once

#include <memory>
#include <vector>

namespace methuselah {

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

std::vector<short> const VON_NEUMANN_2D_NEIGHBORHOOD{-3, -1, 1, 3};
std::vector<short> const MOORE_2D_NEIGHBORHOOD{-4, -3, -2, -1, 1, 2, 3, 4};
std::vector<short> const MOORE_1D_NEIGHBORHOOD{-1, 1};

enum Geometry { BOUNDED_1D, TOROIDAL_1D, BOUNDED_2D, TOROIDAL_2D };

template <typename T>
class Grid {
 public:
  Grid(std::vector<short> neighborOffsets, Geometry geometry) : neighborhood(neighborOffsets), geometry(geometry) {}
  Grid() : neighborhood(MOORE_2D_NEIGHBORHOOD), geometry(TOROIDAL_2D) {}

 private:
  unsigned short const padding;
  std::vector<short> const neighborhood;
  Geometry const geometry;
  std::vector<std::unique_ptr<Cell<T>>> cells;
};

}  // namespace methuselah