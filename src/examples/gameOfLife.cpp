#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using namespace methuselah;

constexpr unsigned int CELL_SIZE = 2;

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 50;

constexpr unsigned short int GRID_WIDTH = 600;
constexpr unsigned short int GRID_HEIGHT = 400;

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

void randomize(Grid<bool>& grid, unsigned short mod = 3) {
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

int main() {
  {
    auto grid = std::shared_ptr<Grid<bool>>(new Grid<bool>{
        {GRID_WIDTH, GRID_HEIGHT},
        Wrapping::BOUNDED,
        {-GRID_WIDTH - 1 - 2, -GRID_WIDTH - 2, -GRID_WIDTH + 1 - 2, -1, 1,
         GRID_WIDTH - 1 + 2, GRID_WIDTH + 2, GRID_WIDTH + 1 + 2},
        lifeUpdate,
        false});
    randomize(*grid);

    GridRenderer2D<bool> renderer{grid, CELL_SIZE, CELL_SIZE, WINDOW_WIDTH,
                                  WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto running = true;
    while (running) {
      eventHandler.handleAll();
      grid->update();
      renderer.render();
      running = !eventHandler.receivedQuitSignal();
    }
  }

  SDL_Quit();

  return 0;
}