#include <SDL2/SDL.h>

#include "methuselah.h"

using namespace methuselah;

constexpr unsigned int CELL_SIZE = 10;
constexpr unsigned int GENERATIONS = 1000;

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 500;

constexpr unsigned short int WINDOW_WIDTH = 100 * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = 100 * CELL_SIZE;

constexpr unsigned short int GRID_WIDTH = WINDOW_WIDTH / CELL_SIZE;
constexpr unsigned short int GRID_HEIGHT = WINDOW_HEIGHT / CELL_SIZE;

int main() {
  // Game of life specific ---------------------------------
  auto lifeUpdate = [](bool* cell, std::vector<bool*> neighbors) {
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
  };
  Grid<bool> grid{
    {GRID_WIDTH, GRID_HEIGHT},
    Wrapping::BOUNDED,
    {-GRID_WIDTH - 1 - 2, -GRID_WIDTH - 2, -GRID_WIDTH + 1 - 2,
                      -1,                                    1,                  
      GRID_WIDTH - 1 + 2,  GRID_WIDTH + 2,  GRID_WIDTH + 1 + 2},
    lifeUpdate,
    false
  };
  grid.setValue({21,20}, true);
  grid.setValue({20,20}, true);
  grid.setValue({19,21}, true);
  grid.setValue({20,21}, true);
  grid.setValue({20,22}, true);
  
  // -------------------------------------------------------

  auto window =
      SDL_CreateWindow("Game Of Life", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Rect rect{0, 0, CELL_SIZE, CELL_SIZE};
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GENERATIONS; ++i) {
    rect.x = 0;
    rect.y = 0;

    for (auto i = 0; i < GRID_HEIGHT; ++i) {
      rect.y = i * CELL_SIZE;
      coord[1] = i;
      for (auto j = 0; j < GRID_WIDTH; ++j) {
        rect.x = j * CELL_SIZE;
        coord[0] = j;
        
        auto value = grid.getValue(coord);
        if (value) {
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        } else {
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        SDL_RenderFillRect(renderer, &rect);
      }
    }
    grid.update();

    SDL_RenderPresent(renderer);
    if (USE_DELAY) {
      SDL_Delay(DELAY);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}