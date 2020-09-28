#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using namespace methuselah;

constexpr unsigned int CELL_SIZE = 10;

constexpr bool USE_DELAY = true;
constexpr unsigned int DELAY = 100;

constexpr unsigned short int GRID_WIDTH = 20;
constexpr unsigned short int GRID_HEIGHT = 20;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

void lifeUpdate(bool* cell, std::vector<bool*> neighbors) {
  auto numNeighbors = 0;
  for (const auto& neighbor : neighbors) {
    numNeighbors += *neighbor;
  }
  auto alive = *cell;
  if (alive && (numNeighbors < 2 || numNeighbors > 3)) {
    *cell = false;
  } else if (!alive && numNeighbors == 3) {
    *cell = true;
  }
}

void randomize(Grid<bool>& grid, unsigned short mod = 2) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      grid.setValue(coord, rand() % mod == 0);
    }
  }
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> colorize(const bool& alive) {
  if (alive) {
    return {255, 255, 255, 255};
  } else {
    return {0, 0, 0, 255};
  }
}

int main() {
  {
    auto grid =
        std::shared_ptr<Grid<bool>>(new Grid<bool>{{GRID_WIDTH, GRID_HEIGHT},
                                                   Wrapping::TOROIDAL,
                                                   Neighborhood::MOORE,
                                                   lifeUpdate});
    randomize(*grid);

    GridRenderer2D<bool> renderer{grid,      colorize,     CELL_SIZE,
                                  CELL_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto running = true;
    while (running) {
      eventHandler.handleAll();
      grid->update();
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