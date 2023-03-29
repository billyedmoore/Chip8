#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include "cpu.h"

void displayInit(void);
void displayQuit(void);
void draw(Chip8 *sys);
void printDisplay(Chip8 *sys);
void handleEvents(Chip8 *sys);
void printKeyboard(Chip8 *sys);
#endif
