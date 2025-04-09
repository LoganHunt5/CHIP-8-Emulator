/*This source code copyrighted by Lazy Foo' Productions 2004-2024
  and may not be redistributed without written permission.*/

// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <stdio.h>

// Screen dimension constants
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

// globals
// Window
SDL_Window *gWindow = NULL;

// surface in the window
// (a 2d image)
SDL_Surface *gScreenSurface = NULL;

// image we will load and show on screen
SDL_Surface *gHelloWorld = NULL;
SDL_Surface *gHelloAlt = NULL;

bool init();

bool loadMedia();

void close();

int main(int argc, char *args[]) {

  if (!init()) {
    printf("failed to init\n");
  } else {
    if (!loadMedia()) {
      printf("failed to load media!\n");
    } else {
      bool alt = false;
      // Main loop flag
      bool quit = false;

      // Event handler
      SDL_Event e;

      while (!quit) {
        // handle events in the even queue
        while (SDL_PollEvent(&e) != 0) {
          if (e.type == SDL_QUIT) {
            quit = true;
          }
        }
        if (alt) {
          SDL_BlitSurface(gHelloAlt, NULL, gScreenSurface, NULL);
          alt = !alt;
        } else {
          // apply image
          SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
          alt = !alt;
        }
        printf("%d\n", alt);
        SDL_UpdateWindowSurface(gWindow);
      }
    }
  }
  close();
  return 0;
}

bool init() {

  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not init! SDL_Error: %s\n", SDL_GetError());
    success = false;
  } else {
    // create
    gWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    } else {
      // get window surface
      gScreenSurface = SDL_GetWindowSurface(gWindow);
    }
  }
  return success;
}

bool loadMedia() {
  bool success = true;
  gHelloWorld = SDL_LoadBMP("TestingImages/hello_world.bmp");
  gHelloAlt = SDL_LoadBMP("TestingImages/hello_alt.bmp");
  if (gHelloWorld == NULL || gHelloAlt == NULL) {
    printf("Unable to load image %s, Error: %s\n",
           "TestingImages/hello_world.bmp", SDL_GetError());
    success = false;
  }
  return success;
}

void close() {

  // deallocate surface
  SDL_FreeSurface(gHelloWorld);
  gHelloWorld = NULL;

  SDL_DestroyWindow(gWindow);
  gWindow = NULL;

  SDL_Quit;
}
