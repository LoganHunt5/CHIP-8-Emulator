// Using SDL and standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <thread>
#include <unordered_map>

#define FRAMERATE 60

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
  uint32_t video[64 * 128]{};
  uint16_t opcode{};
  int flags[64];
  int speed;
  bool quit;
  bool render;
  bool clear;
};

enum quirkType { CHIP, SUPERCHIPMODERN, SUPERCHIPLEGACY, XOCHIP };

bool loadFont(chip8 *Chip);

bool importGame(char *fn, std::ifstream *file, chip8 *Chip);

void initChip(chip8 *Chip);

void loop(chip8 *Chip);

void redraw(chip8 *Chip);

void makeTablekeyKeytable();

bool init();

void close();

void renderDraw(bool clear, bool on);

void resize();

bool checkQuit();

void op_00CN(chip8 *Chip);

void op_00FB(chip8 *Chip);

void op_00FC(chip8 *Chip);

void op_3NNN(chip8 *Chip);

void op_4XNN(chip8 *Chip);

void op_5XY0(chip8 *Chip);

void op_6XNN(chip8 *Chip);

void op_7XNN(chip8 *Chip);

void op_8NNN(chip8 *Chip);

void op_9XY0(chip8 *Chip);

void op_BNNN(chip8 *Chip);

void op_CXNN(chip8 *Chip);

void op_DXY0(chip8 *Chip);

void op_ENNN(chip8 *Chip);

void op_FX07(chip8 *Chip);

bool op_FX0A(chip8 *Chip);

void op_FX15(chip8 *Chip);

void op_FX18(chip8 *Chip);

void op_FX1E(chip8 *Chip);

void op_FX29(chip8 *Chip);

void op_FX30(chip8 *Chip);

void op_FX33(chip8 *Chip);

void op_FX55(chip8 *Chip);

void op_FX65(chip8 *Chip);
