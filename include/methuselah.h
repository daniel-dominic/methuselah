#pragma once

#include <memory>

namespace methuselah {

template<typename T, class... Args>
class Cell {
 public:
  Cell(std::unique_ptr<T> value) : value(std::move(value)) {}
  Cell() : value(std::make_unique<T>()) {}

  T* getValue() { return value.get(); }

 private:
  std::unique_ptr<T> value;
};

template <typename T>
class Grid {

};

}  // namespace mthus