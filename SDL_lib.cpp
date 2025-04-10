#include "SDL_lib.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

/*bool init(SDL_Window *gWindow, int h, int w, SDL_Renderer *renderer) {

  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not init! SDL_Error: %s\n", SDL_GetError());
    success = false;
  } else {
    // create
    gWindow = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(gWindow, -1, 0);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderPresent(renderer);
    if (gWindow == NULL || renderer == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    }
  }
  return success;
}

void draw(SDL_Window *gWindow, SDL_Renderer *renderer, uint8_t x, uint8_t y,
          uint8_t n) {}

void close(SDL_Window *gWindow, SDL_Renderer *renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(gWindow);

  SDL_Quit();
}*/
