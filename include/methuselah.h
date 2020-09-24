#pragma once

#include <cmath>
#include <functional>
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
  Cell(std::unique_ptr<T> value, bool onBorder) : value(std::move(value)) {}
  Cell(T value, bool onBorder) : value(std::make_unique<T>(value)) {}
  Cell() : value(std::make_unique<T>()) {}

  T* getValue() { return value.get(); }
  void setValue(T value) { *value.get() = value; }

 private:
  std::unique_ptr<T> value;
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
       const std::vector<short>& neighborhood, T defaultValue,
       unsigned short int maxNeighborDistance = 1)
      : shape(shape),
        size(multiplyAll<size_t>(shape)),
        padding(determinePadding(shape) * maxNeighborDistance),
        dimensions(shape.size()),
        boundary(boundary),
        neighborhood(neighborhood),
        defaultValue(defaultValue),
        deadCell(std::make_unique<T>(defaultValue)) {
    for (auto i = 0; i < size; ++i) {
      cells.push_back(std::make_unique<T>(defaultValue));
    }

    paddedCells.resize(size + padding);

    switch (boundary) {
      case Boundary::BOUNDED:
        makeBounded();
        break;

      case Boundary::TOROIDAL:
        makeToroidal();
        break;
    }

    // auto halfPadding = padding / 2;
    // for (auto i = halfPadding; i < size + halfPadding; ++i) {
    // }
  }

 private:
  Cell<T>* makeBounded() {
    unsigned int halfPadding = padding / 2;
    for (auto i = 0; i < halfPadding; ++i) {
      paddedCells[i] = deadCell.get();
    }
    for (auto i = size + padding - 1; i > halfPadding; ++i) {
      paddedCells[i] = deadCell.get();
    }
  }

  Cell<T>* makeToroidal() {}

  std::vector<size_t> const shape;
  size_t const size;
  size_t const padding;
  unsigned short int const dimensions;
  std::vector<short> neighborhood;
  Boundary const boundary;

  T const defaultValue;
  std::unique_ptr<Cell<T>> const deadCell;
  std::vector<Cell<T>* const> paddedCells;
  std::vector<std::unique_ptr<Cell<T>>> cells;
};

}  // namespace methuselah