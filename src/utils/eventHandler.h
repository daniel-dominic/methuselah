#pragma once

#include <SDL2/SDL.h>

#include <functional>
#include <unordered_map>

namespace methuselah {

class EventHandler {
 public:
  EventHandler() : quitSignal(false) {}

  void handleAll() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
        case SDL_QUIT:
          quitSignal = true;
          break;
        case SDL_KEYDOWN:
          if (e.key.keysym.sym == SDLK_ESCAPE) {
            quitSignal = true;
          }
          auto it = keyDownActions.find(e.key.keysym.sym);
          if (it != keyDownActions.end()) {
            it->second();
          }
          break;
      }
    }
  }

  void registerKeyDownAction(SDL_Keycode keycode,
                             std::function<void()> action) {
    keyDownActions.insert({keycode, action});
  }

  bool receivedQuitSignal() const { return quitSignal; }

 private:
  std::unordered_map<SDL_Keycode, std::function<void()>> keyDownActions;
  bool quitSignal;
};

}  // namespace methuselah