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

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 500;

constexpr unsigned short int SCALE = 1;

constexpr unsigned short int ORIGIN_X = 40;
constexpr unsigned short int ORIGIN_Y = 0;

constexpr unsigned short int GRID_WIDTH = 80;
constexpr unsigned short int GRID_HEIGHT = 80;
constexpr unsigned short int GRID_DEPTH = 3;

constexpr unsigned short int WINDOW_WIDTH = 1000;
constexpr unsigned short int WINDOW_HEIGHT = 800;

void lifeUpdate(bool* cell, std::vector<bool*> neighbors) {
  auto numNeighbors = 0;
  for (const auto& neighbor : neighbors) {
    numNeighbors += *neighbor;
  }
  auto alive = *cell;
  if (alive && (numNeighbors == 0 || numNeighbors > 2)) {
    *cell = false;
  }
  else if (!alive && (numNeighbors == 2 || numNeighbors == 4)) {
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
SDL_Rect mapper(const bool& alive, const std::vector<size_t>& coord) {
  if (alive) {
    return {(int)(CELL_WIDTH * (2 + coord[2])), 0, CELL_WIDTH, CELL_HEIGHT};
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
    grid->setValue({1,1,0}, true);
    grid->setValue({2,0,0}, true);

    IsometricSpriteRenderer<bool> renderer{
        grid,        mapper,       "data/isometric.png", CELL_WIDTH,
        CELL_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT,        ORIGIN_X,
        ORIGIN_Y,    SCALE};

    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });
    
    auto paused = true;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto oneStep = true;
    eventHandler.registerKeyDownAction(SDLK_SPACE, [&]() { oneStep = true; });

    eventHandler.registerKeyDownAction(SDLK_UP, [&]() { renderer.incrementRenderDepth(); });
    eventHandler.registerKeyDownAction(SDLK_DOWN, [&]() { renderer.decrementRenderDepth(); });

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