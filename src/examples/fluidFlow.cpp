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

constexpr unsigned int CELL_SIZE = 4;

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 20;

constexpr unsigned short int GRID_WIDTH = 400;
constexpr unsigned short int GRID_HEIGHT = 200;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

constexpr uint8_t WATER_MAX = 15;

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
  for (auto i = 0; i < 3 && cell->water < WATER_MAX; ++i) {
    if (neighbors[i]->water > cell->water) {
      cell->water += 1;
      return;
    }
  }

  for (auto i = 5; i < 8; ++i) {
    if (neighbors[i]->passable && neighbors[i]->water < cell->water &&
        neighbors[i]->water < WATER_MAX) {
      cell->water -= 1;
      return;
    }
  }

  // if (neighbors[1]->water > cell->water) {
  //   cell->water += 1;
  // } else if (neighbors[6]->water < cell->water && neighbors[6]->passable) {
  //   cell->water -= 1;
  // }
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> colorize(const Cell& cell) {
  uint8_t r{0}, g{0}, b{0};
  if (cell.water) {
    auto amt = 255 * (cell.water / (double)(WATER_MAX));
    r = amt;
    g = amt;
    b = amt;
  }
  return {r, g, b, 255};
}

// Randomize
// =========

double randUnitInterval() { return (double)(rand()) / (double)(RAND_MAX); }

void randomize(Grid<Cell>& grid, uint8_t mod = WATER_MAX) {
  srand(time(0)*100);
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      grid.setValue(coord, Cell{(uint8_t)(rand() % mod), true});
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
                                                   Cell{0, false}});
    randomize(*grid);

    Ortho2DColorRenderer<Cell> renderer{grid,      colorize,     CELL_SIZE,
                                        CELL_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto paused = false;
    eventHandler.registerKeyDownAction(SDLK_p, [&]() { paused ^= true; });

    auto oneStep = false;
    eventHandler.registerKeyDownAction(SDLK_SPACE, [&]() { oneStep = true; });

    eventHandler.registerMouseClickAction(
        [&](int32_t x, int32_t y) { oneStep = true; });

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