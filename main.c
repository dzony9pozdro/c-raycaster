#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#define FOV 100
#define MAP_W 24
#define MAP_H 18
#define CELL 50

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

void draw_map(SDL_Renderer *renderer, int *debug) {
  int map[MAP_H][MAP_W] = {
      {0, 1, 0, 1},
      {1, 0, 1, 0},
      {0, 1, 0, 1},
  };
  int px = 0, py = 0;

  SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
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
void first_check(SDL_Renderer *renderer, Camera *cam, Ray_params *ray) {


  double dx, dy;

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

  ray->v_hit = (Vec2){cam->pos.x + dx, cam->pos.y + dx * (ray->dir.y / ray->dir.x)};
  ray->h_hit = (Vec2){cam->pos.x + dy * (ray->dir.x / ray->dir.y), cam->pos.y + dy};

  SDL_Rect h = {ray->h_hit.x, ray->h_hit.y, 8, 8};
  SDL_Rect v = {ray->v_hit.x, ray->v_hit.y, 8, 8};

  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
  SDL_RenderFillRect(renderer, &h);
  SDL_RenderFillRect(renderer, &v);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}
void next_checks(SDL_Renderer *renderer, Ray_params *ray) {

  double dx, dy;
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

  ray->v_hit = (Vec2){ray->v_hit.x + dx, ray->v_hit.y + dx * (ray->dir.y / ray->dir.x)};
  ray->h_hit = (Vec2){ray->h_hit.x + dy * (ray->dir.x / ray->dir.y), ray->h_hit.y + dy};

  // printf(" %.2f, %.2f \n", ray->v_hit.x, ray->v_hit.y);
  // printf(" %.2f, %.2f \n", ray->h_hit.x, ray->h_hit.y);

  SDL_Rect nh = {ray->h_hit.x, ray->h_hit.y, 8, 8};
  SDL_Rect nv = {ray->v_hit.x, ray->v_hit.y, 8, 8};

  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_RenderFillRect(renderer, &nh);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &nv);
}
void cast_ray(SDL_Renderer *renderer, Camera *cam, double deg) {
  Ray_params ray;
  ray.dir = (Vec2){cos(deg), sin(deg)};
  first_check(renderer, cam, &ray);

  ray.v_hit = (Vec2){ray.v_hit.x, ray.v_hit.y};
  ray.h_hit = (Vec2){ray.h_hit.x, ray.h_hit.y};

  for (int i = 0; i < (MAP_W + MAP_W/12); i++){
  next_checks(renderer, &ray);
  }
}
void cast_rays(SDL_Renderer *renderer, Camera *cam){
  double deg = fmod(cam->rad, 2 * M_PI);

  if (deg < 0) deg += 2 * M_PI;
  // printf("deg: %.3f PI\n", deg/M_PI);

  double step = 2.0/FOV;
  double raydeg = deg - step * (FOV / 2.0);
  for (int i = 0; i < FOV; i++){
  if (i == 0){
    printf("raydeg @0: %.3f\n", raydeg);
  }
  if (i == (FOV - 1)){
    printf("raydeg @ max :  %.3f\n", raydeg);
  }
  cast_ray(renderer, cam, raydeg);
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
  double line_length = 880;

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
  float acceleration = 10;
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
      turn(-0.02 * M_PI, &cam);
    }

    if (keys[SDL_SCANCODE_K]) {
      turn(0.02 * M_PI, &cam);
    }

    SDL_SetRenderDrawColor(renderer, 30, 60, 120, 255);  // blue-ish
    SDL_RenderClear(renderer);

    draw_map(renderer, &debug);
    update_player(&cam);

    draw_player(renderer, &cam);

    cast_rays(renderer, &cam);

    SDL_RenderPresent(renderer);
    SDL_Delay(12);  // in ms
  }

  // TODO:
  // make the ray check all intersections, not just the first possible one
  // next_check.x += CELL, next_check.y += CELL * dir.y/dir.x, and the inverse for the
  // other line direction. -ish? cast all of the rays - probably call this function in a
  // loop?? maybe bring the loop into this function, and extract the calculations into a
  // helper draw pillars of some const / distance height - distance is easy math probably
  // also vary color based on distance? need to draw a map for this, array makes it easy,
  // then any coordinate%CELL == 0 is a hit? ish?

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
