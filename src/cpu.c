/**
 * Functions to represent the cpu.
 *
 * Approx functions:
 *  system* system_init() -> Initialise the system.
 *  int load_rom(char* rom) -> Whether the cpu loaded successfully.
 *  void cycle(system* sys) -> Cycle the system.
 */
#include "cpu.h"

#include <stdlib.h> // for malloc
#include <string.h> // for memset

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

  // Set the display to blank
  memset(sys->Display, 0, sizeof(sys->Display));
  
  // Set the timers
  sys->DelayTimer = 0;
  sys->SoundTimer = 0;

  return sys;
}
