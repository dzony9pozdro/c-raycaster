#include <SDL3/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_RAY_DEPTH 2
#define FOV 100
#define MAP_W 12
#define MAP_H 9
#define CELL 100

static SDL_Renderer *gr;

static int map[MAP_H][MAP_W] = {
    {1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},  //
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

typedef struct {
  double x, y;
} Vec2;

static Vec2 walls[MAP_H * MAP_W];

typedef struct {
  Vec2 pos;
  Vec2 dir;
  Vec2 vel;
  double rad;
} Camera;

typedef struct {
  Vec2 y_axis_hit;
  Vec2 x_axis_hit;
  Vec2 dir;
} Ray_params;

Camera camera_default(void) {
  Camera c = {.pos = {.x = 300, .y = 300},
              .dir = {.x = 1, .y = 0},
              .vel = {.x = 0, .y = 0},
              .rad = 0};
  return c;
}
void draw_grid(void) {
  static int px = 0;
  static int py = 0;

  for (int row = 0; row < MAP_H; row++) {
    py = CELL * row;
    SDL_FRect r = {0, (float)py, 30000, 1};
    SDL_RenderFillRect(gr, &r);
  }

  for (int col = 0; col < MAP_W; col++) {
    px = CELL * col;
    SDL_FRect r = {(float)px, 0, 1, 300000};
    SDL_RenderFillRect(gr, &r);
  }
}

void draw_map() {
  static int px = 0;
  static int py = 0;
  int i = 0;

  SDL_SetRenderDrawColor(gr, 200, 200, 200, 255);
  // int wall_count = 0;
  for (int cell_y = 0; cell_y < MAP_H; cell_y++) {
    for (int cell_x = 0; cell_x < MAP_W; cell_x++) {
      px = CELL * cell_x;
      py = CELL * cell_y;
      if (map[cell_y][cell_x] == 1) {
        walls[i++] = (Vec2){cell_x, cell_y};
        // wall_count += 1;
        SDL_FRect r = {(float)px, (float)py, CELL, CELL};
        SDL_RenderFillRect(gr, &r);
      }
    }
  }

  // for (int j = 0; j < wall_count; j++) {
  //   printf("cell_X: %d, cell_Y %d\n", (int)walls[j].x, (int)walls[j].y);
  // }
  draw_grid();
}
void debug_draw(Vec2 hit) {
  SDL_FRect h = {(float)hit.x, (float)hit.y, 8, 8};
  SDL_SetRenderDrawColor(gr, 255, 255, 255, 200);
  SDL_RenderFillRect(gr, &h);
  SDL_SetRenderDrawColor(gr, 255, 0, 0, 255);
}
void delta(int depth, Ray_params *ray, double *dx, double *dy, Camera *cam) {
  if (depth == 0) {
    if (ray->dir.x < 0) {
      *dx = -fmod(cam->pos.x, CELL);  // neg

    } else if (ray->dir.x > 0) {
      *dx = CELL - fmod(cam->pos.x, CELL);  // pos
    } else {
      //   TODO: (edge case) figure out what to do about this eventually
      *dx = 0;
    }

    if (ray->dir.y < 0) {
      *dy = -fmod(cam->pos.y, CELL);  // neg
    } else if (ray->dir.y > 0) {
      *dy = CELL - fmod(cam->pos.y, CELL);  // pos
    } else {
      //  TODO: (edge case) figure out what to do about this eventually
      *dy = 0;
    }
  } else {
    if (ray->dir.x < 0) {
      *dx = -CELL;
    } else {
      *dx = CELL;
    }
    if (ray->dir.y < 0) {
      *dy = -CELL;
    } else {
      *dy = CELL;
    }
  }
}
void check(Camera *cam, Ray_params *ray, int depth) {
  double dx;
  double dy;
  double slope = ray->dir.x / ray->dir.y;

  delta(depth, ray, &dx, &dy, cam);

  if (depth == 0) {
    ray->y_axis_hit = (Vec2){cam->pos.x + dx, cam->pos.y + (dx / slope)};
    ray->x_axis_hit = (Vec2){cam->pos.x + (dy * slope), cam->pos.y + dy};
  } else {
    ray->y_axis_hit =
        (Vec2){ray->y_axis_hit.x + dx, ray->y_axis_hit.y + (dx / slope)};
    ray->x_axis_hit =
        (Vec2){ray->x_axis_hit.x + (dy * slope), ray->x_axis_hit.y + dy};
  }

  debug_draw(ray->x_axis_hit);
  debug_draw(ray->y_axis_hit);
}

void cast_ray(Camera *cam, double deg) {
  Ray_params ray;
  ray.dir = (Vec2){cos(deg), sin(deg)};

  for (int depth = 0; depth < MAX_RAY_DEPTH; depth++) {
    check(cam, &ray, depth);
  }
}

void cast_rays(Camera *cam) {
  double deg = fmod(cam->rad, 2 * M_PI);

  if (deg < 0) {
    deg += 2 * M_PI;
  }

  // double step = 1.0 / FOV;
  // double raydeg = deg - (step * (FOV / 2.0));

  double raydeg = deg;

  cast_ray(cam, raydeg);
  for (int i = 0; i < FOV; i++) {
    // cast_ray(cam, raydeg);
    // raydeg += step;
  }
}

void update_player(Camera *cam) {
  cam->pos.x += cam->vel.x;
  cam->pos.y += cam->vel.y;

  cam->vel.x /= 2;
  cam->vel.y /= 2;
}

void draw_player(Camera *cam) {
  SDL_SetRenderDrawColor(gr, 200, 200, 200, 255);
  double line_length = 150;

  SDL_FRect p = {(float)(cam->pos.x - (CELL / 8.0)),
                 (float)(cam->pos.y - (CELL / 8.0)), CELL / 4.0, CELL / 4.0};

  SDL_SetRenderDrawColor(gr, 100, 100, 100, 190);

  SDL_RenderFillRect(gr, &p);

  SDL_SetRenderDrawColor(gr, 255, 0, 255, 255);

  // dir vector
  SDL_SetRenderDrawColor(gr, 0, 255, 0, 255);
  SDL_RenderLine(gr, (float)cam->pos.x, (float)cam->pos.y,
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
  float acceleration = 5;
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

    draw_map();

    update_player(&cam);

    draw_player(&cam);

    cast_rays(&cam);

    SDL_RenderPresent(gr);
    SDL_Delay(12);  // in ms
  }

  SDL_DestroyRenderer(gr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
