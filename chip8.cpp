// TODO: Send the clear screen command to SDL file... so start the decode step
// with switch statement

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stack>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

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
  std::vector<uint8_t> registers[16]{};
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
    /*if (counter % 2 == 1) {

      printf("%02X", x);
    } else {

      printf("%02X ", x);
    }*/
  }
  std::cout << '\n' << std::endl;
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
      for (int i = 0; i < 600; i++) {
        // fetch
        // combine the 2 8 bit halves of the instruction
        Chip->opcode = ((uint16_t)Chip->memory[Chip->pc] << 8) |
                       ((uint16_t)Chip->memory[Chip->pc + 1]);
        Chip->pc += 2;
        // printf("%04X ", Chip->opcode);
      }
      // printf("\n")
      while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
      }
      SDL_UpdateWindowSurface(gWindow);
    }
  }
}
