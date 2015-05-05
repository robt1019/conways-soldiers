#include "display.h"

// Sets up a window struct, including window, renderer and 'finished' boolean variable
void setup_SDL (window *w, int height, int width){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Problem with initializing SDL Video\n");
        SDL_Quit();
        exit(1);
    } 
    w->finished = 0;

    w->win= SDL_CreateWindow("Conway's Soldiers", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        if(w->win == NULL){
        printf("problem initializing window in setup_SDL function\n");
        SDL_Quit();
        exit(1);
    }

    w->renderer = SDL_CreateRenderer(w->win, -1, 0);
    if(w->renderer == NULL){
        printf("problem initializing renderer in setup_SDL function\n");
        SDL_Quit();
        exit(1);
    }

    // Set initial screen colour to black
    SDL_SetRenderDrawColor(w->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(w->renderer);
    SDL_RenderPresent(w->renderer);

}

void check_for_quit(window *w){
    SDL_Event event;
    while(SDL_PollEvent(&event)) {      
        switch (event.type){
        case SDL_QUIT:
            w->finished = 1;
       }
    }
}


// Filled Circle centred at (cx,cy) of radius r, see :
// http://content.gpwiki.org/index.php/SDL:Tutorials:Drawing_and_Filling_Circles
void RenderFillCircle(SDL_Renderer *rend, int cx, int cy, int r){

   for (double dy = 1; dy <= r; dy += 1.0) {
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        SDL_RenderDrawLine(rend, cx-dx, cy+r-dy, cx+dx, cy+r-dy);
        SDL_RenderDrawLine(rend, cx-dx, cy-r+dy, cx+dx, cy-r+dy);
   }

}

// Circle centred at (cx,cy) of radius r, see :
// http://content.gpwiki.org/index.php/SDL:Tutorials:Drawing_and_Filling_Circles
void RenderDrawCircle(SDL_Renderer *rend, int cx, int cy, int r){

   double dx, dy;
   dx = floor(sqrt((2.0 * r ) ));
   SDL_RenderDrawLine(rend, cx-dx, cy+r, cx+dx, cy+r);
   SDL_RenderDrawLine(rend, cx-dx, cy-r, cx+dx, cy-r);
   for (dy = 1; dy <= r; dy += 1.0) {
        dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        SDL_RenderDrawPoint(rend, cx+dx, cy+r-dy);
        SDL_RenderDrawPoint(rend, cx+dx, cy-r+dy);
        SDL_RenderDrawPoint(rend, cx-dx, cy+r-dy);
        SDL_RenderDrawPoint(rend, cx-dx, cy-r+dy);
   }

}
