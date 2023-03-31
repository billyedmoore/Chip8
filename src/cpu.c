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
    // 0x00EE: Return from subroutine.
    case 0x00EE:
      // Set the PC to the value from the top of the stack.
      sys->PC = sys->Stack[sys->StackPointer];
      // Clear the top of the stack.
      sys->Stack[sys->StackPointer] = 0;
      // Decrement StackPointer.
      sys->StackPointer--;

      break;
    }
    break;

  // 0x1NNN: Jump to NNN.
  case 0x1000:
    sys->PC = (opcode & 0x0FFF);
    break;

  // 0x2NNN: Call subroutine.
  case 0x2000:
    // Push the current value of the PC to the stack.
    sys->Stack[sys->StackPointer] = sys->PC;
    sys->StackPointer++;
    // If pointing to outside of stack throw error.
    if (sys->StackPointer > 16) {
      sys->Quit = 1;
      printf("Stack Depth Exceeded.\n");
    }
    // Set the PC to NNN.
    sys->PC = (opcode & 0x0FFF);
    break;

  // 0x3XNN: Skip if VX == NN.
  case 0x3000:
    if (sys->V[X] == (opcode & 0x00FF)) {
      // Skip an instruction.
      sys->PC += 2;
    }
    break;

  // 0x4XNN: Skip if VX != NN.
  case 0x4000:
    if (sys->V[X] != (opcode & 0x00FF)) {
      // Skip an instruction.
      sys->PC += 2;
    }
    break;

  // 0x5XY0: Skip if VX == VY.
  case 0x5000:
    // If VX == VY.
    if (sys->V[X] == sys->V[Y]) {
      // Skip an instruction.
      sys->PC += 2;
    }
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

  case 0x8000:
    switch (opcode & 0x000F) {
    // 0x8XY0: Set VX to VY.
    case 0x0000:
      // Set VX to VY.
      sys->V[X] = sys->V[Y];
      break;

    // 0x8XY1: Binary or between VX and VY -> VX.
    case 0x0001:
      sys->V[X] = sys->V[X] | sys->V[Y];
      break;

    // 0x8XY2: Binary and between VX and VY -> VX.
    case 0x0002:
      sys->V[X] = sys->V[X] & sys->V[Y];
      break;

    // 0x8XY3: Bitwise xor between VX and VY -> VX.
    case 0x0003:
      sys->V[X] = sys->V[X] ^ sys->V[Y];
      break;

    // 0x8XY4: Add VX to VY and store in VX.
    case 0x0004:
      sys->V[X] = sys->V[X] + sys->V[Y];
      break;

    // 0x8XY5: Subtract VY from VX and store in VX.
    case 0x0005:
      // if VX>VY then set VF=1
      if (sys->V[X] > sys->V[Y]) {
        sys->V[0xF] = 1;
      }
      // else then set VF=0
      else {
        sys->V[0xF] = 0;
      }

      sys->V[X] = sys->V[X] - sys->V[Y];
      break;

    // 0x8XY6: Shift right 1. Set VF to 1 if the least significant bit is 1
    //         otherwise set to 0.
    // AMBIGUOUS - Alternately VX <- VY before shift.
    case 0x0006:
      // If the least significant bit is 1.
      if (sys->V[X] & 1) {
        sys->V[0xF] = 1;
      }
      // Else set VF = 0.
      else {
        sys->V[0xF] = 0;
      }

      sys->V[X] = sys->V[X] >> 1;
      break;

    // 0x8XY7: Subtract VX from VY and store in VX.
    case 0x0007:
      // If VY>VX then set VF=1.
      if (sys->V[Y] > sys->V[X]) {
        sys->V[0xF] = 1;
      }
      // Else set VF=0.
      else {
        sys->V[0xF] = 0;
      }

      sys->V[X] = sys->V[Y] - sys->V[X];
      break;
    // 0x8XYE: Shift left 1. Set VF to 1 if the most significant bit is 1
    //         otherwise set to 0.
    // AMBIGUOUS - Alternately VX <- VY before shift.
    case 0x000E:
      // If the most significant bit is 1. Hence if VX & 10000000 != 0.
      if (sys->V[X] & 0x80) {
        sys->V[0xF] = 1;
      }
      // Else set VF = 0.
      else {
        sys->V[0xF] = 0;
      }

      sys->V[X] = sys->V[X] << 1;
      break;
    }
    break;

  // 0x9XY0: Skip if VX != VY.
  case 0x9000:
    if (sys->V[X] != sys->V[Y]) {
      // Skip an instruction.
      sys->PC += 2;
    }
    break;

  // 0xANNN: Set I register to NNN.
  case 0xA000:
    sys->I = (opcode & 0x0FFF);
    break;

  // 0xBNNN: Set PC <- NNN + V0.
  // AMBIGUOUS - Alternately PC <- XNN + VX.
  case 0xB000:
    sys->PC = sys->V[0] + (opcode & 0x0FFF);
    break;

  // 0xCXNN: Generate a random 8 bit number, r. VX <- r & NN.
  case 0xC000: {
    uint8_t r = rand() % 256; // Random num between 0 and 255
    sys->V[X] = r & (opcode & 0x00FF);

    break;
  }

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
  case 0xE000:
    switch (opcode & 0xF0FF) {
    // 0xEX9E: Skip if key VX is pressed.
    case 0xE09E:
      if (sys->Keyboard[sys->V[X]]) {
        sys->PC += 2;
      }
      break;

    // 0xEXA1: Skip if key VC is not pressed.
    case 0xE0A1:
      if (!sys->Keyboard[sys->V[X]]) {
        sys->PC += 2;
      }
      break;
    }
    break;
  case 0xF000:
    switch (opcode & 0xF0FF) {
    // 0xFX07:  Set VX = DelayTimer.
    case 0xF007:
      sys->V[X] = sys->DelayTimer;
      break;

    // 0xFX0A: Wait until a key is pressed the store the value of that
    //         key in VX. Decrements the PC and runs again if no key is
    //         pressed.
    case 0xF00A: {
      int key_pressed = 0;
      // For key on keyboard.
      for (int i = 0; i < 16; i++) {
        // If key pressed.
        if (sys->Keyboard[i]) {
          // Set VX to key.
          sys->V[X] = i;
          key_pressed = 1;
        }
      }
      // If not pressed decrement the PC to run again.
      if (!key_pressed) {
        sys->PC -= 2;
      }
      break;
    }
    // 0xFX15:  Set DelayTimer = VX.
    case 0xF015:
      sys->DelayTimer = sys->V[X];
      break;

    // 0xFX18: Set SoundTimer = VX.
    case 0xF018:
      sys->SoundTimer = sys->V[X];
      break;

    // 0xFX1E: Set I=VX+I.
    case 0xF01E:
      sys->I = sys->I + sys->V[X];
      break;

    // 0xFX29: Set I to the location of sprite in memory;
    case 0xF029:
      // Set I to the value of the font for the specified char.
      sys->I = 0x050 + sys->V[X];
      break;

    // 0xFX33: Store VX as 3 digits in BDC at addresses I, I+1 and I+2.
    case 0xF033: {
      // Get the number from VX.
      int8_t numb = sys->V[X];

      // The first digit.
      int8_t dig_one = numb % 10;
      // The second digit.
      int8_t dig_two = numb % 100 - dig_one;
      // The third digit.
      int8_t dig_three = numb - numb % 100;

      // Store in memory.
      sys->Memory[sys->I] = dig_one;
      sys->Memory[sys->I + 1] = dig_two;
      sys->Memory[sys->I + 2] = dig_three;
      break;
    }

    // 0xFX55: Read V0->VX into memory starting at memory address I.
    case 0xF055: {
      int index = sys->I;
      for (int i = 0; i < X; i++) {
        sys->Memory[index] = sys->V[i];
        i++;
      }
      break;
    }

    // 0xFX65: Read from memory starting at address I into V0->X.
    case 0xF065: {
      int index = sys->I;
      for (int i = 0; i < X; i++) {
        sys->V[i] = sys->Memory[index];
        i++;
      }
      break;
    }
    }
    break;
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

/**
 * Decrement delay and sound timers if they are > 0. Should runat a rate of
 * ~60hz.
 *
 * Parameters:
 *  Chip8* sys: The system state.
 */
void decrementTimers(Chip8 *sys) {
  if (sys->DelayTimer > 0) {
    sys->DelayTimer--;
  }
  if (sys->SoundTimer > 0) {
    sys->SoundTimer--;
  }
}
