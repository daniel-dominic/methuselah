#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>

#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using namespace methuselah;

constexpr unsigned int CELL_WIDTH = 16;
constexpr unsigned int CELL_HEIGHT = 16;

constexpr bool USE_DELAY = true;
constexpr unsigned int DELAY = 100;

constexpr unsigned short int SCALE = 2;

constexpr unsigned short int GRID_WIDTH = 30;
constexpr unsigned short int GRID_HEIGHT = 30;
constexpr unsigned short int GRID_DEPTH = 9;

constexpr int ORIGIN_X = GRID_WIDTH / 2;
constexpr int ORIGIN_Y = (GRID_DEPTH - 1) / 2;

constexpr unsigned short int WINDOW_WIDTH =
    ((GRID_WIDTH + 1) * CELL_WIDTH) * SCALE;
constexpr unsigned short int WINDOW_HEIGHT =
    (((GRID_HEIGHT / 2) + (GRID_DEPTH - 1)) * CELL_HEIGHT) * SCALE;

void lifeUpdate(bool* cell, std::vector<bool*> neighbors) {
  auto numNeighbors = 0;
  for (const auto& neighbor : neighbors) {
    numNeighbors += *neighbor;
  }
  auto alive = *cell;
  if (alive && !(numNeighbors == 5 || numNeighbors == 7)) {
    *cell = false;
  } else if (!alive && (numNeighbors == 6)) {
    *cell = true;
  }
}

void randomize(Grid<bool>& grid, unsigned short mod = 12) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0, 0};
  for (auto i = 0; i < GRID_DEPTH; ++i) {
    coord[2] = i;
    for (auto j = 0; j < GRID_HEIGHT; ++j) {
      coord[1] = j;
      for (auto k = 0; k < GRID_WIDTH; ++k) {
        coord[0] = k;

        auto v = rand() % mod == 0;
        if (v) {
          grid.setValue(coord, v);
        }
      }
    }
  }
}

SDL_Rect mapper(const bool& alive, const std::vector<size_t>& coord) {
  if (alive) {
    return {2 * CELL_WIDTH, 0, CELL_WIDTH, CELL_HEIGHT};
  } else {
    return {0, 0, CELL_WIDTH, CELL_HEIGHT};
  }
}

void drawGlider_S56B2(std::shared_ptr<Grid<bool>> grid, size_t x, size_t y,
                      size_t z) {
  grid->setValue({x + 0, y + 0, z + 0}, true);
  grid->setValue({x + 1, y + 0, z + 0}, true);

  grid->setValue({x + 0, y + 1, z + 0}, true);
  grid->setValue({x + 1, y + 1, z + 0}, true);

  grid->setValue({x + 0, y + 2, z + 0}, true);
  grid->setValue({x + 1, y + 2, z + 0}, true);

  grid->setValue({x + 0, y + 2, z - 1}, true);
  grid->setValue({x + 1, y + 2, z - 1}, true);

  grid->setValue({x + 0, y + 1, z - 2}, true);
  grid->setValue({x + 1, y + 1, z - 2}, true);
}

int main() {
  {
    auto grid = std::shared_ptr<Grid<bool>>(
        new Grid<bool>{{GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH},
                       Wrapping::TOROIDAL,
                       Neighborhood::MOORE,
                       lifeUpdate});

    drawGlider_S56B2(grid, 4, 1, 3);
    drawGlider_S56B2(grid, 8, 5, 3);

    IsometricSpriteRenderer<bool> renderer{
        grid,        mapper,       "data/isometric.png", CELL_WIDTH,
        CELL_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT,        ORIGIN_X,
        ORIGIN_Y,    SCALE};

    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    srand(time(0));
    eventHandler.registerKeyDownAction(SDLK_g, [&]() {
      try {
        drawGlider_S56B2(grid, rand() % GRID_WIDTH, rand() % GRID_HEIGHT,
                         rand() % GRID_DEPTH);
      } catch (std::out_of_range e) {
        std::cout << "oops\n";
      }
    });

    auto paused = true;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto oneStep = true;
    eventHandler.registerKeyDownAction(SDLK_SPACE, [&]() { oneStep = true; });

    eventHandler.registerKeyDownAction(
        SDLK_UP, [&]() { renderer.incrementRenderDepth(); });
    eventHandler.registerKeyDownAction(
        SDLK_DOWN, [&]() { renderer.decrementRenderDepth(); });

    auto running = true;
    while (running) {
      eventHandler.handleAll();
      if (!paused || oneStep) {
        grid->update();
      }
      renderer.render();
      running = !eventHandler.receivedQuitSignal();
      if (USE_DELAY) {
        SDL_Delay(DELAY);
      }
      oneStep = false;
    }
  }

  SDL_Quit();

  return 0;
}