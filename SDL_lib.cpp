#include "SDL_lib.h"
/*
    while (!quit) {
      // handle events in the even queue
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
      }
      SDL_UpdateWindowSurface(gWindow);
    }
  }
}
*/

bool init(SDL_Window *gWindow, int h, int w, SDL_Renderer *renderer) {

  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not init! SDL_Error: %s\n", SDL_GetError());
    success = false;
  } else {
    // create
    SDL_CreateWindowAndRenderer(w, h, 0, &gWindow, &renderer);
    if (gWindow == NULL || renderer == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    }
  }
  return success;
}

void update(SDL_Window *gWindow, SDL_Renderer *renderer, int x, int y, int n) {}

void close(SDL_Window *gWindow, SDL_Renderer *renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(gWindow);

  SDL_Quit();
}
