/**
 * Functions to draw to the screen and receive keypresses.
 *
 * Approx functions:
 *  void draw(system* sys) ->
 *  peripherals_init(SDL_Window**, SDL_Renderer**, SDL_Texture)
 *  draw(SDL_Renderer*, SDL_Texture*)
 *  handle_keypr(????)
 */
#include "peripheral.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>

// To move potentially
SDL_Window *screen;
SDL_Renderer *renderer;

/**
 * Initialise the display window.
 */
void displayInit(void) {
  SDL_Init(SDL_INIT_VIDEO);

  screen = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 64 * 8, 32 * 8, 0);
  renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
}

/*
 * Quit SDL and destroy the screen and renderer.
 */
void displayQuit(void) {
  SDL_DestroyWindow(screen);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}

/**
 * Update the window to the current state of the system.
 * Parameters:
 *  Chip8* sys: the chip8 "system"
 */
void draw(Chip8 *sys) {
  // Clear the display
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // For pixel in system
  for (int x; x < 64; x++) {
    for (int y; y < 32; y++) {

      // if pixel is 1 draw rect
      if (sys->Display[x + (y * 64)]) {
        SDL_Rect rect;

        rect.x = x * 8;
        rect.y = y * 8;
        rect.w = 8;
        rect.h = 8;

        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }
}


int handleEvents(){
  SDL_Event event;

  if (SDL_PollEvent(&event)){
    if (event.type == SDL_QUIT)
      return 1;
  }

  return 0;

}
