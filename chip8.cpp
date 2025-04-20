// TODO VF incremented for every row that is past bot of screen

#include "chip8_lib.h"
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>

// Screen dimensions (not constant)
int SCREEN_WIDTH = 64;
int SCREEN_HEIGHT = 32;

// globals
// Window
SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
// to clear and redraw every frame
SDL_Renderer *gFiller = NULL;
const uint8_t *keyboardstate = SDL_GetKeyboardState(NULL);

std::unordered_map<int, uint8_t> keytable;

int tablekey[17];

enum quirkType chipType = CHIP;

int main(int argc, char *argv[]) {
  chip8 TheChip;
  std::ifstream gameFile;
  makeTablekeyKeytable();
  if (loadFont(&TheChip)) {
    if (importGame(argv[1], &gameFile, &TheChip)) {
      initChip(&TheChip);
      if (!init()) {
        printf("failed to init\n");
      } else {
        // Main loop flag
        bool quit = false;
        SDL_RenderClear(gFiller);
        const double performance_freq = (double)SDL_GetPerformanceFrequency();

        while (!TheChip.quit) {
          // true if a draw instruction is read
          Uint64 start = SDL_GetPerformanceCounter();

          if (TheChip.delay_timer > 0) {
            --TheChip.delay_timer;
          }
          if (TheChip.sound_timer > 0) {
            --TheChip.sound_timer;
          }

          loop(&TheChip);
          if (chipType == CHIP) {
            if (TheChip.clear) {
              TheChip.clear = false;
              renderDraw(true, false);
            }
            if (TheChip.render) {
              TheChip.render = false;
              SDL_RenderPresent(gRenderer);
            }
          } else {
            SDL_RenderPresent(gFiller);
            SDL_RenderPresent(gRenderer);
          }
          Uint64 end = SDL_GetPerformanceCounter();

          double timed = ((end - start) * 1000) / performance_freq;

          // printf("%f\n", timed);
          // for 60fps

          if (timed < 1000.00 / FRAMERATE) {
            SDL_Delay((Uint32)(1000.00 / FRAMERATE - timed));
          }
        }
      }
      close();
    }
  }

  return 0;
}

bool loadFont(chip8 *Chip) {
  uint8_t tempfont[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };
  uint8_t largefont[] = {
      //  large fonts
      0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C, // 0
      0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, // 1
      0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, // 2
      0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C, // 3
      0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, // 4
      0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C, // 5
      0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, // 6
      0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, // 7
      0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, // 8
      0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C  // 9
  };
  uint16_t inc = 0x50;
  for (auto x : tempfont) {
    Chip->memory[inc++] = x;
  }
  inc = 0xA0;
  for (auto x : largefont) {
    Chip->memory[inc++] = x;
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
  while ((*file).get((char &)x) && Chip->pc < 0x0FFE) {
    instruction = ((uint8_t)x);
    Chip->memory[Chip->pc++] = (uint8_t)x;
    // printf("%04X \n", Chip->pc);
  }
  std::cout << std::endl;
  Chip->pc = 0x200;
  file->close();
  return true;
}

void initChip(chip8 *Chip) {
  std::srand(std::time(0));
  for (int i = 0; i < 128 * 64; ++i) {
    Chip->video[i] = 0x00000000;
  }
  Chip->render = false;
  Chip->quit = false;
  Chip->clear = false;
  for (int i = 0; i < 16; i++) {
    Chip->registers[i] = 0;
  }
  switch (chipType) {
  case 0:
    Chip->speed = 10;
    break;
  case 1:
  case 2:
  case 3:
    Chip->speed = 50;
    break;
  }
}

void loop(chip8 *Chip) {

  int cycles = 0;
  while (cycles < Chip->speed) {
    cycles++;
    if (checkQuit()) {
      Chip->quit = true;
      cycles = Chip->speed;
    }

    // fetch
    // combine the 2 8 bit halves of the instruction
    Chip->opcode = ((uint16_t)Chip->memory[Chip->pc] << 8) |
                   ((uint16_t)Chip->memory[Chip->pc + 1]);
    Chip->pc += 2;

    //  decode
    uint8_t firstNumber = (Chip->opcode >> 8) & 0xF0;

    // printf("index: %04X\n", Chip->index);
    // printf("Full: %04X\n", Chip->opcode);

    switch (firstNumber) {
    case 0x00:
      switch (Chip->opcode >> 4) {
      case 0x000C:
        // scroll down N pixels
        if (SCREEN_HEIGHT == 64) {
          op_00CN(Chip);
        }
        break;
      }
      switch (Chip->opcode) {
      case 0x00E0:
        for (int i = 0; i < 8192; ++i) {
          Chip->video[i] = 0x00000000;
        }
        Chip->clear = true;
        if (chipType == CHIP) {
          cycles = Chip->speed;
        } else {
          renderDraw(true, false);
        }
        break;
      case 0x00EE:
        Chip->pc = Chip->stack.top();
        (Chip->stack).pop();
        break;
      case 0x00FB:
        // scroll right 4 pixels
        if (SCREEN_HEIGHT == 64) {
          op_00FB(Chip);
        }
        break;
      case 0x00FC:
        // scroll left 4 pixels
        if (SCREEN_HEIGHT == 64) {
          op_00FC(Chip);
        }
        break;
      case 0x00FD:
        switch (chipType) {
        case (0):
          break;
        case (1):
        case (2):
        case (3):
          close();
          cycles = Chip->speed;
          Chip->quit = true;
          break;
        }
        break;
      case 0x00FE:
        // lores toggle
        switch (chipType) {
        case (0):
          break;
        case (1):
        case (2):
        case (3):
          SCREEN_HEIGHT = 32;
          SCREEN_WIDTH = 64;
          resize();
          break;
        }
        break;
      case 0x00FF:
        // hires toggle
        switch (chipType) {
        case (0):
          break;
        case (1):
        case (2):
        case (3):
          SCREEN_HEIGHT = 64;
          SCREEN_WIDTH = 128;
          resize();
          break;
        }
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
      op_3NNN(Chip);
      break;

    case 0x40:
      // if VX != NN in 4XNN skip 1 instruction
      op_4XNN(Chip);
      break;

    case 0x50:
      // if VX == VY in 5XY0 skip 1 instruction
      op_5XY0(Chip);

      break;

    case 0x60:
      // 6XNN
      op_6XNN(Chip);
      break;

    case 0x70:
      // 7XNN
      op_7XNN(Chip);
      break;

    case 0x80:
      op_8NNN(Chip);
      break;
    case 0x90:
      // if VX != VY in 9XY0 skip 1 instruction
      op_9XY0(Chip);
      break;

    case 0xA0:
      Chip->index = Chip->opcode & 0x0FFF;
      break;

    case 0xB0:
      // BNNN
      op_BNNN(Chip);
      break;

    case 0xC0:
      // CXNN
      op_CXNN(Chip);
      break;
    case 0xD0:
      op_DXY0(Chip);
      Chip->render = true;
      if (chipType == CHIP) {
        cycles = Chip->speed;
      }
      break;

    case 0xE0:
      // has E0A1 and E09E
      op_ENNN(Chip);
      break;

    case 0xF0:
      switch ((uint8_t)Chip->opcode) {
      case 0x07:
        op_FX07(Chip);
        break;
      case 0x0A:
        // wait for keypress
        // FX0A
        if (op_FX0A(Chip)) {
          Chip->quit = true;
          cycles = Chip->speed;
        }
        break;
      case 0x15:
        op_FX15(Chip);
        break;
      case 0x18:
        op_FX18(Chip);
        break;
      case 0x1E:
        op_FX1E(Chip);
        break;
      case 0x29:
        op_FX29(Chip);
        // access font at VX
        break;
      case 0x30:
        op_FX30(Chip);
        break;
      case 0x33:
        op_FX33(Chip);
        // store bcd of VX at index, index+1, index+2
        break;
      case 0x55:
        op_FX55(Chip);
        // memory dump from V0 to VX inlcuding VX
        break;
      case 0x65:
        op_FX65(Chip);
        // Write from memeory V0 to VX inlcuding VX
        break;
      case 0x75:
        puts("save Flags");
        break;
      case 0x85:
        puts("load flags");
        break;
      }
      break;
    }
  }
}

bool checkQuit() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    if (e.type == SDL_QUIT) {
      return true;
    }
  }
  return false;
}

void makeTablekeyKeytable() {
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
    gFiller = SDL_CreateRenderer(gWindow, -1, 0);
    SDL_SetRenderDrawColor(gFiller, 34, 0, 0, 255);

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    float userWidth = DM.w;
    float userHeight = DM.h;
    SDL_RenderSetScale(gRenderer, (userWidth - 100) / SCREEN_WIDTH,
                       (userHeight - 100) / SCREEN_HEIGHT);
    SDL_SetWindowSize(gWindow, userWidth - 100, userHeight - 100);
    renderDraw(true, false);

    if (gWindow == NULL || gRenderer == NULL) {
      printf("Window could not be shown. %s\n", SDL_GetError());
      success = false;
    }
  }
  return success;
}

void resize() {
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  float userWidth = DM.w;
  float userHeight = DM.h;
  SDL_RenderSetScale(gRenderer, (userWidth - 100) / SCREEN_WIDTH,
                     (userHeight - 100) / SCREEN_HEIGHT);
  SDL_RenderSetScale(gFiller, (userWidth - 100) / SCREEN_WIDTH,
                     (userHeight - 100) / SCREEN_HEIGHT);
  renderDraw(true, false);
}

// sets the color or clears
void renderDraw(bool clear, bool on) {
  if (on) {
    SDL_SetRenderDrawColor(gRenderer, 34, 0, 0, 255);
  } else {
    SDL_SetRenderDrawColor(gRenderer, 170, 153, 153, 255);
  }
  if (clear) {
    SDL_RenderClear(gRenderer);
    SDL_RenderPresent(gRenderer);
  }
}

void redraw(chip8 *Chip) {
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
    if (Chip->video[i] == 0xFFFFFFFF) {
      renderDraw(false, true);
      SDL_RenderDrawPoint(gRenderer, i % SCREEN_WIDTH, i / SCREEN_WIDTH);
    } else {
      renderDraw(false, false);
      SDL_RenderDrawPoint(gRenderer, i % SCREEN_WIDTH, i / SCREEN_WIDTH);
    }
  }
  SDL_RenderPresent(gRenderer);
}

void close() {
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyRenderer(gFiller);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
}

void op_00CN(chip8 *Chip) {
  switch (chipType) {
  case (0):
    break;
  case (1):
  case (2):
  case (3):
    puts("Scroll down");
    int N = Chip->opcode & 0x000F;
    for (int i = SCREEN_HEIGHT * SCREEN_WIDTH - 1 - SCREEN_WIDTH * N; i >= 0;
         i--) {
      Chip->video[i + SCREEN_WIDTH * N] = Chip->video[i];
    }
    for (int i = 0; i < SCREEN_WIDTH * N; i++) {
      Chip->video[i] = 0x00000000;
    }
    renderDraw(true, false);
    redraw(Chip);
    break;
  }
}

void op_00FB(chip8 *Chip) {
  switch (chipType) {
  case (0):
    break;
  case (1):
  case (2):
  case (3):
    puts("Scroll right");
    for (int i = SCREEN_HEIGHT * SCREEN_WIDTH - 5; i >= 0; i--) {
      Chip->video[i + 4] = Chip->video[i];
      if (i % SCREEN_WIDTH < 4) {
        Chip->video[i] = 0x00000000;
      }
    }
    Chip->video[0] = 0x00000000;
    Chip->video[1] = 0x00000000;
    Chip->video[2] = 0x00000000;
    Chip->video[3] = 0x00000000;
    renderDraw(true, false);
    redraw(Chip);
    break;
  }
}

void op_00FC(chip8 *Chip) {
  switch (chipType) {
  case (0):
    break;
  case (1):
  case (2):
  case (3):
    puts("lefts croll");
    for (int i = 4; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
      Chip->video[i - 4] = Chip->video[i];
      if (i % SCREEN_WIDTH > SCREEN_WIDTH - 4) {
        Chip->video[i] = 0x00000000;
      }
    }
    Chip->video[SCREEN_WIDTH - 1] = 0x00000000;
    Chip->video[SCREEN_WIDTH - 2] = 0x00000000;
    Chip->video[SCREEN_WIDTH - 3] = 0x00000000;
    Chip->video[SCREEN_WIDTH - 4] = 0x00000000;
    renderDraw(true, false);
    redraw(Chip);
    break;
  }
}

void op_3NNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  if (*data1 == (uint8_t)(Chip->opcode & 0x00FF)) {
    Chip->pc += 2;
  }
}

void op_4XNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  if (*data1 != (uint8_t)(Chip->opcode & 0x00FF)) {
    Chip->pc += 2;
  }
}
void op_5XY0(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
  uint8_t *data2 = &(Chip->registers[regi2]);
  if (*data1 == *data2) {
    Chip->pc += 2;
  }
}

void op_6XNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  *data1 = (uint8_t)(Chip->opcode);
}

void op_7XNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  *data1 += (uint8_t)(Chip->opcode);
}

void op_8NNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
  uint8_t *data2 = &(Chip->registers[regi2]);
  bool carryout;
  bool underflow;
  bool overflow;
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
}

void op_9XY0(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
  uint8_t *data2 = &(Chip->registers[regi2]);
  if (*data1 != *data2) {
    Chip->pc += 2;
  }
}

void op_BNNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
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
}

void op_CXNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  *data1 = std::rand() % 256 & (uint8_t)Chip->opcode;
}

void op_DXY0(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  uint8_t regi2 = (uint8_t)((Chip->opcode & 0x00F0) >> 4);
  uint8_t *data2 = &(Chip->registers[regi2]);
  int N;
  uint8_t x;
  uint8_t y;
  bool left16;
  bool rowCollision;
  switch (chipType) {
  case 0:
    // draw command
    Chip->render = true;
    x = *data1 % SCREEN_WIDTH;
    y = *data2 % SCREEN_HEIGHT;
    // height
    N = (uint8_t)(Chip->opcode) & 0x0F;
    // printf("x: %X\n", x);

    // VF keeps track of collision in sprites
    Chip->registers[0x0F] = 0;
    //
    // printf("Drawing Sprite: %X at X: %X, Y: %X, Height: %X\n ", Chip->index,
    // x,
    //        y, N);

    for (int row = 0; row < N; row++) {
      if (row + y >= SCREEN_HEIGHT) {
        break;
      }
      uint8_t spriteRow = Chip->memory[Chip->index + row];
      // printf("Sprite Row: %X\n", spriteRow);

      for (int col = 0; col < 8; col++) {

        if (col + x >= SCREEN_WIDTH) {
          break;
        }

        uint32_t *screenPixel =
            &(Chip->video[((row + y) * SCREEN_WIDTH) + (col + x) - 1]);

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
        } else {
          if (*screenPixel == 0x00000000) {
            renderDraw(false, false);
          } else {
            renderDraw(false, true);
          }
        }
        SDL_RenderDrawPoint(gRenderer, (col + x), (row + y));
        //           puts("");
      }
    }
    break;
  case 1:
  case 2:
  case 3:
    // draw command
    x = *data1 % SCREEN_WIDTH;
    y = *data2 % SCREEN_HEIGHT;
    // height
    N = (int)(Chip->opcode) & 0x0F;
    // printf("x: %X\n", x);
    // VF keeps track of collision in sprites

    if (N == 0) {
      left16 = true;
    } else {
      left16 = false;
    }
    Chip->registers[0x0F] = 0;
    // printf("Drawing Sprite: %X at X: %X, Y: %X, Height: %X\n ", Chip->index,
    // x, y, N);

    if (left16) {
      // puts("BINTING BIG STPRITE");
      rowCollision = false;
      for (int row = 0; row < 32; row++) {
        if (row / 2 + y >= SCREEN_HEIGHT) {
          break;
        }
        // printf("%X\n", Chip->index + row);
        uint8_t spriteRow = Chip->memory[Chip->index + row];
        // printf("Sprite Row: %X\n", spriteRow);

        for (int col = 0; col < 8; col++) {
          if (col + x >= SCREEN_WIDTH) {
            break;
          }
          uint32_t *screenPixel =
              &(Chip->video[((row / 2 + y) * SCREEN_WIDTH) + (col + x) - 1]);

          uint8_t spriteCur = ((spriteRow >> (7 - col)) & 0x01);
          // printf("%d ", ((row + y) * SCREEN_WIDTH) + (col + x) - 1);
          // collision
          if (spriteCur) {
            if (*screenPixel == 0xFFFFFFFF) {
              *screenPixel = 0x00000000;
              rowCollision = true;
              renderDraw(false, false);
            } else {
              *screenPixel = 0xFFFFFFFF;
              renderDraw(false, true);
              // printf("drawing at %d, %d ", col + x, row + y);
            }
            SDL_RenderDrawPoint(gRenderer, (col + x), (row / 2 + y));
          } else {
            if (*screenPixel == 0x00000000) {
              renderDraw(false, false);
            } else {
              renderDraw(false, true);
            }
          }
          SDL_RenderDrawPoint(gRenderer, (col + x), (row / 2 + y));
        }
        // puts("");
        left16 = !left16;
        if (!left16) {
          x += 8;
        } else {
          if (rowCollision) {
            Chip->registers[0x0F] += 1;
            rowCollision = false;
          }
          x -= 8;
        }
      }
    } else {
      for (int row = 0; row < N; row++) {
        if (row + y >= SCREEN_HEIGHT) {
          break;
        }
        uint8_t spriteRow = Chip->memory[Chip->index + row];
        // printf("Sprite Row %02X:", spriteRow);

        for (int col = 0; col < 8; col++) {
          if (col + x >= SCREEN_WIDTH) {
            break;
          }

          uint32_t *screenPixel =
              &(Chip->video[((row + y) * SCREEN_WIDTH) + (col + x) - 1]);
          // printf("row: %d, y: %d, width: %d, col: %d, x: %d\n", row, y,
          //     SCREEN_WIDTH, col, x);
          // printf("%d\n", ((row + y) * SCREEN_WIDTH) + (col + x));
          uint8_t spriteCur = ((spriteRow >> (7 - col)) & 0x01);
          // printf("%X ", spriteCur);
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
          } else {
            if (*screenPixel == 0x00000000) {
              renderDraw(false, false);
            } else {
              renderDraw(false, true);
            }
          }
          SDL_RenderDrawPoint(gRenderer, (col + x), (row + y));
        }
        //  puts("");
      }
    }
    break;
  }
}

void op_ENNN(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
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
}

void op_FX07(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  *data1 = Chip->delay_timer;
}

bool op_FX0A(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  SDL_Event e;
  while (true) {

    std::chrono::system_clock::time_point presstimerstart =
        std::chrono::high_resolution_clock::now();
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        return true;
      }
      if (e.type == SDL_KEYUP) {
        if (keytable.find((int)e.key.keysym.scancode) != keytable.end()) {
          *data1 = keytable[(int)e.key.keysym.scancode];
          return false;
        }
      }
    }

    std::chrono::system_clock::time_point presstimerend =
        std::chrono::high_resolution_clock::now();
    long presswaittime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             presstimerend - presstimerstart)
                             .count();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000 / FRAMERATE - presswaittime));
    if (Chip->delay_timer > 0) {
      --Chip->delay_timer;
    }
    if (Chip->sound_timer > 0) {
      --Chip->sound_timer;
    }
  }
}

void op_FX15(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  Chip->delay_timer = *data1;
}

void op_FX18(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  Chip->sound_timer = *data1;
}

void op_FX1E(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  Chip->index += *data1;
  Chip->index = Chip->index & 0x0FFF;
}

void op_FX29(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  Chip->index = 0x50 + (*data1 & 0x0F) * 5;
}

void op_FX30(chip8 *Chip) {
  puts("large font");
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  Chip->index = 0xA0 + (*data1 & 0x0F) * 10;
}
void op_FX33(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
  uint8_t *data1 = &(Chip->registers[regi1]);
  int bcdint = *data1;
  for (int i = 0; i < 3; ++i) {
    Chip->memory[Chip->index + (2 - i)] = (uint8_t)(bcdint % 10);
    bcdint /= 10;
  }
}

void op_FX55(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
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
}

void op_FX65(chip8 *Chip) {
  uint8_t regi1 = (uint8_t)((Chip->opcode & 0x0F00) >> 8);
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
}
