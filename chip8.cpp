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
  uint8_t registers[16]{};
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
        uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
        uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
        uint8_t *data1 = &(Chip->registers[regi1]);
        uint8_t *data2 = &(Chip->registers[regi2]);
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
          Chip->pc = Chip->opcode & 0x0FFF;
          break;

        case 0x20:
          Chip->pc = Chip->opcode & 0x0FFF;
          Chip->stack.push(Chip->opcode & 0x0FFF);
          break;

        case 0x30:
          // if VX = NN in 3XNN skip 1 instruction
          if (*data1 == (uint8_t)(Chip->opcode & 0x00FF)) {
            Chip->pc += 2;
          }
          break;

        case 0x40:
          // if VX != NN in 4XNN skip 1 instruction
          if (*data1 != (uint8_t)(Chip->opcode & 0x00FF)) {
            Chip->pc += 2;
          }
          break;

        case 0x50:
          // if VX == VY in 5XY0 skip 1 instruction
          if (*data1 == *data2) {
            Chip->pc += 2;
          }
          break;

        case 0x60:
          *data1 = (uint8_t)(Chip->opcode);
          break;

        case 0x70:
          *data1 += (uint8_t)(Chip->opcode);
          break;
        case 0x80:
          switch ((uint8_t)(Chip->opcode & 0x000F)) {
          case 0x00:
            *data1 = *data2;
            break;
          case 0x01:
            *data1 |= *data2;
            break;
          case 0x02:
            *data1 &= *data2;
            break;
          case 0x03:
            *data1 ^= *data2;
            break;
          case 0x04:
            // VF set to 0 if overflow
            Chip->registers[0x0F] = 0;
            if ((*data2 > 0) && (*data1 > UINT8_MAX - *data1)) {
              Chip->registers[0x0F] = 1;
            }
            *data1 += *data2;
            break;
          case 0x05:
            // VF set to 0 if underflow
            Chip->registers[0x0F] = 1;
            if (*data1 < *data2) {
              Chip->registers[0x0F] = 0;
            }
            *data1 -= *data2;
            break;
          case 0x06:
            // store least significant then bit shift by 1
            Chip->registers[0x0F] = *data1 & 0x01;
            *data1 >>= 1;
            break;
          case 0x07:
            // VF set to 0 if underflow
            Chip->registers[0x0F] = 1;
            if (*data1 > *data2) {
              Chip->registers[0x0F] = 0;
            }
            *data1 = *data2 - *data1;
            break;
          case 0x0E:
            // store least significant then bit shift by 1
            // POSSIBLE BUG didn't really rhingk about it
            Chip->registers[0x0F] = *data1 >> 7;
            *data1 <<= 1;
            break;
          }
          break;
        case 0x90:
          // if VX != VY in 9XY0 skip 1 instruction
          if (*data1 != *data2) {
            Chip->pc += 2;
          }
          break;

        case 0xA0:
          Chip->index = Chip->opcode & 0x0FFF;
          break;

        case 0xD0:
          // draw command
          render = true;
          uint8_t x = *data1 % SCREEN_WIDTH;
          uint8_t y = *data2 % SCREEN_HEIGHT;
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
                SDL_SetRenderDrawColor(gRenderer, 91, 222, 139, 255);
                SDL_RenderDrawPoint(gRenderer, col + x, row + y);
              } else {
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
                SDL_RenderDrawPoint(gRenderer, (int)(col + x), (int)(row + y));
              }
            }
            // puts("");
          }
          break;
          /*
        case 0xE0:
        switch((uint8_t)Chip->opcode){
            case 0x9E:

            break;
            case 0xA1:
            break;
          }*/
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
    gWindow = SDL_CreateWindow("CHIP-8", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    SDL_SetRenderDrawColor(gRenderer, 91, 222, 139, 255);
    SDL_RenderPresent(gRenderer);

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    float userWidth = DM.w;
    float userHeight = DM.h;
    std::cout << (userWidth - 100) / SCREEN_WIDTH << std::endl;
    SDL_RenderSetScale(gRenderer, (userWidth) / SCREEN_WIDTH,
                       (userHeight) / SCREEN_HEIGHT);
    SDL_SetWindowSize(gWindow, (userWidth) / SCREEN_WIDTH * SCREEN_WIDTH,
                      (userHeight) / SCREEN_HEIGHT * SCREEN_HEIGHT);

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
