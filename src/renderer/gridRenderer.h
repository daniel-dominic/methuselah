#pragma once

#include <SDL2/SDL.h>

#include <cstdint>
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
class GridRenderer2D : public GridRenderer<T> {
 public:
  GridRenderer2D(std::shared_ptr<Grid<T>> grid, uint16_t cellWidth,
                 uint16_t cellHeight, uint16_t windowWidth,
                 uint16_t windowHeight)
      : GridRenderer<T>(grid, cellWidth, cellHeight, windowWidth,
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
        if (value) {
          SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);
        } else {
          SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
        }
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
  std::vector<size_t> coord;
  uint16_t gridWidth;
  uint16_t gridHeight;
};
}  // namespace methuselah
