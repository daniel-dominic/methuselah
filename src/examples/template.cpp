#include <SDL2/SDL.h>

constexpr unsigned int CELL_SIZE = 10;
constexpr unsigned int GENERATIONS = 1000;
constexpr unsigned int DELAY = 500;

constexpr unsigned short int WINDOW_WIDTH = 800;
constexpr unsigned short int WINDOW_HEIGHT = 600;

int main(int argc, char* argv[]) {
  SDL_Rect SrcR{0, 0, CELL_SIZE, CELL_SIZE};

  auto window =
      SDL_CreateWindow("Game Of Life", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  for (auto i = 0; i < GENERATIONS; ++i) {
    SDL_RenderPresent(renderer);
    SDL_Delay(DELAY);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}