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

int main(int argc, char **argv) {

  // If wrong number of args passed exit.
  if (argc != 2) {
    printf("You passed the incorrect number of args.\n");
    printf("Usage: ./a.out path/to/game.ch8\n");
    exit(1);
  }

  // Initialise system.
  Chip8 *sys = systemInit();
  // Load rom.
  loadRom(argv[1], sys);

  // If rom not loaded.
  if (sys->FileNotFound) {
    printf("Couldn't load rom.");
    exit(1);
  }

  // Initialise a display.
  displayInit();

  while (1) {
    cycleSystem(sys);
    draw(sys);
    handleEvents(sys);

    if (sys->Quit) {
      printf("Quiting\n");
      break;
    }
  }

  // Free up memory.
  free(sys);
  displayQuit();
}
