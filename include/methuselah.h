#pragma once

#include <cmath>
#include <memory>
#include <numeric>
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
  void setValue(T value) { *value.get() = value; }

 private:
  std::unique_ptr<T> value;
  bool const onBorder;
};

// Grid
// ====------------------------------------------------------------------------
std::vector<short> const VON_NEUMANN_2D_NEIGHBORHOOD{-3, -1, 1, 3};
std::vector<short> const MOORE_2D_NEIGHBORHOOD{-4, -3, -2, -1, 1, 2, 3, 4};
std::vector<short> const MOORE_1D_NEIGHBORHOOD{-1, 1};

enum Boundary { BOUNDED, TOROIDAL };

namespace {  // "Internal" helper functions
template <typename T>
T multiplyAll(const std::vector<T>& vec) {
  return std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<T>());
}

// Expand each dimension out one unit on each extremity,
// then subtract the size of the resulting shape from the
// original. This gives you the number of cells needed to
// pad the grid by one unit.
size_t determinePadding(const std::vector<size_t>& shape) {
  auto numDims = shape.size();
  auto expanded = std::vector<size_t>(shape);
  for (auto i = 0; i < numDims; ++i) {
    expanded[i] += 2;
  }

  auto size = multiplyAll<size_t>(shape);
  auto expandedSize = multiplyAll<size_t>(shape);
  return expandedSize - size;
}
}  // namespace

template <typename T>
class Grid {
 public:
  Grid(const std::vector<size_t>& shape, Boundary boundary,
       const std::vector<short>& neighborhood)
      : shape(shape),
        size(multiplyAll<size_t>(shape)),
        padding(determinePadding(shape)),
        dimensions(shape.size()),
        boundary(boundary),
        neighborhood(neighborhood) {}

 private:
  std::vector<size_t> const shape;
  size_t const size;
  size_t const padding;
  unsigned short int const dimensions;
  std::vector<short> const neighborhood;
  Boundary const boundary;
  std::vector<std::unique_ptr<Cell<T>>> cells;
};

}  // namespace methuselah