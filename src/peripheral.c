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
#include <SDL2/SDL_keyboard.h>
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
                            SDL_WINDOWPOS_CENTERED, 64 * 16, 32 * 16, 0);
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
  for (int x = 0; x < 64; x++) {
    for (int y = 0; y < 32; y++) {

      // printf("(%i,%i) - %i\n",x,y,sys->Display[x + (y * 64)]);
      // if pixel is 1 draw rect
      if (sys->Display[x + (y * 64)]) {
        SDL_Rect rect;

        rect.x = x * 16;
        rect.y = y * 16;
        rect.w = 16;
        rect.h = 16;

        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
      }
    }
  }
  SDL_RenderPresent(renderer);
}

/**
 * Debug function to print the display out to stdout.
 *
 * Parameters:
 *  Chip8* sys: the system state.
 */
void printDisplay(Chip8 *sys) {
  for (int i = 0; i < (64 * 32); i++) {
    if (i % 64 == 0) {
      printf("%i\n", sys->Display[i]);
    } else {
      printf("%i", sys->Display[i]);
    }
  }
  putchar('\n');
}

/**
 * Debug function to print the keyboard out to stdout.
 *
 * Parameters:
 *  Chip8* sys: the system state.
 */
void printKeyboard(Chip8 *sys){
  for(int y = 0; y < 4; y++){
    for (int x = 0; x < 4; x++){
      printf("%i ",sys->Keyboard[x+y*4]);
    }
    printf("\n");
  }}

SDL_Scancode keys[16] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

/**
 * Handle SDL events.
 *
 * Handle Quit event by setting sys->Quit to 1 (meaning true).
 * Handle keypress event by setting the relevant think in sys.
 *
 * Parameters:
 *  Chip8* sys: The current state of the system.
 */
void handleEvents(Chip8 *sys) {
  SDL_Event event;

  const Uint8 *keyState = SDL_GetKeyboardState(NULL);
  // If an event has happened
  if (SDL_PollEvent(&event)) {
    // If user quits set sys->Quit and return.
    if (SDL_QUIT == event.type){
      sys->Quit = 1;
      return;

      }
    // If escape key is pressed set sys->Quit and return.
    if (keyState[SDL_SCANCODE_ESCAPE]) {
      sys->Quit = 1;
      return;
    }
    
    // For each key in the defined key map (see sys->Keyboard) set the state
    // stored in sys to match the current state.
    for (int keyCode =0; keyCode < 16; keyCode ++){
      sys->Keyboard[keyCode] = keyState[keys[keyCode]];
    }
  }
}
