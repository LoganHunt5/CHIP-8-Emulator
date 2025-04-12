// TODO Working on 8XY4

#include "SDL_lib.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_map>

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

enum quirkType { CHIP, SUPERCHIPMODERN, SUPERCHIPLEGACY, XOCHIP };
enum quirkType chipType = SUPERCHIPMODERN;

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
  uint32_t video[32 * 64]{0x00000000};
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
  uint16_t inc = 0x050;
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
    std::unordered_map<int, uint8_t> keytable;
    keytable[(int)SDL_SCANCODE_1] = 0x01;
    keytable[(int)SDL_SCANCODE_2] = 0x02;
    keytable[(int)SDL_SCANCODE_3] = 0x03;
    keytable[(int)SDL_SCANCODE_4] = 0x0C;
    keytable[(int)SDL_SCANCODE_Q] = 0x04;
    keytable[(int)SDL_SCANCODE_W] = 0x05;
    keytable[(int)SDL_SCANCODE_E] = 0x06;
    keytable[(int)SDL_SCANCODE_R] = 0x0D;
    keytable[(int)SDL_SCANCODE_A] = 0x07;
    keytable[(int)SDL_SCANCODE_S] = 0x08;
    keytable[(int)SDL_SCANCODE_D] = 0x09;
    keytable[(int)SDL_SCANCODE_F] = 0x0E;
    keytable[(int)SDL_SCANCODE_Z] = 0x0A;
    keytable[(int)SDL_SCANCODE_X] = 0x00;
    keytable[(int)SDL_SCANCODE_C] = 0x0B;
    keytable[(int)SDL_SCANCODE_V] = 0x0F;

    int tablekey[17];
    tablekey[0x01] = (int)SDL_SCANCODE_1;
    tablekey[0x02] = (int)SDL_SCANCODE_2;
    tablekey[0x03] = (int)SDL_SCANCODE_3;
    tablekey[0x0C] = (int)SDL_SCANCODE_4;
    tablekey[0x04] = (int)SDL_SCANCODE_Q;
    tablekey[0x05] = (int)SDL_SCANCODE_W;
    tablekey[0x06] = (int)SDL_SCANCODE_E;
    tablekey[0x0D] = (int)SDL_SCANCODE_R;
    tablekey[0x07] = (int)SDL_SCANCODE_A;
    tablekey[0x08] = (int)SDL_SCANCODE_S;
    tablekey[0x09] = (int)SDL_SCANCODE_D;
    tablekey[0x0E] = (int)SDL_SCANCODE_F;
    tablekey[0x0A] = (int)SDL_SCANCODE_Z;
    tablekey[0x00] = (int)SDL_SCANCODE_X;
    tablekey[0x0B] = (int)SDL_SCANCODE_C;
    tablekey[0x0F] = (int)SDL_SCANCODE_V;
    std::srand(std::time(0));

    // used in cases
    uint8_t firstNumber;
    uint8_t regi1;
    uint8_t regi2;
    uint8_t *data1;
    uint8_t *data2;
    uint8_t x;
    uint8_t y;
    uint8_t N;
    int bcdint;
    bool overflow;
    bool underflow;
    bool collision;
    uint8_t carryout;
    int cycles;
    const uint8_t *keyboardstate = SDL_GetKeyboardState(NULL);
    // for keypress
    SDL_Event event;
    bool press;

    // Event handler
    SDL_Event e;

    while (!quit) {
      // true if a draw instruction is read
      bool render = false;
      std::chrono::system_clock::time_point timerstart =
          std::chrono::high_resolution_clock::now();
      if (Chip->delay_timer > 0) {
        --Chip->delay_timer;
      }
      if (Chip->sound_timer > 0) {
        --Chip->sound_timer;
      }
      cycles = 0;
      while (!render && cycles < 10) {
        cycles++;
        while (SDL_PollEvent(&e) != 0) {
          if (e.type == SDL_QUIT) {
            quit = true;
            render = true;
          }
        }
        // fetch
        // combine the 2 8 bit halves of the instruction
        Chip->opcode = ((uint16_t)Chip->memory[Chip->pc] << 8) |
                       ((uint16_t)Chip->memory[Chip->pc + 1]);
        Chip->pc += 2;

        //  decode
        firstNumber = (Chip->opcode >> 8) & 0xF0;
        regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
        regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
        data1 = &(Chip->registers[regi1]);
        data2 = &(Chip->registers[regi2]);
        /*
        if (Chip->index == 0x00AC) {
          printf("Full: %04X\n", Chip->opcode);
        }
        */
        switch (firstNumber) {
        case 0x00:
          switch (Chip->opcode) {
          case 0x00E0:
            //            puts("clearing");
            memset(Chip->video, 0x00000000, sizeof(Chip->video));
            renderDraw(true, false);
            break;
          case 0x00EE:
            Chip->pc = Chip->stack.top();
            (Chip->stack).pop();
            // std::cout << "Stack Top" << std::hex << Chip->stack.top()
            //           << std::endl;
            break;
          case 0x00FE:
            // lores toggle
            break;
          case 0x00FF:
            // hires toggle
            break;
          }
          break;

        case 0x10:
          Chip->pc = Chip->opcode & 0x0FFF;
          break;

        case 0x20:
          Chip->stack.push(Chip->pc & 0x0FFF);
          Chip->pc = Chip->opcode & 0x0FFF;
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
            switch (chipType) {
            case (0):
              Chip->registers[0x0F] = 0;
              break;
            case (1):
            case (2):
            case (3):
              break;
            }
            *data1 |= *data2;
            break;
          case 0x02:
            switch (chipType) {
            case (0):
              Chip->registers[0x0F] = 0;
              break;
            case (1):
            case (2):
            case (3):
              break;
            }
            *data1 &= *data2;
            break;
          case 0x03:
            switch (chipType) {
            case (0):
              Chip->registers[0x0F] = 0;
              break;
            case (1):
            case (2):
            case (3):
              break;
            }
            *data1 ^= *data2;
            break;
          case 0x04:
            // VF set to 1 if overflow
            overflow = false;
            if (((uint16_t)*data1 + (uint16_t)*data2) > 0x00FF) {
              overflow = true;
            }
            *data1 += *data2;
            if (overflow) {
              Chip->registers[0x0F] = 1;
            } else {
              Chip->registers[0x0F] = 0;
            }
            break;
          case 0x05:
            // VF set to 0 if underflow
            underflow = false;
            if (*data1 < *data2) {
              underflow = true;
            }
            *data1 -= *data2;
            if (underflow) {
              Chip->registers[0x0F] = 0;
            } else {
              Chip->registers[0x0F] = 1;
            }
            break;
          case 0x06:
            // store least significant then bit shift by 1
            switch (chipType) {
            case (0):
            case (3):
              carryout = *data2 & 0x01;
              *data1 = *data2 >> 1;
              Chip->registers[0x0F] = carryout;
              break;
            case (1):
            case (2):
              carryout = *data1 & 0x01;
              *data1 >>= 1;
              Chip->registers[0x0F] = carryout;
              break;
            }
            break;
          case 0x07:
            // VF set to 0 if underflow
            underflow = false;
            if (*data1 > *data2) {
              underflow = true;
            }
            *data1 = *data2 - *data1;
            if (underflow) {
              Chip->registers[0x0F] = 0;
            } else {
              Chip->registers[0x0F] = 1;
            }
            break;
          case 0x0E:
            // store least significant then bit shift by 1
            switch (chipType) {
            case (0):
            case (3):
              carryout = *data2 >> 7;
              *data1 = *data2 << 1;
              Chip->registers[0x0F] = carryout;
              break;
            case (1):
            case (2):
              carryout = *data1 >> 7;
              *data1 <<= 1;
              Chip->registers[0x0F] = carryout;
              break;
            }

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

        case 0xB0:
          switch (chipType) {
          case (0):
          case (3):
            Chip->pc = Chip->registers[0] + Chip->opcode & 0x0FFF;
            break;
          case (1):
          case (2):
            Chip->pc = *data1 + Chip->opcode & 0x0FFF;
            break;
          }
          break;

        case 0xC0:
          *data1 = std::rand() % 256 & (uint8_t)Chip->opcode;
          break;
        case 0xD0:
          // draw command
          render = true;
          x = *data1 % SCREEN_WIDTH;
          y = *data2 % SCREEN_HEIGHT;
          // height
          N = (uint8_t)(Chip->opcode) & 0x0F;
          // printf("x: %X\n", x);
          // VF keeps track of collision in sprites

          Chip->registers[0x0F] = 0;
          //          printf("Drawing Sprite: %X at X: %X, Y: %X, Height: %X\n",
          //                 Chip->index, x, y, N);
          for (int row = 0; row < N; row++) {
            if (row + y > SCREEN_HEIGHT) {
              break;
            }
            uint8_t spriteRow = Chip->memory[Chip->index + row];
            // printf("Sprite Row: %X\n", spriteRow);

            for (int col = 0; col < 8; col++) {

              // POSSIBLE BUG BECAUSE OF SCALE CHANGE IN INIT
              if (col + x > SCREEN_WIDTH) {
                break;
              }

              uint32_t *screenPixel =
                  &(Chip->video[((row + y) * SCREEN_WIDTH) + (col + x)]);

              uint8_t spriteCur = ((spriteRow >> (7 - col)) & 0x01);
              //              printf("%X ", spriteCur);
              // collision
              if (spriteCur) {
                if (*screenPixel == 0xFFFFFFFF) {
                  *screenPixel = 0x00000000;
                  Chip->registers[0x0F] = 1;
                  renderDraw(false, false);
                } else {
                  *screenPixel = 0xFFFFFFFF;
                  renderDraw(false, true);
                  // printf("drawing at %d, %d ", col + x, row + y);
                }
                SDL_RenderDrawPoint(gRenderer, (col + x), (row + y));
              }
            }
            //           puts("");
          }
          break;

        case 0xE0:
          switch ((uint8_t)Chip->opcode) {
          case 0x9E:
            // increment if key pressed is same as
            SDL_PumpEvents();
            if (keyboardstate[tablekey[*data1]] == 1) {
              Chip->pc += 2;
            }
            break;
          case 0xA1:
            SDL_PumpEvents();
            // printf("EXA1: %d\n", keyboardstate[SDL_SCANCODE_V]);
            if (keyboardstate[tablekey[*data1]] == 0) {
              Chip->pc += 2;
            }
          }
          break;
          break;

        case 0xF0:
          switch ((uint8_t)Chip->opcode) {
          case 0x07:
            *data1 = Chip->delay_timer;
            break;
          case 0x0A:
            // wait for keypress
            press = false;
            while (!press) {
              std::chrono::system_clock::time_point presstimerstart =
                  std::chrono::high_resolution_clock::now();
              if (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                  quit = true;
                  press = true;
                  break;
                }
                // std::cout << "scancode: " << event.key.keysym.scancode
                //           << std::endl;
                if (keytable.find((int)event.key.keysym.scancode) !=
                    keytable.end()) {
                  *data1 = keytable[(int)event.key.keysym.scancode];
                  press = true;
                }
              }
              std::chrono::system_clock::time_point presstimerend =
                  std::chrono::high_resolution_clock::now();
              long presswaittime =
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      presstimerend - presstimerstart)
                      .count();
              std::this_thread::sleep_for(
                  std::chrono::milliseconds(1000 / 60 - presswaittime));
              if (Chip->delay_timer > 0) {
                --Chip->delay_timer;
              }
              if (Chip->sound_timer > 0) {
                --Chip->sound_timer;
              }
            }
            break;
          case 0x15:
            Chip->delay_timer = *data1;
            break;
          case 0x18:
            Chip->sound_timer = *data1;
            break;
          case 0x1E:
            Chip->index += *data1;
            Chip->index = Chip->index & 0x0FFF;
            break;
          case 0x29:
            // access font at VX
            Chip->index = 0x050 + (*data1 & 0x0F) * 5;
            break;
          case 0x33:
            // store bcd of VX at index, index+1, index+2
            // NEEDS TESTING
            bcdint = *data1;
            for (int i = 0; i < 3; ++i) {
              Chip->memory[Chip->index + (2 - i)] = (uint8_t)(bcdint % 10);
              bcdint /= 10;
            }
            break;
          case 0x55:
            // memory dump from V0 to VX inlcuding VX
            switch (chipType) {
            case (0):
            case (3):
              for (int i = 0; i <= regi1; ++i) {
                Chip->memory[Chip->index] = Chip->registers[i];
                ++Chip->index;
              }
              break;
            case (1):
            case (2):
              for (int i = 0; i <= regi1; ++i) {
                Chip->memory[Chip->index + i] = Chip->registers[i];
              }
              break;
            }

            break;
          case 0x65:
            // Write from memeory V0 to VX inlcuding VX
            switch (chipType) {
            case (0):
            case (3):
              for (int i = 0; i <= regi1; ++i) {
                Chip->registers[i] = Chip->memory[Chip->index];
                ++Chip->index;
              }
              break;
            case (1):
            case (2):
              for (int i = 0; i <= regi1; ++i) {
                Chip->registers[i] = Chip->memory[Chip->index + i];
              }
              break;
            }

            break;
          }
          break;
        }
      }

      //  printf("%04X ", Chip->opcode);
      // printf("\n")
      std::chrono::system_clock::time_point timerend =
          std::chrono::high_resolution_clock::now();
      long waittime = std::chrono::duration_cast<std::chrono::milliseconds>(
                          timerend - timerstart)
                          .count();
      // for 60fps
      std::this_thread::sleep_for(
          std::chrono::milliseconds(1000 / 60 - waittime));

      SDL_RenderPresent(gRenderer);
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

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    float userWidth = DM.w;
    float userHeight = DM.h;
    SDL_RenderSetScale(gRenderer, (userWidth - 100) / SCREEN_WIDTH,
                       (userHeight - 100) / SCREEN_HEIGHT);
    SDL_SetWindowSize(gWindow, userWidth - 100, userHeight - 100);
    renderDraw(true, false);
    SDL_RenderPresent(gRenderer);

    if (gWindow == NULL || gRenderer == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    }
  }
  return success;
}

// just sets the color or clears
void renderDraw(bool clear, bool on) {
  if (!on || clear) {
    SDL_SetRenderDrawColor(gRenderer, 170, 153, 153, 255);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 34, 0, 0, 255);
  }
  if (clear) {
    SDL_RenderClear(gRenderer);
  }
}

void close() {
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
}
