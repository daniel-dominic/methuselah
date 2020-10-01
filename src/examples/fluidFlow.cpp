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

constexpr unsigned int CELL_SIZE = 12;

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 500;

constexpr unsigned short int GRID_WIDTH = 50;
constexpr unsigned short int GRID_HEIGHT = 50;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

constexpr double BETA1 = 2.0;
constexpr double BETA2 = BETA1;
constexpr double GAMMA1 = 1.0;
constexpr double GAMMA2 = GAMMA1;

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
  bool water;
};

void update(Cell* cell, std::vector<Cell*> neighbors) {
  if (!cell->water && neighbors[3]->water) {
    cell->water = true;
  }
  else if(cell->water && !neighbors[3]->water) {
    cell->water = false;
  }

  // Loss
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> colorize(const Cell& cell) {
  uint8_t r{0},g{0},b{0};
  if (cell.water) {
    r,g,b = 255,255,255;
  }
  return {r, g, b, 255};
}

// Randomize
// =========

double randUnitInterval() { return (double)(rand()) / (double)(RAND_MAX); }

bool randBool() { return rand() % 2 == 0; }

void randomize(Grid<Cell>& grid) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      grid.setValue(coord, Cell{(bool)(rand() % 2), (bool)(rand() % 2) });
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
                                                   update});
    randomize(*grid);

    Ortho2DColorRenderer<Cell> renderer{grid,      colorize,     CELL_SIZE,
                                        CELL_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT};
    EventHandler eventHandler;
    eventHandler.registerKeyDownAction(SDLK_r, [&]() { randomize(*grid); });

    auto paused = true;
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