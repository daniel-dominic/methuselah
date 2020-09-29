#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

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

template <typename T>
class IsometricSpriteRenderer : public GridRenderer<T> {
 public:
  IsometricSpriteRenderer(std::shared_ptr<Grid<T>> grid,
                          std::function<SDL_Rect(const T&)> mapper,
                          std::string spritesheetPath, uint16_t cellWidth,
                          uint16_t cellHeight, uint16_t windowWidth,
                          uint16_t windowHeight, int originX, int originY,
                          uint16_t scale)
      : mapper(mapper),
        spritesheet(nullptr, SDL_DestroyTexture),
        originX(originX),
        originY(originY),
        scale(scale),
        GridRenderer<T>(grid, cellWidth, cellHeight, windowWidth,
                        windowHeight) {
    auto shape = grid->getShape();
    gridWidth = shape[0];
    gridHeight = shape[1];
    gridDepth = shape[2];

    renderDepth = gridDepth;

    auto surface = IMG_Load(spritesheetPath.c_str());
    if (surface == nullptr) {
      throw -1;
    }
    spritesheet.reset(SDL_CreateTextureFromSurface(renderer.get(), surface));
    SDL_FreeSurface(surface);
  }

  void render() {
    SDL_RenderClear(renderer.get());

    std::vector<size_t> coord{0, 0, 0};
    for (int z = 0; z < renderDepth; ++z) {
      coord[2] = z;
      for (int y = 0; y < gridHeight; ++y) {
        coord[1] = y;
        for (int x = 0; x < gridWidth; ++x) {
          coord[0] = x;
          auto dest = toDestRect(x, y, z);
          auto src = mapper(grid->getValue(coord));
          if (src.x != 0) {
            src.x += 16 * z;
          }
          SDL_RenderCopy(renderer.get(), spritesheet.get(), &src, &dest);
        }
      }
    }
    SDL_RenderPresent(renderer.get());
  }

  using GridRenderer<T>::grid;
  using GridRenderer<T>::rect;
  using GridRenderer<T>::renderer;
  using GridRenderer<T>::cellWidth;
  using GridRenderer<T>::cellHeight;

  void incrementRenderDepth() {
    if (renderDepth >= gridDepth - 1) {
      renderDepth = gridDepth;
    } else {
      renderDepth += 1;
    }
  }

  void decrementRenderDepth() {
    if (renderDepth <= 1) {
      renderDepth = 0;
    } else {
      renderDepth -= 1;
    }
  }

 private:
  SDL_Rect toDestRect(int x, int y, int z) {
    return {((originX * cellWidth) + (x - y) * (cellWidth / 2)) * scale,
            ((originY * cellHeight) + (x + y) * (cellHeight / 4) -
             (z * (cellHeight / 2))) *
                scale,
            cellWidth * scale, cellHeight * scale};
  }

  std::function<SDL_Rect(const T&)> mapper;
  uint16_t scale;
  uint16_t gridWidth;
  uint16_t gridHeight;
  uint16_t gridDepth;
  uint16_t renderDepth;
  int originX;
  int originY;
  std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> spritesheet;
};

}  // namespace methuselah
