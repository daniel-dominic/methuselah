#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "gridRenderer.h"
#include "methuselah.h"

using namespace methuselah;

constexpr unsigned int CELL_SIZE = 6;

constexpr bool USE_DELAY = true;
constexpr unsigned int DELAY = 50;

constexpr unsigned short int GRID_WIDTH = 200;
constexpr unsigned short int GRID_HEIGHT = 100;

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
  srand(time(nullptr));
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

    GridRenderer2D<bool> renderer{grid, CELL_SIZE, CELL_SIZE, WINDOW_WIDTH,
                                  WINDOW_HEIGHT};

    auto running = true;
    while (running) {
      SDL_Event e;
      while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
          case SDL_QUIT:
            running = false;
            break;
          case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) {
              running = false;
            }
            if (e.key.keysym.sym == SDLK_r) {
              randomize(*grid);
            }
            break;
        }
      }

      grid->update();
      renderer.render();
    }
  }

  SDL_Quit();

  return 0;
}