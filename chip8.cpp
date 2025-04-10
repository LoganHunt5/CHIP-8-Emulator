// TODO: after jump to F70, 1F70 code, it is giving 0000 as hexes

#include "SDL_lib.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stack>
#include <thread>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

// Screen dimension constants
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

// globals
// Window
SDL_Window *gWindow = NULL;
//
SDL_Renderer *gRenderer = NULL;

class chip8 {
public:
  uint8_t memory[4096]{};
  uint16_t pc{};
  uint16_t index{};
  std::stack<uint16_t> stack{};
  uint8_t sp{};
  uint8_t delay_timer{};
  uint8_t sound_timer{};
  int8_t registers[16]{};
  // to hold a hex color value, 0x00000000 or 0xFFFFFFFF
  uint32_t video[32 * 64]{};
  uint16_t opcode{};
};

bool loadFont(chip8 *Chip);

bool importGame(char *fn, std::ifstream *file, chip8 *Chip);

void loop(chip8 *Chip);

bool init();

void close();

int main(int argc, char *argv[]) {
  chip8 TheChip;
  std::ifstream gameFile;
  if (loadFont(&TheChip)) {
    if (importGame(argv[1], &gameFile, &TheChip)) {
      loop(&TheChip);
      close();
    }
  }

  return 0;
}

bool loadFont(chip8 *Chip) {
  uint8_t tempfont[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF1, 0x10,
      0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
      0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
      0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
      0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
      0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
      0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};
  uint8_t inc = 0x050;
  for (auto x : tempfont) {
    Chip->memory[inc] = x;
    ++inc;
  }
  return true;
}

bool importGame(char *fn, std::ifstream *file, chip8 *Chip) {
  file->open(fn);
  if (!file->is_open()) {
    std::cerr << "couldn't open file:" << fn << std::endl;
    return false;
  }
  Chip->pc = 0x200;
  unsigned char x;
  uint8_t instruction;
  while ((*file).get((char &)x)) {
    instruction = ((uint8_t)x);
    Chip->memory[Chip->pc++] = (uint8_t)x;
    // printf("%02X ", instruction);
  }
  std::cout << std::endl;
  Chip->pc = 0x200;
  return true;
}

void loop(chip8 *Chip) {
  if (!init()) {
    printf("failed to init\n");
  } else {
    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    while (!quit) {
      // true if a draw instruction is read
      bool render = false;
      std::chrono::system_clock::time_point timerstart =
          std::chrono::high_resolution_clock::now();
      for (int i = 0; i < 1; i++) {
        // fetch
        // combine the 2 8 bit halves of the instruction
        Chip->opcode = ((uint16_t)Chip->memory[Chip->pc] << 8) |
                       ((uint16_t)Chip->memory[Chip->pc + 1]);
        Chip->pc += 2;

        //  decode
        uint8_t firstNumber = (Chip->opcode >> 8) & 0xF0;
        // printf("First Number: %02X Full: %04X\n", firstNumber, Chip->opcode);
        switch (firstNumber) {

        case 0x00:
          switch (Chip->opcode) {
          case 0x00E0:
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
            SDL_RenderClear(gRenderer);
            break;
          case 0x00EE:
            Chip->pc = Chip->stack.top();
            (Chip->stack).pop();
            break;
          }
          break;

        case 0x10:
          switch (Chip->opcode) {
          default:
            Chip->pc = Chip->opcode & 0x0FFF;
            break;
          }
          break;

        case 0x20:
          switch (Chip->opcode) {
          default:
            Chip->pc = Chip->opcode & 0x0FFF;
            Chip->stack.push(Chip->opcode & 0x0FFF);
            break;
          }
          break;

        case 0x30:
          switch (Chip->opcode) {
          default:
            // if VX = NN in 3XNN skip 1 instruction
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            if (Chip->registers[regi] == (uint8_t)(Chip->opcode & 0x00FF)) {
              Chip->pc += 2;
            }
            break;
          }
          break;

        case 0x40:
          switch (Chip->opcode) {
          default:
            // if VX != NN in 4XNN skip 1 instruction
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            if (Chip->registers[regi] != (uint8_t)(Chip->opcode & 0x00FF)) {
              Chip->pc += 2;
            }
            break;
          }
          break;

        case 0x50:
          switch (Chip->opcode) {
          default:
            // if VX == VY in 5XY0 skip 1 instruction
            uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
            if (Chip->registers[regi1] == Chip->registers[regi2]) {
              Chip->pc += 2;
            }
            break;
          }
          break;

        case 0x60:
          switch (Chip->opcode) {
          default:
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            Chip->registers[regi] = (uint8_t)(Chip->opcode);
            break;
          }
          break;

        case 0x70:
          switch (Chip->opcode) {
          default:
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            Chip->registers[regi] += (uint8_t)(Chip->opcode);
            break;
          }
          break;

        case 0x90:
          switch (Chip->opcode) {
          default:
            // if VX != VY in 9XY0 skip 1 instruction
            uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
            uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
            if (Chip->registers[regi1] != Chip->registers[regi2]) {
              Chip->pc += 2;
            }
            break;
          }
          break;

        case 0xA0:
          switch (Chip->opcode) {
          default:
            Chip->index = Chip->opcode & 0x0FFF;
            break;
          }
          break;

        case 0xD0:
          switch (Chip->opcode) {
          // draw command
          default:
            render = true;
            uint8_t x =
                Chip->registers[(uint8_t)((Chip->opcode & 0x0F00) << 8)] %
                SCREEN_WIDTH;
            uint8_t y = Chip->registers[(uint8_t)(Chip->opcode & 0x00F0)] %
                        SCREEN_HEIGHT;
            // height
            uint8_t N = (uint8_t)(Chip->opcode) & 0x0F;
            // printf("x: %X\n", x);
            // VF keeps track of collision in sprites
            Chip->registers[0x0F] = 0;

            printf("Drawing Sprite: %X at X: %X, Y: %X, Height: %X\n",
                   Chip->index, x, y, N);
            for (int row = 0; row < N; row++) {
              uint8_t spriteRow = Chip->memory[Chip->index + row];
              // printf("Sprite Row: %X\n", spriteRow);

              for (int col = 0; col < 8; col++) {

                // POSSIBLE BUG BECAUSE OF SCALE CHANGE IN INIT
                if (col + x > SCREEN_WIDTH) {
                  break;
                }

                uint32_t *screenPixel =
                    &(Chip->video[((row + y) * SCREEN_WIDTH) + (col + x)]);

                uint8_t spriteCur = (((spriteRow >> (7 - col)) & 0x01) == 0x01);
                // printf("%X ", spriteCur);
                // collision
                if (spriteCur && (*screenPixel == 0xFFFFFFFF)) {
                  *screenPixel = 0x00000000;
                  Chip->registers[0x0F] = 1;
                } else if (spriteCur) {
                  *screenPixel ^= 0xFFFFFFFF;
                }

                if (*screenPixel == 0xFFFFFFFF) {
                  // printf("drawing at %d, %d ", col + x, row + y);
                  SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                  SDL_RenderDrawPoint(gRenderer, col + x, row + y);
                } else {
                  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
                  SDL_RenderDrawPoint(gRenderer, (int)(col + x),
                                      (int)(row + y));
                }
              }
              // puts("");
            }
            break;
          }
          break;
        }
      }

      //  printf("%04X ", Chip->opcode);
      // printf("\n")
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
      }
      std::chrono::system_clock::time_point timerend =
          std::chrono::high_resolution_clock::now();
      long waittime = std::chrono::duration_cast<std::chrono::microseconds>(
                          timerend - timerstart)
                          .count();
      // for 60fps
      std::this_thread::sleep_for(
          std::chrono::milliseconds(1000 / 60 - waittime));

      // 1fps
      // std::this_thread::sleep_for(std::chrono::milliseconds(1000 -
      // waittime)); puts("rendering");
      if (render) {
        render = false;
        SDL_RenderPresent(gRenderer);
      }
    }
  }
}

bool init() {

  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not init! SDL_Error: %s\n", SDL_GetError());
    success = false;
  } else {
    // create
    gWindow = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                               SCREEN_HEIGHT, 0);
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_RenderPresent(gRenderer);
    SDL_RenderSetScale(gRenderer, 6, 6);
    SDL_SetWindowSize(gWindow, SCREEN_WIDTH * 6, SCREEN_HEIGHT * 6);
    if (gWindow == NULL || gRenderer == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    }
  }
  return success;
}

void close() {
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
}
