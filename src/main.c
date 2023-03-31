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
#include <time.h>

// The ratio of cycle hrz to the hrz timers should be run at (60hz).
#define TIMERS_RATIO 5

int main(int argc, char **argv) {

  // If wrong number of args passed exit.
  if (argc != 2) {
    printf("You passed the incorrect number of args.\n");
    printf("Usage: ./a.out path/to/game.ch8\n");
    exit(1);
  }
  // Seed random.
  srand(time(NULL));

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

  // Used to determine the number of loops per decrement of timers.
  int timers_count = 0;
  while (1) {

    cycleSystem(sys);
    draw(sys);
    handleEvents(sys);

    if (sys->Quit) {
      printf("Quiting\n");
      break;
    }

    // Timers should be decremented every 60hz. This is achieved by storing an
    // approximate ratio of the system clock speed and 60hz.
    if (timers_count == TIMERS_RATIO) {
      timers_count = 0;     // Reset the timers count.
      decrementTimers(sys); // Decrement the timers.
    }

    timers_count++;
  }

  // Free up memory.
  free(sys);
  displayQuit();
}
