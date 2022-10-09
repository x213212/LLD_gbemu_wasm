// #include <SDL2/SDL.h>
// #include <SDL2/SDL2_gfxPrimitives.h>
// #ifdef __EMSCRIPTEN__
// #include <emscripten.h>
// #endif

// SDL_Window *window;
// SDL_Renderer *renderer;

// SDL_Point center = {.x = 100, .y = 100};
// const int radius = 100;

// void redraw() {
//   SDL_SetRenderDrawColor(renderer, /* RGBA: black */ 0x00, 0x00, 0x00, 0xFF);
//   SDL_RenderClear(renderer);
//   filledCircleRGBA(renderer, center.x, center.y, radius,
//                    /* RGBA: green */ 0x00, 0x80, 0x00, 0xFF);
//   SDL_RenderPresent(renderer);
// }

// uint32_t ticksForNextKeyDown = 0;

// bool handle_events() {
//   SDL_Event event;
//   SDL_PollEvent(&event);
//   if (event.type == SDL_QUIT) {
//     return false;
//   }
//   if (event.type == SDL_KEYDOWN) {
//     uint32_t ticksNow = SDL_GetTicks();
//     if (SDL_TICKS_PASSED(ticksNow, ticksForNextKeyDown)) {
//       // Throttle keydown events for 10ms.
//       ticksForNextKeyDown = ticksNow + 10;
//       switch (event.key.keysym.sym) {
//         case SDLK_UP:
//           center.y -= 1;
//           break;
//         case SDLK_DOWN:
//           center.y += 1;
//           break;
//         case SDLK_RIGHT:
//           center.x += 1;
//           break;
//         case SDLK_LEFT:
//           center.x -= 1;
//           break;
//       }
//       redraw();
//     }
//   }
//   return true;
// }

// void run_main_loop() {
// #ifdef __EMSCRIPTEN__
//   emscripten_set_main_loop([]() { handle_events(); }, 0, true);
// #else
//   while (handle_events())
//     ;
// #endif
// }
 #include <unistd.h>
#include "./include/common.h"

int main() {
  return emu_run();
  // SDL_Init(SDL_INIT_VIDEO);

  // SDL_CreateWindowAndRenderer(300, 300, 0, &window, &renderer);

  // redraw();
  // run_main_loop();

  // SDL_DestroyRenderer(renderer);
  // SDL_DestroyWindow(window);

  // SDL_Quit();
}
