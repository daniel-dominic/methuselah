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

using Color = std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>;

constexpr unsigned int CELL_SIZE = 6;

constexpr bool USE_DELAY = false;
constexpr unsigned int DELAY = 20;

constexpr unsigned short int GRID_WIDTH = 200;
constexpr unsigned short int GRID_HEIGHT = 150;

constexpr unsigned short int WINDOW_WIDTH = GRID_WIDTH * CELL_SIZE;
constexpr unsigned short int WINDOW_HEIGHT = GRID_HEIGHT * CELL_SIZE;

constexpr uint8_t WATER_MAX = 255;

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

// pos in range [0, 1]
//
// Thanks to:
// https://stackoverflow.com/questions/5960979/using-c-vectorinsert-to-add-to-end-of-vector
Color gradient(double pos) {
    //we want to normalize ratio so that it fits in to 6 regions
    //where each region is 256 units long
    int normalized = int(pos * 256 * 6);

    //find the distance to the start of the closest region
    uint8_t x = normalized % 256;

    uint8_t red = 0, grn = 0, blu = 0;
    switch(normalized / 256) {
    case 0: red = 255;      grn = x;        blu = 0;       break;//red
    case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
    case 2: red = 0;        grn = 255;      blu = x;       break;//green
    case 3: red = 0;        grn = 255 - x;  blu = 255;     break;//cyan
    case 4: red = x;        grn = 0;        blu = 255;     break;//blue
    case 5: red = 255;      grn = 0;        blu = 255 - x; break;//magenta
    }

    return {red, grn, blu, 255};
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

    if (sum > 2 && cell->water < WATER_MAX) {
      ++cell->water;
    }
    else if (!sum && cell->water) {
      --cell->water;
    }
  }

}

Color colorize(const Cell& cell) {
  //return gradient(cell.water / (double)(WATER_MAX));
  if (!cell.passable)
    return {100,150,100,255};
  auto x = (uint8_t)(255 - (cell.water / (double)(WATER_MAX))*255);
  return {x,x,x,255};
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
      if (rand() % 100 < 30) {
        grid.setValue(coord, Cell{0, false});
      }
      else {
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