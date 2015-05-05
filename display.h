#include <SDL.h>
#include <math.h>

#define SDL_8BITCOLOUR 256

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif


// All info required for windows / renderer & event loop
struct window {
   SDL_bool finished;
   SDL_Window *win;
   SDL_Renderer *renderer;
};
typedef struct window window;

void setup_SDL (window *w, int height, int width);
void check_for_quit (window *w);
void RenderFillCircle (SDL_Renderer *rend, int cx, int cy, int r);
void RenderDrawCircle (SDL_Renderer *rend, int cx, int cy, int r);
