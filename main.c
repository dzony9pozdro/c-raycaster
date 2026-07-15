#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#define MAP_W 16
#define MAP_H 9
#define CELL 64

typedef struct {
  double x, y;
} Vec2;
typedef struct {

  Vec2 pos;
  Vec2 dir;
  Vec2 vel;
  double deg;
} Camera;

Camera camera_default(void) {
  Camera c = {.pos = {.x = 0, .y = 0},
              .dir = {.x = 1, .y = 0},
              .vel = {.x = 0, .y = 0},
              .deg = 0};
  return c;
}

void draw_map(SDL_Renderer *renderer) {

  int map[MAP_H][MAP_W] = {
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
      {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
      {1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
      {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  };
  int px = 0, py = 0;

  for (int w = 0; w < MAP_W; w++) {
    for (int h = 0; h < MAP_H; h++) {

      px = CELL * w;
      py = CELL * h;
      if (map[h][w] == 1) {
        SDL_Rect r = {px, py, CELL, CELL};
        SDL_RenderFillRect(renderer, &r);
      }
    }
  }
}
void draw_player(SDL_Renderer *renderer, Camera *cam) {

  cam->pos.x += cam->vel.x;
  cam->pos.y += cam->vel.y;

  double line_length = 50; // sqrt(cam->dir.x * cam->dir.y) *

  cam->vel.x /= 2;
  cam->vel.y /= 2;

  fprintf(stderr, "pos: %.2f, %.2f deg: %.2f dir: %.2f, %.2f \n", cam->pos.x,
          cam->pos.y, cam->deg, cam->dir.x, cam->dir.y);
  SDL_Rect p = {cam->pos.x - CELL / 4.0, cam->pos.y - CELL / 4.0, CELL / 2,
                CELL / 2};
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &p);

  SDL_RenderDrawLine(renderer, cam->pos.x, cam->pos.y,
                     cam->pos.x + cam->dir.x * line_length,
                     cam->pos.y + cam->dir.y * line_length);
}

void turn(double direction, Camera *cam) {
    cam->deg += direction;                // accumulate total angle
    cam->dir.x = cos(cam->deg);           // rebuild dir FROM the total
    cam->dir.y = sin(cam->deg);
}


int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window =
      SDL_CreateWindow("raycaster", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 16 * CELL, 9 * CELL, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  bool running = true;

  Camera cam = camera_default();

  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  float acceleration = 11;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        running = false;
    }

    if (keys[SDL_SCANCODE_W] && cam.vel.y < 300) {
      cam.vel.y -= acceleration;
    }
    if (keys[SDL_SCANCODE_S] && cam.vel.y > -300) {
      cam.vel.y += acceleration;
    }
    if (keys[SDL_SCANCODE_A] && cam.vel.x > -300) {
      cam.vel.x -= acceleration;
    }
    if (keys[SDL_SCANCODE_D] && cam.vel.x < 300) {
      cam.vel.x += acceleration;
    }

    if (keys[SDL_SCANCODE_J]) {
      turn(-0.3, &cam);
    }

    if (keys[SDL_SCANCODE_K]) {
      turn(0.3, &cam);
    }

    SDL_SetRenderDrawColor(renderer, 30, 60, 120, 255); // blue-ish
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

    draw_map(renderer);
    draw_player(renderer, &cam);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);   // ~16ms per frame ≈ 60fps
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
