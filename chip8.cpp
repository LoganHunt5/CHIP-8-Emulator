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

int main(int argc, char *argv[]) {
  chip8 TheChip;
  std::ifstream gameFile;
  if (loadFont(&TheChip)) {
    if (importGame(argv[1], &gameFile, &TheChip)) {
      loop(&TheChip);
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
  uint8_t x = 0x00;
  int counter = 0;
  while (*file >> x) {
    Chip->memory[Chip->pc] = x;
    Chip->pc++;
    counter++;
  }
  std::cout << '\n' << std::endl;
  Chip->pc = 0x200;
  return true;
}

void loop(chip8 *Chip) {
  if (!init(gWindow, SCREEN_HEIGHT, SCREEN_WIDTH, gRenderer)) {
    printf("failed to init\n");
  } else {
    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    while (!quit) {
      std::chrono::system_clock::time_point timerstart =
          std::chrono::high_resolution_clock::now();
      for (int i = 0; i < 6; i++) {
        // fetch
        // combine the 2 8 bit halves of the instruction
        Chip->opcode = ((uint16_t)Chip->memory[Chip->pc] << 8) |
                       ((uint16_t)Chip->memory[Chip->pc + 1]);
        Chip->pc += 2;

        //  decode
        uint8_t firstNumber = (Chip->opcode >> 8) & 0xF0;
        printf("First Number: %02X Full: %04X\n", firstNumber, Chip->opcode);
        switch (firstNumber) {

        case 0x00:
          switch (Chip->opcode) {
          case 0x00E0:
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
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) << 8);
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
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) << 8);
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
            uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) << 8);
            uint8_t regi2 = (uint8_t)(Chip->opcode & 0x00F0);
            if (Chip->registers[regi1] == Chip->registers[regi2]) {
              Chip->pc += 2;
            }
            break;
          }
          break;

        case 0x60:
          switch (Chip->opcode) {
          default:
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) << 8);
            Chip->registers[regi] = (uint8_t)(Chip->opcode);
            break;
          }
          break;

        case 0x70:
          switch (Chip->opcode) {
          default:
            uint8_t regi = (uint8_t)((Chip->opcode & 0x0F00) << 8);
            Chip->registers[regi] += (uint8_t)((Chip->opcode & 0x00FF) << 8);
            break;
          }
          break;

        case 0x90:
          switch (Chip->opcode) {
          default:
            // if VX != VY in 9XY0 skip 1 instruction
            uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) << 8);
            uint8_t regi2 = (uint8_t)(Chip->opcode & 0x00F0);
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
            printf("%X", Chip->opcode);
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
      // std::this_thread::sleep_for(
      //    std::chrono::milliseconds(1000 / 60 - waittime));

      std::this_thread::sleep_for(std::chrono::milliseconds(1000 - waittime));
      SDL_UpdateWindowSurface(gWindow);
    }
  }
}
