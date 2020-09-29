#pragma once

#include <SDL2/SDL.h>

#include <cstdint>
#include <functional>
#include <memory>

#include "methuselah.h"

namespace methuselah {

template <typename T>
class GridRenderer {
 public:
  GridRenderer(std::shared_ptr<Grid<T>> grid, uint16_t cellWidth,
               uint16_t cellHeight, uint16_t windowWidth, uint16_t windowHeight)
      : grid(grid),
        cellWidth(cellWidth),
        cellHeight(cellHeight),
        windowWidth(windowWidth),
        windowHeight(windowHeight),
        window(nullptr, SDL_DestroyWindow),
        renderer(nullptr, SDL_DestroyRenderer) {
    rect = SDL_Rect{0, 0, cellWidth, cellHeight};
    window.reset(SDL_CreateWindow("Methuselah", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, windowWidth,
                                  windowHeight, 0));
    renderer.reset(
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
  }

  virtual void render() = 0;

 protected:
  std::shared_ptr<Grid<T>> grid;
  uint16_t const cellWidth;
  uint16_t const cellHeight;
  uint16_t const windowWidth;
  uint16_t const windowHeight;
  SDL_Rect rect;
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
  std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer;
};

template <typename T>
class Ortho2DColorRenderer : public GridRenderer<T> {
 public:
  Ortho2DColorRenderer(
      std::shared_ptr<Grid<T>> grid,
      std::function<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>(const T&)>
          colorize,
      uint16_t cellWidth, uint16_t cellHeight, uint16_t windowWidth,
      uint16_t windowHeight)
      : colorize(colorize),
        GridRenderer<T>(grid, cellWidth, cellHeight, windowWidth,
                        windowHeight) {
    auto shape = grid->getShape();
    gridWidth = shape[0];
    gridHeight = shape[1];
    coord = std::vector<size_t>{0, 0};
  }

  void render() {
    rect.x = 0;
    rect.y = 0;

    for (auto i = 0; i < gridHeight; ++i) {
      rect.y = i * cellHeight;
      coord[1] = i;
      for (auto j = 0; j < gridWidth; ++j) {
        rect.x = j * cellWidth;
        coord[0] = j;

        auto value = grid->getValue(coord);
        auto color = colorize(value);
        auto r = std::get<0>(color);
        auto g = std::get<1>(color);
        auto b = std::get<2>(color);
        auto a = std::get<3>(color);
        SDL_SetRenderDrawColor(renderer.get(), r, g, b, a);
        SDL_RenderFillRect(renderer.get(), &rect);
      }
    }

    SDL_RenderPresent(renderer.get());
  }

  using GridRenderer<T>::grid;
  using GridRenderer<T>::rect;
  using GridRenderer<T>::renderer;
  using GridRenderer<T>::cellWidth;
  using GridRenderer<T>::cellHeight;

 private:
  std::function<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>(const T&)>
      colorize;
  std::vector<size_t> coord;
  uint16_t gridWidth;
  uint16_t gridHeight;
};
}  // namespace methuselah
