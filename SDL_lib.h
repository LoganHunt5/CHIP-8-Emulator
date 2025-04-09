// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>

bool init(SDL_Window *gWindow, int h, int w, SDL_Renderer *renderer);
void draw(SDL_Window *gWindow, SDL_Renderer *renderer, int x, int y, int n);
void close(SDL_Window *gWindow, SDL_Renderer *renderer);
