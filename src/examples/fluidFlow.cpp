#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

#include "color.h"
#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using methuselah::EventHandler;
using methuselah::Grid;
using methuselah::Neighborhood;
using methuselah::Ortho2DColorRenderer;
using methuselah::Wrapping;

constexpr unsigned int CELL_SIZE = 20;

constexpr bool USE_DELAY = true;
constexpr unsigned int DELAY = 100;

constexpr unsigned short int GRID_WIDTH = 30;
constexpr unsigned short int GRID_HEIGHT = 30;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

constexpr uint8_t WATER_MAX = 7;

// Helper Functions
// ================
double cosineSimilarity(std::vector<double> vectorA,
                        std::vector<double> vectorB) {
  double dotProduct = 0.0;
  double normA = 0.0;
  double normB = 0.0;
  for (int i = 0; i < vectorA.size(); ++i) {
    dotProduct += vectorA[i] * vectorB[i];
    normA += std::pow(vectorA[i], 2);
    normB += std::pow(vectorB[i], 2);
  }
  if (normA == 0 || normB == 0) {
    return 0;
  }
  return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
}

// Fluid Flow
// ==========
constexpr uint8_t NUM_NEIGHBORS = 8;

struct Cell {
  uint8_t water;
  bool passable;
};

void update(Cell* cell, std::vector<Cell*> neighbors) {
  auto sum = 0;
  if (cell->passable) {
    for (auto i = 0; i < 8 && cell->water < WATER_MAX; ++i) {
      if (neighbors[i]->passable && neighbors[i]->water > cell->water) {
        ++sum;
      }
    }

    if (sum && cell->water < WATER_MAX) {
      ++cell->water;
    } else if (!sum && cell->water) {
      --cell->water;
    }
  }
}

Color colorize(const Cell& cell) {
  // return gradient(cell.water / (double)(WATER_MAX));
  if (!cell.passable) return {100, 255, 100, 255};
  auto x = (uint8_t)(255 - (cell.water / (double)(WATER_MAX)) * 255);
  return {x, x, 255, 255};
}

// Randomize
// =========

double randUnitInterval() { return (double)(rand()) / (double)(RAND_MAX); }

void randomize(Grid<Cell>& grid, uint8_t mod = WATER_MAX,
               uint8_t immovableAmt = 0) {
  srand(time(0) * 100);
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      if (rand() % 100 < immovableAmt) {
        grid.setValue(coord, Cell{0, false});
      } else {
        grid.setValue(coord, Cell{(uint8_t)(rand() % mod), true});
      }
    }
  }
}

// Main Function
// =============

int main() {
  {
    auto grid =
        std::shared_ptr<Grid<Cell>>(new Grid<Cell>{{GRID_WIDTH, GRID_HEIGHT},
                                                   Wrapping::TOROIDAL,
                                                   Neighborhood::MOORE,
                                                   update,
                                                   Cell{0, false}});
    randomize(*grid);

    Ortho2DColorRenderer<Cell> renderer{grid,      colorize,     CELL_SIZE,
                                        CELL_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto paused = true;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto oneStep = false;
    eventHandler.registerKeyDownAction(SDLK_SPACE, [&]() { oneStep = true; });

    eventHandler.registerMouseClickAction([&](int32_t x, int32_t y) {
      size_t cellX = x / CELL_SIZE;
      size_t cellY = y / CELL_SIZE;
      bool passable = grid->getValue({cellX, cellY}).passable ^ true;
      grid->setValue({cellX, cellY}, Cell{0, passable});
    });

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