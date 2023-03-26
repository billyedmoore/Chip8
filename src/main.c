/**
 * Should have the main loop.
 *
 * Each cycle should:
 *  cycle(system*)
 *  handle_keypr(system*)
 *  draw()
 */
#include "cpu.h"
#include "peripheral.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  Chip8 *sys = systemInit();
  displayInit();
  for (int i; i < 10; i++) {
    draw(sys);
  }
  free(sys);
}
