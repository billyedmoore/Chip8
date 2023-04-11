/**
 * Functions to represent the cpu.
 *
 * Approx functions:
 *  system* system_init() -> Initialise the system.
 *  int load_rom(char* rom) -> Whether the cpu loaded successfully.
 *  void cycle(system* sys) -> Cycle the system.
 */
#include "cpu.h"
#include "logging.h"

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

  simpleLog(WARN, "Created a new Chip8 instance.\n");
  return sys;
}

/**
 * Make one cycle of the fetch-decode-execute cycle.
 */
void cycleSystem(Chip8 *sys) {

  // Fetch the operation from memory, 16 bit made up from two memory locations.
  uint16_t opcode = (sys->Memory[sys->PC] << 8) | sys->Memory[sys->PC + 1];

  // Increment the PC.
  // sys->PC += 2;

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
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Cleared the display.\n", opcode);
      break;
    // 0x00EE: Return from subroutine.
    case 0x00EE:
      // Set the PC to the value from the top of the stack.
      sys->PC = sys->Stack[sys->StackPointer];
      // Decrement StackPointer.
      sys->StackPointer--;
      sys->PC += 2;
      simpleLog(INFO, "%#04X - Returned from subroutine.(PC=%#03X and SP=%i)\n",
                opcode, sys->PC, sys->StackPointer);
      break;
    default:
      simpleLog(WARN, "Unknown opcode: %x.\n", opcode);
      break;
    }
    break;

  // 0x1NNN: Jump to NNN.
  case 0x1000:
    sys->PC = (opcode & 0x0FFF);
    simpleLog(INFO, "%#04X - Jumped to NNN=%#03X PC=%#03X.\n", opcode,
              (opcode & 0x0FFF), sys->PC);
    break;

  // 0x2NNN: Call subroutine.
  case 0x2000:
    // Increment the stack pointer.
    sys->StackPointer++;
    // Push the current value of the PC to the stack.
    sys->Stack[sys->StackPointer] = sys->PC;
    // If pointing to outside of stack throw error.
    if (sys->StackPointer >= 64) {
      sys->Quit = 1;
      simpleLog(WARN, "Stack Depth Exceeded.\n");
    }
    // Set the PC to NNN.
    sys->PC = (opcode & 0x0FFF);
    // sys->PC += 2;
    simpleLog(
        INFO,
        "%#04X - Called a subroutine added current PC to the stack. Jumped "
        "to %#03X. SP is %i.\n",
        opcode, sys->PC, sys->StackPointer);
    break;

  // 0x3XNN: Skip if VX == NN.
  case 0x3000:
    if (sys->V[X] == (opcode & 0x00FF)) {
      // Skip an instruction.
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Skipped as (V%X=%#04X) == (NN=%#04X)\n", opcode,
                X, sys->V[X], (opcode & 0x00FF));
    } else {
      simpleLog(INFO, "%#04X - Not Skipped as (V%X=%#04X) != (NN=%#04X)\n",
                opcode, X, sys->PC, (opcode & 0x00FF));
    }
    sys->PC += 2;
    break;

  // 0x4XNN: Skip if VX != NN.
  case 0x4000:
    if (sys->V[X] != (opcode & 0x00FF)) {
      // Skip an instruction.
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Skipped as (V%X=%#04X) != (NN=%#04X)\n", opcode,
                X, sys->V[X], (opcode & 0x00FF));
    } else {
      simpleLog(INFO, "%#06X - Not skipped as (V%X=%#04X) == (NN=%#04X)\n",
                opcode, X, sys->V[X], (opcode & 0x00FF));
    }
    sys->PC += 2;
    break;

  // 0x5XY0: Skip if VX == VY.
  case 0x5000:
    // If VX == VY.
    if (sys->V[X] == sys->V[Y]) {
      // Skip an instruction.
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Skipped as (V%X=%#04X) == (VY=%#04X)\n", opcode,
                X, sys->V[X], sys->V[Y]);
    }
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Not skipped as (V%X=%#04X) != (VY=%#04X)\n",
              opcode, X, sys->V[X], (opcode & 0x00FF));
    break;

  // 0x6XNN: Set register X.
  case 0x6000:
    sys->V[X] = (opcode & 0x00FF);
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Set V%X = %#04X\n", opcode, X, sys->V[X]);
    break;

  // 0x7XNN: Add NN to register X.
  case 0x7000: {
    uint8_t NN = (opcode & 0x00FF);
    uint8_t VX = sys->V[X];
    sys->V[X] += NN;
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Set V%X = V%X(%#04i) + NN(%#04i) = %#04i\n",
              opcode, X, X, VX, NN, sys->V[X]);
    break;
  }

  case 0x8000:
    switch (opcode & 0x000F) {
    // 0x8XY0: Set VX to VY.
    case 0x0000:
      // Set VX to VY.
      sys->V[X] = sys->V[Y];
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set V%X=VY(%#04X)\n", opcode, X, sys->V[X]);
      break;

    // 0x8XY1: Binary or between VX and VY -> VX.
    case 0x0001: {
      int8_t VX = sys->V[X];
      int8_t VY = sys->V[Y];
      sys->V[X] = sys->V[X] | sys->V[Y];
      sys->V[0xF] = 0;
      sys->PC += 2;
      simpleLog(INFO,
                "%#06X - Binary OR. V%X = V%X(%#04X) | V%X(%#04X) = %#04X\n",
                opcode, X, X, VX, Y, VY, sys->V[X]);
      break;
    }

    // 0x8XY2: Binary and between VX and VY -> VX.
    case 0x0002: {
      int8_t VX = sys->V[X];
      int8_t VY = sys->V[Y];
      sys->V[X] = sys->V[X] & sys->V[Y];
      sys->V[0xF] = 0;
      sys->PC += 2;
      simpleLog(INFO,
                "%#06X - Binary AND. V%X = V%X(%#04X) & V%X(%#04X) = %#04X\n",
                opcode, X, X, VX, Y, VY, sys->V[X]);
      break;
    }

    // 0x8XY3: Bitwise xor between VX and VY -> VX.
    case 0x0003: {
      int8_t VX = sys->V[X];
      int8_t VY = sys->V[Y];
      sys->V[X] = sys->V[X] ^ sys->V[Y];
      sys->V[0xF] = 0;
      sys->PC += 2;
      simpleLog(INFO,
                "%#06X - Binary XOR. V%X = V%X(%#04X) ^ V%X(%#04X) = %#04X\n",
                opcode, X, X, VX, Y, VY, sys->V[X]);
      break;
    }

    // 0x8XY4: Add VX to VY and store in VX.
    case 0x0004: {
      uint8_t VX = sys->V[X];
      uint8_t VY = sys->V[Y];
      int result = sys->V[X] + sys->V[Y];
      sys->V[0xF] = (result > 0xFF);
      sys->V[X] = result;
      sys->PC += 2;
      simpleLog(
          INFO,
          "%#06X - Add V%X and V%X. V%X = V%X(%#04X) + V%X(%#04X) = %#04X\n",
          opcode, X, Y, X, X, VX, Y, VY, sys->V[X]);
      break;
    }

    // 0x8XY5: Subtract VY from VX and store in VX.
    case 0x0005: {
      uint8_t VX = sys->V[X];
      uint8_t VY = sys->V[Y];

      sys->V[X] = sys->V[X] - sys->V[Y];
      sys->V[0xF] = sys->V[X] > sys->V[Y];
      sys->PC += 2;
      simpleLog(
          INFO,
          "%#06X - Subtract V%X from V%X. V%X = V%X(%#04X) - V%X(%#04X) = "
          "%#04X. VF = %i\n",
          opcode, Y, X, X, X, VX, Y, VY, sys->V[X], sys->V[0xF]);

      break;
    }

    // 0x8XY6: Shift right 1. Set VF to 1 if the least significant bit is 1
    //         otherwise set to 0.
    // AMBIGUOUS - Alternately VX <- VY before shift.
    case 0x0006: {
      // If the least significant bit is 1.
      sys->V[X] = sys->V[Y];
      uint8_t VX = sys->V[X];
      uint8_t VF = sys->V[X] & 0x1;
      sys->V[X] = sys->V[X] >> 1;
      sys->V[0xF] = VF;
      sys->PC += 2;
      simpleLog(
          INFO,
          "%#06X - Shift V%X one bit right. V%X = V%X(%#04X) >> 1 = %#04X. "
          "VF = %i.\n",
          opcode, X, X, X, VX, sys->V[X], sys->V[0xF]);
      break;
    }

    // 0x8XY7: Subtract VX from VY and store in VX.
    case 0x0007: {
      uint8_t VX = sys->V[X];
      uint8_t VY = sys->V[Y];
      sys->V[X] = sys->V[Y] - sys->V[X];
      sys->V[0xF] = (sys->V[Y] > sys->V[X]);
      sys->PC += 2;
      simpleLog(
          INFO,
          "%#06X - Subtract V%X from V%X. V%X = V%X(%#04X) - V%X(%#04X) = "
          "%#04X. VF = %i\n",
          opcode, X, Y, X, X, VX, Y, VY, sys->V[X], sys->V[0xF]);
      break;
    }

    // 0x8XYE: Shift left 1. Set VF to 1 if the most significant bit is 1
    //         otherwise set to 0.
    // AMBIGUOUS - Alternately VX <- VY before shift.
    case 0x000E: {
      sys->V[X] = sys->V[Y];
      uint8_t VX = sys->V[X];
      // If the most significant bit is 1. Hence if VX & 10000000 != 0.
      uint8_t VF = (sys->V[X] >> 7) & 1;
      sys->V[X] = sys->V[X] << 1;
      sys->V[0xF] = VF;
      sys->PC += 2;
      simpleLog(
          INFO,
          "%#06X - Shift V%X one bit right. V%X = V%X(%#04X) >> 1 = %#04X. "
          "VF = %i.\n",
          opcode, X, X, X, VX, sys->V[X], sys->V[0xF]);
      break;
    }

    default:
      simpleLog(WARN, "%#06X - Unknown opcode: .\n", opcode);
      break;
    }
    break;

  // 0x9XY0: Skip if VX != VY.
  case 0x9000:
    if (sys->V[X] != sys->V[Y]) {
      // Skip an instruction.
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Skipped as (V%X=%#04X) != (V%X=%#04X)\n", opcode,
                X, sys->V[X], Y, sys->V[Y]);
    } else {
      simpleLog(INFO, "%#06X - Not skipped as (V%X=%#04X) == (V%X=%#04X)\n",
                opcode, X, sys->V[X], Y, sys->V[Y]);
    }
    sys->PC += 2;
    break;

  // 0xANNN: Set I register to NNN.
  case 0xA000:
    sys->I = (opcode & 0x0FFF);
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Set I to %#03X\n", opcode, (opcode & 0x0FFF));
    break;

  // 0xBNNN: Set PC <- NNN + V0.
  // AMBIGUOUS - Alternately PC <- XNN + VX.
  case 0xB000:
    sys->PC = sys->V[0] + (opcode & 0x0FFF);
    // sys->PC += 2;
    simpleLog(INFO, "%#06X - Set PC = %#05X + V0(%#04X) = %#04X\n", opcode,
              opcode & 0x0FFF, sys->V[0], sys->PC);
    break;

  // 0xCXNN: Generate a random 8 bit number, r. VX <- r & NN.
  case 0xC000: {
    uint8_t r = rand() % 256; // Random num between 0 and 255
    sys->V[X] = r & (opcode & 0x00FF);
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Set V%X = rand(%#04X) & %#04X = %#04X\n", opcode,
              X, r, opcode & 0x00FF, sys->V[X]);
    break;
  }

  // 0xDXYN: Draw to display.
  case 0xD000: {
    int8_t x = sys->V[X] % 64;
    int8_t y = sys->V[Y] % 32;
    int8_t N = (opcode & 0x000F);

    sys->V[0xF] = 0;

    // For y in height
    for (int lineCount = 0; lineCount < N; lineCount++) {
      int8_t spriteBlock = sys->Memory[(sys->I) + lineCount];

      // From most to least significant bit.
      int x_moved = 0;
      for (int xCount = 7; xCount >= 0; xCount--) {
        // Get the relevant bit.
        int8_t px = (spriteBlock >> xCount) & 1;
        // Get the index of the relevant pixel
        int pxIndex = x + (y * 64);

        if (px) {
          // If 1 in sprite and on display then set VF.
          if (sys->Display[pxIndex]) {
            sys->V[0xF] = 1;
          }
          // If 1 in sprite and not on display then set display px to 1.
          sys->Display[pxIndex] ^= 1;
        }
        x_moved++;
        x++;

        if (x > 63) {
          break;
        }
      }
      x -= x_moved;
      y++;

      if (y > 31) {
        break;
      }
    }
    sys->PC += 2;
    simpleLog(INFO, "%#06X - Drawn to display.(VX=%#04X, VY=%#04X)\n", opcode,
              sys->V[X], sys->V[Y]);
    break;
  }

  case 0xE000:
    switch (opcode & 0xF0FF) {
    // 0xEX9E: Skip if key VX is pressed.
    case 0xE09E:
      if (sys->Keyboard[sys->V[X]]) {
        sys->PC += 2;
        simpleLog(INFO, "%#06X - Skipped as key V%X=%#04X is pressed.\n",
                  opcode, X, sys->V[X]);
      } else {
        simpleLog(INFO,
                  "%#06X - Not skipped as key V%X=%#04X is not pressed.\n",
                  opcode, X, sys->V[X]);
      }
      sys->PC += 2;
      break;

    // 0xEXA1: Skip if key VC is not pressed.
    case 0xE0A1:
      if (!sys->Keyboard[sys->V[X]]) {
        sys->PC += 2;
        simpleLog(INFO, "%#06X - Skipped as key V%X=%#04X is not pressed.\n",
                  opcode, X, sys->V[X]);
      }
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Not skipped as key V%X=%#04X is pressed.\n",
                opcode, X, sys->V[X]);
      break;

    default:
      printf("Unknown opcode: %X.\n", opcode);
      break;
    }
    break;

  case 0xF000:
    switch (opcode & 0x00FF) {
    // 0xFX07:  Set VX = DelayTimer.
    case 0x0007:
      sys->V[X] = sys->DelayTimer;
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set V%X= delay timer(%#04X).\n", opcode, X,
                sys->V[X]);
      break;

    // 0xFX0A: Wait until a key is pressed the store the value of that
    //         key in VX. Decrements the PC and runs again if no key is
    //         pressed.
    case 0x000A: {
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
      // Only increment the PC if key_pressed.
      if (key_pressed) {
        sys->PC += 2;
        simpleLog(INFO, "%#06X - %#04X key pressed.\n", opcode);
      } else {
        simpleLog(INFO, "%#06X - Waiting for key to be pressed.\n", opcode);
      }
      break;
    }
    // 0xFX15:  Set DelayTimer = VX.
    case 0x0015:
      sys->DelayTimer = sys->V[X];
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set delay timer = V%X(%#04X) \n", opcode, X,
                sys->V[X]);
      break;

    // 0xFX18: Set SoundTimer = VX.
    case 0x0018:
      sys->SoundTimer = sys->V[X];
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set sound timer = V%X(%#04X) \n", opcode, X,
                sys->V[X]);
      break;

    // 0xFX1E: Set I=VX+I.
    case 0x001E: {
      int8_t I = sys->I;
      sys->I = sys->I + sys->V[X];
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set I = V%X(%#04X) + I(%#04X) = %#04X\n", opcode,
                X, sys->V[X], I, sys->I);
      break;
    }

    // 0xFX29: Set I to the location of sprite in memory;
    case 0x0029:
      // Set I to the value of the font for the specified char.
      sys->I = 0x050 + (sys->V[X] * 5);
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Set I = location of char %i = %#04X\n", opcode,
                sys->V[X], sys->I);
      break;

    // 0xFX33: Store VX as 3 digits in BDC at addresses I, I+1 and I+2.
    case 0x0033: {
      // Get the number from VX.
      uint8_t numb = sys->V[X];
      // Store in memory.
      sys->Memory[sys->I] = (numb % 1000) / 100;
      sys->Memory[sys->I + 1] = (numb % 100) / 10;
      sys->Memory[sys->I + 2] = (numb % 10);
      sys->PC += 2;
      simpleLog(INFO,
                "%#06X - V%X(%X) -> [%#04x] = %i, [%#04x] = %i, [%#04x] = %i\n",
                opcode, X, sys->V[X], sys->I, (numb % 1000) / 100, sys->I + 1,
                (numb % 100) / 10, sys->I + 2, (numb % 10));
      break;
    }

    // 0xFX55: Read V0->VX into memory starting at memory address I.
    case 0x0055: {
      int index = sys->I;
      for (int i = 0; i <= X; i++) {
        sys->Memory[index] = sys->V[i];
        index++;
      }
      sys->I++;
      sys->PC += 2;
      simpleLog(INFO,
                "%#06X - Read from V0 -> V%X into memory starting at %#06X\n",
                opcode, X, sys->I);
      break;
    }

    // 0xFX65: Read from memory starting at address I into V0->X.
    case 0x0065: {
      int index = sys->I;
      for (int i = 0; i <= X; i++) {
        sys->V[i] = sys->Memory[index];
        index++;
      }
      sys->I++;
      sys->PC += 2;
      simpleLog(INFO, "%#06X - Read memory into V0 -> V%X starting at %#06X\n",
                opcode, X, sys->I);
      break;
    }
    default:
      simpleLog(WARN, "Unknown opcode: %#06X.\n", opcode);
      break;
    }
    break;
  default:
    simpleLog(WARN, "Unknown opcode: %#06X.\n", opcode);
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
    return;
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
