#pragma once

#include <assert.h>

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

class InvalidOperationException : public std::runtime_error {
 public:
  InvalidOperationException(const std::string& arg) : std::runtime_error(arg) {}
  InvalidOperationException() : InvalidOperationException("") {}
};

// Cell
// ====------------------------------------------------------------------------
template <typename T>
class Cell {
 public:
  Cell(const T& val)
      : value(std::make_unique<T>(val)),
        futureValue(std::make_unique<T>(val)) {}
  Cell() {}

  virtual T* get() { return value.get(); }
  virtual T* getFuture() { return futureValue.get(); }
  virtual bool isOutOfBounds() { return false; }

  virtual void set(const T& val) {
    *value = val;
    *futureValue = val;
  }

  // TODO: Profile & make sure you're not wasting too much time
  //       frequently calling the copy constructor.
  virtual void incrementTime() { *value = *futureValue; }

 private:
  std::unique_ptr<T> value;
  std::unique_ptr<T> futureValue;
};

template <typename T>
class OutOfBoundsCell final : public Cell<T> {
 public:
  OutOfBoundsCell(T* ptr) : ptr(ptr) {}
  OutOfBoundsCell() : ptr(nullptr) {}

  T* get() { return ptr; }
  T* getFuture() {
    throw InvalidOperationException("Cannot get future value for OOB cell.");
  }
  void set(T* ptr) { this->ptr = ptr; }
  bool isOutOfBounds() { return true; }
  void incrementTime() {}

 private:
  T* ptr;
};

// Grid
// ====------------------------------------------------------------------------
// Convert these to functions sensitive to the shape of the grid
std::vector<short> const VON_NEUMANN_2D_NEIGHBORHOOD{-3, -1, 1, 3};
std::vector<short> const MOORE_2D_NEIGHBORHOOD{-4, -3, -2, -1, 1, 2, 3, 4};
std::vector<short> const MOORE_1D_NEIGHBORHOOD{-1, 1};

enum Wrapping { BOUNDED, TOROIDAL };

namespace {  // Helper functions
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
  auto expandedSize = multiplyAll<size_t>(expanded);
  return expandedSize - size;
}

std::vector<size_t> allZeros(size_t length) {
  auto result = std::vector<size_t>();
  result.insert(result.begin(), length, 0);
  return result;
}
}  // namespace

template <typename T>
class Grid {
 public:
  Grid(const std::vector<size_t>& shape, Wrapping wrapping,
       const std::vector<short>& neighborhood,
       std::function<void(T*, const std::vector<T*>&)> cellUpdate,
       T defaultValue, unsigned short int maxNeighborDistance = 1)
      // TODO: If neighborhood is const, should determine maxNeighborDistance
      // based on the neighborhood provided
      : shape(shape),
        size(multiplyAll<size_t>(shape)),
        maxNeighborDistance(maxNeighborDistance),
        padding(determinePadding(shape) * maxNeighborDistance),
        singleDimPadding(maxNeighborDistance * 2),
        numDimensions(shape.size()),
        wrapping(wrapping),
        neighborhood(neighborhood),
        cellUpdate(cellUpdate),
        defaultValue(defaultValue),
        defaultCellValue(std::unique_ptr<T>(new T(defaultValue))) {
    auto coordinate = allZeros(numDimensions);
    for (auto i = 0; i < size + padding; ++i) {
      if (isOutOfBounds(coordinate)) {
        cells.push_back(std::unique_ptr<Cell<T>>(new OutOfBoundsCell<T>()));
      } else {
        cells.push_back(std::make_unique<Cell<T>>(defaultValue));
      }
      incrementCoordinate(coordinate);
    }

    for (auto i = 0; i < size + padding; ++i) {
      auto cell = cells[i].get();
      if (cell->isOutOfBounds()) {
        auto oobCell = static_cast<OutOfBoundsCell<T>*>(cell);
        switch (wrapping) {
          case Wrapping::BOUNDED:
            oobCell->set(defaultCellValue.get());
            break;

          case Wrapping::TOROIDAL:
            throw NotImplementedException();
            break;
        }
      }
    }

    neighbors.resize(neighborhood.size());
  }

  void update() {
    incrementTime();
    for (auto i = 0; i < size + padding; ++i) {
      auto cell = getCell(i);
      if (!cell->isOutOfBounds()) {
        auto j = 0;
        for (auto offset : neighborhood) {
          auto nidx = i + offset;
          neighbors[j++] = getCell(nidx)->get();
        }
        auto futureCell = cell->getFuture();
        cellUpdate(futureCell, neighbors);
      }
    }
  }

  // TODO: Write an iterator for this class

  const T& getValue(const std::vector<size_t>& coordinates) {
    return getValue(getIdx(coordinates));
  }

  void setValue(const std::vector<size_t>& coordinates, const T& val) {
    auto idx = getIdx(coordinates);
    auto cell = getCell(idx);
    if (cell->isOutOfBounds()) {
      throw std::out_of_range(
          "Can't manually set value for out of bounds indices");
    }
    setValue(idx, val);
  }

  size_t getSize() { return size; }

 private:
  // Immutable member variables
  std::vector<size_t> const shape;
  size_t const size;
  size_t const maxNeighborDistance;
  size_t const singleDimPadding;
  size_t const padding;
  unsigned short int const numDimensions;
  std::vector<short> const neighborhood;
  Wrapping const wrapping;
  T const defaultValue;
  std::unique_ptr<T> const defaultCellValue;
  std::function<void(T*, const std::vector<T*>&)> const cellUpdate;

  // Mutable member variables
  std::vector<std::unique_ptr<Cell<T>>> cells;
  std::vector<T*> neighbors;

  // Private member functions
  const T& getValue(size_t idx) { return *(getCell(idx)->get()); }
  void setValue(size_t idx, const T& val) { getCell(idx)->set(val); }

  Cell<T>* getCell(size_t idx) { return cells[idx].get(); }

  void incrementTime() {
    for (auto i = 0; i < size + padding; ++i) {
      getCell(i)->incrementTime();
    }
  }

  size_t getRealDimSize(size_t idx) { return shape[idx] + singleDimPadding; }

  size_t getIdx(const std::vector<size_t>& coordinates) {
    if (coordinates.size() != numDimensions)
      throw InvalidOperationException(
          "Coordinate numDimensions do not match grid's numDimensions.");

    size_t result{0};
    for (auto i = 0; i < numDimensions; ++i) {
      auto chunk = coordinates[i] + maxNeighborDistance;
      for (auto j = 1; j <= i; ++j) {
        chunk *= getRealDimSize(j - 1);
      }
      result += chunk;
    }
    return result;
  }

  void incrementCoordinate(std::vector<size_t>& coordinate) {
    auto i = 0;
    do {
      coordinate[i] = (coordinate[i] + 1) % getRealDimSize(i);
    } while (coordinate[i] == 0 && ++i < numDimensions);
  }

  bool isOutOfBounds(const std::vector<size_t>& coordinate) {
    for (auto i = 0; i < numDimensions; ++i) {
      auto coord = coordinate[i];
      if (coord < maxNeighborDistance ||
          coord >= shape[i] + maxNeighborDistance) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace methuselah