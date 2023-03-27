/**
 * Functions to represent the cpu.
 *
 * Approx functions:
 *  system* system_init() -> Initialise the system.
 *  int load_rom(char* rom) -> Whether the cpu loaded successfully.
 *  void cycle(system* sys) -> Cycle the system.
 */
#include "cpu.h"

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

  return sys;
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
int loadRom(char *filePath, Chip8 *sys) {
  FILE *fp = fopen(filePath, "rb");
  
  // If failed to open.
  if (fp == NULL) {
    return -1;
  }
  
  // Read the rom into memory starting at 0x200
  fread(sys->Memory+0x200,1,4096-0x200,fp);
  
  return 1;
}
