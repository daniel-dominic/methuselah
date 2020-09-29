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
constexpr unsigned int DELAY = 500;

constexpr unsigned short int SCALE = 3;

constexpr unsigned short int ORIGIN_X = 10;
constexpr unsigned short int ORIGIN_Y = 4;

constexpr unsigned short int GRID_WIDTH = 20;
constexpr unsigned short int GRID_HEIGHT = 20;
constexpr unsigned short int GRID_DEPTH = 3;

constexpr unsigned short int WINDOW_WIDTH = 1000;
constexpr unsigned short int WINDOW_HEIGHT = 800;

void lifeUpdate(bool* cell, std::vector<bool*> neighbors) {
  auto numNeighbors = 0;
  for (const auto& neighbor : neighbors) {
    numNeighbors += *neighbor;
  }
  auto alive = *cell;
  // if (alive && numNeighbors == 3) {
  //   *cell = false;
  // }
  if (!alive && numNeighbors == 2) {
    *cell = true;
  }
}

void randomize(Grid<bool>& grid, unsigned short mod = 2) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0, 0};
  for (auto i = 0; i < GRID_DEPTH; ++i) {
    coord[2] = i;
    for (auto j = 0; j < GRID_HEIGHT; ++j) {
      coord[1] = j;
      for (auto k = 0; k < GRID_WIDTH; ++k) {
        coord[0] = k;

        grid.setValue(coord, rand() % mod == 0);
      }
    }
  }
}

// TODO: Adjust Renderer so that this function receives coordinates
SDL_Rect mapper(const bool& alive) {
  if (alive) {
    return {CELL_WIDTH * 2, 0, CELL_WIDTH, CELL_HEIGHT};
  } else {
    return {0, 0, CELL_WIDTH, CELL_HEIGHT};
  }
}

int main() {
  {
    auto grid = std::shared_ptr<Grid<bool>>(
        new Grid<bool>{{GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH},
                       Wrapping::BOUNDED,
                       Neighborhood::MOORE,
                       lifeUpdate});
    //randomize(*grid);

    grid->setValue({0,0,0}, true);
    grid->setValue({1,0,0}, true);
    // grid->setValue({2,3,0}, true);
    // grid->setValue({3,3,0}, true);

    // grid->setValue({0,2,1}, true);
    // grid->setValue({1,2,1}, true);
    // grid->setValue({2,2,1}, true);
    // grid->setValue({3,2,1}, true);

    // grid->setValue({0,1,2}, true);
    // grid->setValue({1,1,2}, true);
    // grid->setValue({2,1,2}, true);
    // grid->setValue({3,1,2}, true);

    IsometricSpriteRenderer<bool> renderer{
        grid,        mapper,       "data/isometric.png", CELL_WIDTH,
        CELL_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT,        ORIGIN_X,
        ORIGIN_Y,    SCALE};

    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });
    
    auto paused = false;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto z = 3;
    eventHandler.registerKeyDownAction(SDLK_UP, [&]() { renderer.incrementRenderDepth(); });
    eventHandler.registerKeyDownAction(SDLK_DOWN, [&]() { renderer.decrementRenderDepth(); });

    auto running = true;
    while (running) {
      eventHandler.handleAll();
      if (!paused) {
        grid->update();
      }
      renderer.render();
      running = !eventHandler.receivedQuitSignal();
      if (USE_DELAY) {
        SDL_Delay(DELAY);
      }
    }
  }

  SDL_Quit();

  return 0;
}