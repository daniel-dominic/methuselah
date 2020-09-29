#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

#include "eventHandler.h"
#include "gridRenderer.h"
#include "methuselah.h"

using namespace methuselah;

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

struct FluidCell {
  FluidCell(double xVelocity, double yVelocity, uint8_t fluid)
      : xVelocity(xVelocity), yVelocity(yVelocity), fluid(fluid) {
    output.insert(output.begin(), 8, 0);
  }
  double xVelocity;
  double yVelocity;
  uint8_t fluid;
  std::vector<uint8_t> output;
};

constexpr uint8_t NUM_NEIGHBORS = 8;

void update(FluidCell* cell, std::vector<FluidCell*> neighbors) {
  std::fill(cell->output.begin(), cell->output.end(), 0);

  auto fluid = cell->fluid;
  auto xv = cell->xVelocity;
  auto yv = cell->yVelocity;

  // Gain
  for (auto i = 0; i < 8; ++i) {
    auto movement = neighbors[i]->output[i];
    cell->fluid = std::min(255, movement + cell->fluid);
  }

  // Loss
  int maxLoss = (int)(std::abs(xv) + std::abs(yv));
  int loss = std::min(255, maxLoss);

  if (loss > 0) {
    auto coord = std::vector<double>{0, 0};
    auto velocity = std::vector<double>{xv, yv};
    auto similarities = std::vector<double>();
    for (auto y = -1; y <= 1; ++y) {
      coord[1] = y;
      for (auto x = -1; x <= 1; ++x) {
        coord[0] = x;

        if (x == 0 && y == 0) {
          continue;
        }

        similarities.push_back(
            std::max(cosineSimilarity(coord, velocity), 0.0));
      }
    }

    auto similarityTotal = 0.0;
    for (auto similarity : similarities) {
      similarityTotal += similarity;
    }

    for (auto i = 0; i < similarities.size(); ++i) {
      auto movement =
          static_cast<uint8_t>((similarities[i] / similarityTotal) * loss);
      cell->output[NUM_NEIGHBORS - i] = movement;
      cell->fluid -= movement;
    }
  }


  // Velocity change
  // cell->xVelocity = BETA1 * xv * (1.0 - xv) - GAMMA1 * xv * yv;
  // cell->yVelocity = BETA2 * yv * (1.0 - yv) - GAMMA2 * xv * yv;

  cell->xVelocity *= 0.98;
  cell->yVelocity *= 1.0001;
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> colorize(const FluidCell& cell) {
  auto r = cell.fluid / 2;
  auto g = cell.fluid / 2;
  auto b = cell.fluid;
  return {r, g, b, 255};
}

// Randomize
// =========

double randUnitInterval() { return (double)(rand()) / (double)(RAND_MAX); }

bool randBool() { return rand() % 2 == 0; }

void randomize(Grid<FluidCell>& grid) {
  srand(time(0));
  auto coord = std::vector<size_t>{0, 0};
  for (auto i = 0; i < GRID_HEIGHT; ++i) {
    coord[1] = i;
    for (auto j = 0; j < GRID_WIDTH; ++j) {
      coord[0] = j;
      double xv = j * .0015 + i * 0.075;
      double yv = j * 0.05 + i * 0.060;
      grid.setValue(coord, FluidCell{xv, yv, (uint8_t)(rand() % 255)});
    }
  }
}

// Main Function
// =============

int main() {
  {
    auto grid = std::shared_ptr<Grid<FluidCell>>(
        new Grid<FluidCell>{{GRID_WIDTH, GRID_HEIGHT},
                            Wrapping::TOROIDAL,
                            Neighborhood::MOORE,
                            update,
                            FluidCell{0.0, 0.0, 0}});
    randomize(*grid);

    Ortho2DColorRenderer<FluidCell> renderer{grid,      colorize,     CELL_SIZE,
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