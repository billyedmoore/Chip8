#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include "cpu.h"

void displayInit(void);
void displayQuit(void);
void draw(Chip8 *sys);
#endif
