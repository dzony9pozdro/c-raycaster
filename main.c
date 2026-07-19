#include <SDL3/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define FOV 100
#define MAP_W 12
#define MAP_H 9
#define CELL 100

static SDL_Renderer *gr;

typedef struct {
  double x, y;
} Vec2;

typedef struct {
  Vec2 pos;
  Vec2 dir;
  Vec2 vel;
  double rad;
} Camera;

typedef struct {
  Vec2 v_hit;
  Vec2 h_hit;
  Vec2 dir;
} Ray_params;

Camera camera_default(void) {
  Camera c = {.pos = {.x = 300, .y = 300},
              .dir = {.x = 1, .y = 0},
              .vel = {.x = 0, .y = 0},
              .rad = 0};
  return c;
}

void draw_map(SDL_Renderer *renderer) {
  static int map[MAP_H][MAP_W] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  };

  static int px = 0;
  static int py = 0;

  SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);

  for (int row = 0; row < MAP_H; row++) {
    for (int col = 0; col < MAP_W; col++) {
      px = CELL * col;
      py = CELL * row;
      if (map[row][col] == 1) {
        SDL_FRect r = {(float)px, (float)py, CELL, CELL};
        SDL_RenderFillRect(renderer, &r);
      }
    }
  }

  for (int row = 0; row < MAP_H; row++) {

      py = CELL * row;
    SDL_FRect r = {0, (float)py, 30000, 1};
    SDL_RenderFillRect(renderer, &r);
  }

  for (int col = 0; col < MAP_W; col++) {

      px = CELL * col;
    SDL_FRect r = {(float)px, 0, 1, 300000};
    SDL_RenderFillRect(renderer, &r);
  }
}

int is_wall(double x, double y) {
  if (fmod(x, CELL) == 0 || fmod(y, CELL) == 0) {
    return 1;
  }
  return 0;
}

void first_check(SDL_Renderer *renderer, Camera *cam, Ray_params *ray,
                 const int *draw) {
  double dx;
  double dy;

  // vertical
  if (ray->dir.x < 0) {
    dx = -fmod(cam->pos.x, CELL);  // neg
  } else {
    dx = CELL - fmod(cam->pos.x, CELL);  // pos
  }

  // horizontal
  if (ray->dir.y < 0) {
    dy = -fmod(cam->pos.y, CELL);  // neg
  } else {
    dy = CELL - fmod(cam->pos.y, CELL);  // pos
  }

  ray->v_hit =
      (Vec2){cam->pos.x + dx, cam->pos.y + (dx * (ray->dir.y / ray->dir.x))};
  ray->h_hit =
      (Vec2){cam->pos.x + (dy * (ray->dir.x / ray->dir.y)), cam->pos.y + dy};

  SDL_FRect h = {(float)ray->h_hit.x, (float)ray->h_hit.y, 8, 8};
  SDL_FRect v = {(float)ray->v_hit.x, (float)ray->v_hit.y, 8, 8};

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
  if (*draw == 1) {
    SDL_RenderFillRect(renderer, &h);
    SDL_RenderFillRect(renderer, &v);
  }
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}

void next_checks(SDL_Renderer *renderer, Ray_params *ray, const int *draw) {
  double dx;
  double dy;
  if (ray->dir.x < 0) {
    dx = -CELL;
  } else {
    dx = CELL;
  }
  // horizontal
  if (ray->dir.y < 0) {
    dy = -CELL;
  } else {
    dy = CELL;
  }

  ray->v_hit = (Vec2){ray->v_hit.x + dx,
                      ray->v_hit.y + (dx * (ray->dir.y / ray->dir.x))};
  ray->h_hit = (Vec2){ray->h_hit.x + (dy * (ray->dir.x / ray->dir.y)),
                      ray->h_hit.y + dy};

  SDL_FRect nh = {(float)ray->h_hit.x, (float)ray->h_hit.y, 8, 8};
  SDL_FRect nv = {(float)ray->v_hit.x, (float)ray->v_hit.y, 8, 8};

  if (*draw == 1) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_RenderFillRect(renderer, &nh);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_RenderFillRect(renderer, &nv);
  }
}

void cast_ray(SDL_Renderer *renderer, Camera *cam, double deg, int *draw) {
  Ray_params ray;
  ray.dir = (Vec2){cos(deg), sin(deg)};
  first_check(renderer, cam, &ray, draw);

  for (int i = 0; i < (MAP_W + (MAP_W / 12)); i++) {
    next_checks(renderer, &ray, draw);
  }
}

void cast_rays(SDL_Renderer *renderer, Camera *cam) {
  double deg = fmod(cam->rad, 2 * M_PI);

  if (deg < 0) {
    deg += 2 * M_PI;
  }

  double step = 2.0 / FOV;
  double raydeg = deg - (step * (FOV / 2.0));
  int draw = 1;
  for (int i = 0; i < FOV; i++) {
    cast_ray(renderer, cam, raydeg, &draw);
    raydeg += step;
  }
}

void update_player(Camera *cam) {
  cam->pos.x += cam->vel.x;
  cam->pos.y += cam->vel.y;

  cam->vel.x /= 2;
  cam->vel.y /= 2;
}

void draw_player(SDL_Renderer *renderer, Camera *cam) {
  SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
  double line_length = 150;

  SDL_FRect p = {(float)(cam->pos.x - (CELL / 8.0)),
                 (float)(cam->pos.y - (CELL / 8.0)), CELL / 4.0, CELL / 4.0};

  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 190);

  SDL_RenderFillRect(renderer, &p);

  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);

  // dir vector
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderLine(renderer, (float)cam->pos.x, (float)cam->pos.y,
                 (float)cam->pos.x + ((float)(cam->dir.x * line_length)),
                 (float)cam->pos.y + (float)(cam->dir.y * line_length));
}

void turn(double direction, Camera *cam) {
  cam->rad += direction;
  cam->dir.x = cos(cam->rad);
  cam->dir.y = sin(cam->rad);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window =
      SDL_CreateWindow("raycaster", MAP_W * CELL, MAP_H * CELL, 0);

  gr = SDL_CreateRenderer(window, NULL);

  bool running = true;

  Camera cam = camera_default();

  const bool *keys = SDL_GetKeyboardState(NULL);
  float acceleration = 10;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      }
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
      turn(-0.02 * M_PI, &cam);
    }

    if (keys[SDL_SCANCODE_K]) {
      turn(0.02 * M_PI, &cam);
    }

    SDL_SetRenderDrawColor(gr, 30, 60, 120, 255);  // blue-ish
    SDL_RenderClear(gr);

    draw_map(gr);
    update_player(&cam);

    draw_player(gr, &cam);

    cast_rays(gr, &cam);

    SDL_RenderPresent(gr);
    SDL_Delay(12);  // in ms
  }

  // TODO:
  // helper draw pillars of some const / distance height - distance is easy math
  // probably also vary color based on distance? need to draw a map for this,
  // array makes it easy, then any coordinate%CELL == 0 is a hit? ish?

  SDL_DestroyRenderer(gr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
