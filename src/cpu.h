#ifndef CPU_H
#define CPU_H

// For
#include <stdint.h>

typedef struct Chip8 {
  /**
   * General purpose registers: 16 8-bit general purpose variable registers
   * numbered 0->F commonly called V0->VF.
   *
   * VF is called the flag register and will be set to 0 and 1 by many
   * instructions.
   */
  uint8_t V[16];

  /**
   * The index register: 16 bit register used to point at locations in memory.
   */
  uint16_t I;

  /**
   * Program counter: stores the address of the current instruction in memory.
   */
  uint16_t PC;

  /**
   * The stack: used to to call subroutines/functions and return from them. Has
   * a max depth of 64.
   */
  uint16_t Stack[64];

  /**
   * The stack pointer: a pseudo register, points to the top of the stack.
   */
  uint8_t StackPointer;

  /**
   * Memory: 4096 bytes of memory
   *
   * 0x000 -> 0x1FF for the interpreter itself.
   * 0x200 -> 0xFFF to load the rom.
   */
  uint8_t Memory[4096];

  /**
   * Display: an array representing the pixels making up the display. Each
   * value is either 1 or 0.
   */
  uint8_t Display[64 * 32];

  /**
   * Delay Timer: decremented 60 times a second until it reaches 0.
   */
  uint8_t DelayTimer;

  /**
   * Sounder Timer: decremented 60 times a second until it reaches 0. Beeps
   * when > 0.
   */
  uint8_t SoundTimer;

  /**
   * Keyboard: A map of the state of keys, 0 representing not pressed and 1
   *           representing pressed
   *
   * The order of keys (left to right top to bottom): 1,2,3,4,Q,W,E,R,A,S,D,F,
   *                                                  Z,X,C,V
   */
  uint8_t Keyboard[16];

  int Quit;
  int FileNotFound;

} Chip8;

Chip8 *systemInit();
void cycleSystem(Chip8 *sys);
void decrementTimers(Chip8 *sys);
void loadRom(char *filePath, Chip8 *sys);

#endif
