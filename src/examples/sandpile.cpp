#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using methuselah::EventHandler;
using methuselah::Grid;
using methuselah::Neighborhood;
using methuselah::Ortho2DColorRenderer;
using methuselah::Wrapping;

constexpr unsigned int CELL_SIZE = 10;

constexpr bool USE_DELAY = true;
constexpr unsigned int DELAY = 50;

constexpr unsigned short int GRID_WIDTH = 60;
constexpr unsigned short int GRID_HEIGHT = 80;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

// Fluid Flow
// ==========
struct Cell {
  bool sand;
  bool passable;
};

void update(Cell* cell, std::vector<Cell*> neighbors) {
  if (!cell->sand &&
      (neighbors[0]->sand || neighbors[1]->sand || neighbors[2]->sand)) {
    cell->sand = true;
  } else if (cell->sand && ((!neighbors[5]->sand && neighbors[5]->passable) ||
                            (!neighbors[6]->sand && neighbors[6]->passable) ||
                            (!neighbors[7]->sand && neighbors[7]->passable))) {
    cell->sand = false;
  }
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> colorize(const Cell& cell) {
  uint8_t r{50}, g{50}, b{150};
  if (cell.sand) {
    r = 255;
    g = 255;
    b = 0;
  }
  return {r, g, b, 255};
}

// Randomize
// =========

double randUnitInterval() { return (double)(rand()) / (double)(RAND_MAX); }

void randomize(Grid<Cell>& grid, uint8_t mod = 4) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      grid.setValue(coord, Cell{(bool)(rand() % mod == 0), true});
    }
  }
}

// Main Function
// =============

int main() {
  {
    auto grid =
        std::shared_ptr<Grid<Cell>>(new Grid<Cell>{{GRID_WIDTH, GRID_HEIGHT},
                                                   Wrapping::BOUNDED,
                                                   Neighborhood::MOORE,
                                                   update,
                                                   Cell{false, false}});
    randomize(*grid);

    Ortho2DColorRenderer<Cell> renderer{grid,      colorize,     CELL_SIZE,
                                        CELL_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto paused = false;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto oneStep = false;
    eventHandler.registerKeyDownAction(SDLK_SPACE, [&]() { oneStep = true; });

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