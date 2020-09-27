#pragma once

#include <assert.h>

#include <algorithm>
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
enum Wrapping { BOUNDED, TOROIDAL };
enum Neighborhood { MOORE, VON_NEUMANN, CUSTOM };

namespace {  // Helper functions
template <typename T>
T multiplyAll(const std::vector<T>& vec) {
  return std::accumulate(vec.begin(), vec.end(), 1, std::multiplies<T>());
}

std::vector<size_t> allZeros(size_t length) {
  auto result = std::vector<size_t>();
  result.insert(result.begin(), length, 0);
  return result;
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

std::vector<std::vector<int>> generateMooreOffsets(size_t numDimensions,
                                              bool isLastDim = true) {
  if (numDimensions == 0) {
    return {};
  }
  auto head = std::vector<int>{-1, 0, 1};
  auto tails = generateMooreOffsets(numDimensions - 1, false);
  auto offsets = std::vector<std::vector<int>>();
  for (auto x : head) {
    if (tails.empty()) {
      offsets.push_back(std::vector<int>{x});
      continue;
    }
    for (const auto& tail : tails) {
      std::vector<int> result{x};
      result.insert(result.end(), tail.begin(), tail.end());
      if (isLastDim && std::all_of(result.begin(), result.end(),
                                   [](int x) { return x == 0; })) {
        continue;
      }
      offsets.push_back(result);
    }
  }
  return offsets;
}

// TODO: Von Neumann offsets
// NOTE: I think you can just take the Moore offsets, and remove all of
//       those whose sum of absolute values is greater than 1.

}  // namespace

template <typename T>
class Grid {
 public:
  Grid(const std::vector<size_t>& shape, Wrapping wrapping,
       Neighborhood neighborhood,
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
        cellUpdate(cellUpdate),
        defaultValue(defaultValue),
        defaultCellValue(std::unique_ptr<T>(new T(defaultValue))) {
    setNeighborhood(neighborhoodType);

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

  const std::vector<size_t>& getShape() const { return shape; }

  size_t getSize() const { return size; }

  void setNeighborhood(Neighborhood neighborhoodType) {
    if (neighborhoodType == Neighborhood::CUSTOM)
      throw InvalidOperationException(
          "To set custom neighborhood, provide offsets directly");

    this->neighborhoodType = neighborhoodType;
    switch (neighborhoodType) {
      case Neighborhood::MOORE:
        neighborhood = generateMoore(numDimensions);
        break;
      case Neighborhood::VON_NEUMANN:
        throw NotImplementedException();
        break;
    }
    neighbors.resize(neighborhood.size());
  }

  void setNeighborhood(std::vector<std::vector<int>> offsets) {
    neighborhood.clear();
    for (const auto& offset : offsets) {
      neighborhood.push_back(getIdx(offset), false);
    }
    neighbors.resize(neighborhood.size());
  }

 private:
  // Immutable member variables
  std::vector<size_t> const shape;
  size_t const size;
  size_t const maxNeighborDistance;
  size_t const singleDimPadding;
  size_t const padding;
  unsigned short int const numDimensions;
  Wrapping const wrapping;
  T const defaultValue;
  std::unique_ptr<T> const defaultCellValue;

  // Mutable member variables
  std::vector<std::unique_ptr<Cell<T>>> cells;
  std::function<void(T*, const std::vector<T*>&)> cellUpdate;
  Neighborhood neighborhoodType;
  std::vector<int> neighborhood;
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

  long int getOffsetIdx(const std::vector<int>& offsetCoords) {
    if (offsetCoords.size() != numDimensions)
      throw InvalidOperationException(
          "Coordinate numDimensions do not match grid's numDimensions.");

    long int result{0};
    for (auto i = 0; i < numDimensions; ++i) {
      auto chunk = offsetCoords[i];
      for (auto j = 1; j <= i; ++j) {
        chunk *= getRealDimSize(j - 1);
      }
      result += chunk;
    }
    return result;
  }

  std::vector<int> generateMoore(size_t numDimensions) {
    auto offsets = generateMooreOffsets(numDimensions);
    std::vector<int> neighborhood;
    for (const auto& coord : offsets) {
      neighborhood.push_back(getOffsetIdx(coord));
    }
    return neighborhood;
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