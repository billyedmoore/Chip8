/**
 * Functions to represent the cpu.
 *
 * Approx functions:
 *  system* system_init() -> Initialise the system.
 *  int load_rom(char* rom) -> Whether the cpu loaded successfully.
 *  void cycle(system* sys) -> Cycle the system.
 */
#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memset

uint8_t font[] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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
                  0xF0, 0x80, 0xF0, 0x80, 0x80};

/**
 * Create a new system and initialise it.
 *
 * Returns:
 *  Chip* A pointer to the initialised system.
 */
Chip8 *systemInit() {
  Chip8 *sys = malloc(sizeof(Chip8));

  // Initialise V0 -> VF to 0.
  memset(sys->V, 0, sizeof(sys->V));

  // Set index register to 0.
  sys->I = 0;

  // Set Program Counter to the address of the next instruction.
  sys->PC = 0x200;

  // Initialise the stack to an array of 0s.
  memset(sys->Stack, 0, sizeof(sys->Stack));

  // Point at the top of the stack.
  sys->StackPointer = 0;

  // Set the memory to 0 for now
  memset(sys->Memory, 0, sizeof(sys->Memory));

  // Put the font in memory
  // Between 0x050->0x09F
  for (int i = 0; i < 80; i++) {
    sys->Memory[0x050 + i] = font[i];
  }

  // Set the display to blank
  memset(sys->Display, 0, sizeof(sys->Display));

  // Set the timers
  sys->DelayTimer = 0;
  sys->SoundTimer = 0;

  sys->Quit = 0;
  sys->FileNotFound = 0;

  return sys;
}

/**
 * Make one cycle of the fetch-decode-execute cycle.
 */
void cycleSystem(Chip8 *sys) {

  // Fetch the operation from memory, 16 bit made up from two memory locations.
  uint16_t opcode = (sys->Memory[sys->PC] << 8) | sys->Memory[sys->PC + 1];

  // Increment the PC.
  sys->PC += 2;

  // Get the X from some instructions e.g 0x3XNN
  uint16_t X = (opcode & 0x0F00) >> 8;
  // Get the Y from some instructions e.g 0x5XY0
  uint16_t Y = (opcode & 0x00F0) >> 4;

  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode) {
    case 0x00E0:
      // Clear the display.
      memset(sys->Display, 0, sizeof(sys->Display));
      break;
    case 0X00EE:
      // Subroutine?
      break;
    }
    break;

  // 0x1NNN: Jump to NNN.
  case 0x1000:
    sys->PC = (opcode & 0x0FFF);
    break;

  // 0x6XNN: Set register X.
  case 0x6000:
    sys->V[X] = (opcode & 0x00FF);
    break;
  // 0x7XNN: Add NN to register X.
  case 0x7000: {
    uint8_t NN = (opcode & 0x00FF);

    if ((sys->V[X] + NN) > 0xFF) {
      sys->V[X] = 0xFF;
    } else {
      sys->V[X] += NN;
    }
    break;
  }
  // 0xANNN: Set I register to NNN.
  case 0xA000:
    sys->I = (opcode & 0x0FFF);
    break;

  // 0xDXYN: Draw to display.
  case 0xD000: {

    int8_t x = sys->V[X] & 63;
    int8_t y = sys->V[Y] & 31;
    int8_t N = (opcode & 0x000F);

    sys->V[0xF] = 0;

    // For y in height
    for (int yCount = 0; yCount < N; yCount++) {
      int8_t spriteBlock = sys->Memory[(sys->I) + yCount];

      // From most to least significant bit.
      for (int xCount = 7; xCount >= 0; xCount--) {
        // Get the relevant bit.
        int8_t px = (spriteBlock >> xCount) & 1;
        // Get the index of the relevant pixel
        int pxIndex = x + (y * 64);

        // If 1 in sprite and on display then set VF.
        if (px & sys->Display[pxIndex]) {
          sys->V[0xF] = 1;
        }
        // If 1 in sprite and not on display then set display px to 1.
        else if (px & !sys->Display[pxIndex]) {
          sys->Display[pxIndex] = 1;
        }

        // If reach right edge of screen stop.
        if (x == 63) {
          break;
        }
        x++;
      }
      // If reach bottom of the screen.
      if (y == 31) {
        break;
      }
      // Reset the x to the start of the row.
      x -= 8;
      y++;
    }
    break;
  }
  }
}

/**
 * Load a rom into memory.
 *
 * Parameters:
 *  char* filePath: The path of the rom.
 *  Chip8* sys: The system whoes memory to use.
 * Returns:
 *  int: A status code, 1 for succesfully loaded -1 for not.
 */
void loadRom(char *filePath, Chip8 *sys) {
  FILE *fp = fopen(filePath, "rb");

  // If failed to open.
  if (fp == NULL) {
    sys->FileNotFound = 1;
  }

  // Read the rom into memory starting at 0x200
  fread(sys->Memory + 0x200, 1, 4096 - 0x200, fp);
}
