#include <SDL2/SDL.h>
#include <math.h>
#include <stdatomic.h>
#include <stdbool.h>

#define MAP_W (12 / 3)
#define MAP_H (9 / 3)
#define CELL 200

typedef struct {
  double x, y;
} Vec2;

typedef struct {
  Vec2 pos;
  Vec2 dir;
  Vec2 vel;
  double rad;
} Camera;

Camera camera_default(void) {
  Camera c = {.pos = {.x = 300, .y = 300},
              .dir = {.x = 1, .y = 0},
              .vel = {.x = 0, .y = 0},
              .rad = 0};
  return c;
}

void draw_map(SDL_Renderer *renderer, int *debug) {
  int map[MAP_H][MAP_W] = {
      {1, 1, 1, 1},
      {1, 0, 0, 0},
      {1, 0, 1, 0},
  };
  int px = 0, py = 0;

  for (int w = 0; w < MAP_W; w++) {
    for (int h = 0; h < MAP_H; h++) {
      px = CELL * w;
      py = CELL * h;
      if (map[h][w] == 1 && *debug == 0) {
        SDL_Rect r = {px, py, CELL, CELL};
        SDL_RenderFillRect(renderer, &r);
      } else {
        SDL_Rect r = {0, py, 30000, 1};
        SDL_RenderFillRect(renderer, &r);
      }
    }

    SDL_Rect r = {px, 0, 1, 300000};
    SDL_RenderFillRect(renderer, &r);
  }
}

void cast_ray(SDL_Renderer *renderer, Camera *cam) {
  // adjacent
  double pos_x_bound_rel_x = CELL - fmod(cam->pos.x, CELL);
  double pos_y_bound_rel_y = CELL - fmod(cam->pos.y, CELL);

  double neg_x_bound_rel_x = -fmod(cam->pos.x, CELL);
  double neg_y_bound_rel_y = -fmod(cam->pos.y, CELL);

  double y_from_x, x_from_y;

  if (cam->dir.x == 0) {
    cam->dir.x = 0.00001;
  }
  if (cam->dir.y == 0) {
    cam->dir.y = 0.00001;
  }

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

  // vertical
  //  first pos x vertical intersection vertices
  Vec2 posxv1 = {cam->pos.x + pos_x_bound_rel_x, cam->pos.y + pos_x_bound_rel_x * (cam->dir.y / cam->dir.x)};
  SDL_Rect pos_x_fc = {posxv1.x, posxv1.y, 10, 10};

  // first neg x vertical intersection vertices
  Vec2 negxv1 = {cam->pos.x + neg_x_bound_rel_x, cam->pos.y + neg_x_bound_rel_x * (cam->dir.y/cam->dir.x)};
  SDL_Rect neg_x_fc = {negxv1.x, negxv1.y, 10, 10};

  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
  SDL_RenderFillRect(renderer, &pos_x_fc);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &neg_x_fc);

  // horizontal

  //  first pos y vertical intersection vertices
  Vec2 posyv1 = {cam->pos.x + pos_y_bound_rel_y * (cam->dir.x / cam->dir.y), cam->pos.y + pos_y_bound_rel_y};
  SDL_Rect pos_y_fc = {posyv1.x, posyv1.y, 10, 10};

  //  first neg y vertical intersection vertices
  Vec2 negyv1 = {cam->pos.x + neg_y_bound_rel_y * (cam->dir.x/cam->dir.y), cam->pos.y + neg_y_bound_rel_y};
  SDL_Rect neg_y_fc = {negyv1.x, negyv1.y, 10, 10};

  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  SDL_RenderFillRect(renderer, &pos_y_fc);
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  SDL_RenderFillRect(renderer, &neg_y_fc);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

  Vec2 check;
  Vec2 interval;
  Vec2 next_check = {.x = check.x + interval.x, .y = check.y + interval.y};

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}

void draw_player(SDL_Renderer *renderer, Camera *cam) {
  cam->pos.x += cam->vel.x;
  cam->pos.y += cam->vel.y;

  double line_length = 280;

  cam->vel.x /= 2;
  cam->vel.y /= 2;

  SDL_Rect p = {cam->pos.x - CELL / 8.0, cam->pos.y - CELL / 8.0, CELL / 4, CELL / 4};

  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 190);

  SDL_RenderFillRect(renderer, &p);

  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

  // sin
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderDrawLine(renderer, cam->pos.x, cam->pos.y, cam->pos.x,
                     cam->pos.y + cam->dir.y * line_length);
  // cos

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  SDL_RenderDrawLine(renderer, cam->pos.x, cam->pos.y,
                     cam->pos.x + cam->dir.x * line_length, cam->pos.y);

  // dir vector
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderDrawLine(renderer, cam->pos.x, cam->pos.y,
                     cam->pos.x + cam->dir.x * line_length,
                     cam->pos.y + cam->dir.y * line_length);
}

void turn(double direction, Camera *cam) {
  cam->rad += direction;
  cam->dir.x = cos(cam->rad);
  cam->dir.y = sin(cam->rad);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  int debug = 1;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window =
      SDL_CreateWindow("raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       MAP_W * CELL, MAP_H * CELL, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

  bool running = true;

  Camera cam = camera_default();

  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  float acceleration = 11;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
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
      turn(-0.08, &cam);
    }

    if (keys[SDL_SCANCODE_K]) {
      turn(0.08, &cam);
    }

    SDL_SetRenderDrawColor(renderer, 30, 60, 120, 255);  // blue-ish
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    draw_map(renderer, &debug);
    draw_player(renderer, &cam);
    cast_ray(renderer, &cam);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~16ms per frame ≈ 60fps
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
