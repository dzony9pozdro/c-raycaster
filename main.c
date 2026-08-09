#include <SDL3/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_RAY_DEPTH 5
#define FOV 80
#define MAP_W 12
#define MAP_H 9
#define CELL 100

#define TARGET_FPS 150
#define TICK_HZ 60

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
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} color;

static int g_debug = 0;
enum axis { X, Y };

[[maybe_unused]] static const struct {
  color red;
  color green;
  color blue;
  color magenta;
  color yellow;
  color white;
} colors = {
    .red = {255, 0, 0, 200},
    .green = {0, 255, 0, 200},
    .blue = {0, 0, 255, 200},
    .magenta = {255, 0, 255, 200},
    .yellow = {255, 255, 0, 200},
    .white = {255, 255, 255, 200},
};

typedef struct {
  double x, y;
} Vec2;

typedef struct {
  double ax;
  double ay;
  double turn_dir;
} Input;
typedef struct {
  Vec2 delta;
  double dist;
} Hit;

static Vec2 walls[MAP_H * MAP_W];

typedef struct {
  Vec2 pos;
  Vec2 dir;
  Vec2 vel;
  Vec2 relative_pos;
  double rad;

} Camera;

typedef struct {
  Vec2 dir;
  Vec2 pos;
  double slope;
  int depth;
  Vec2 relative_pos;
} Ray;

static Camera camera_default(void) {
  Camera c = {.pos = {.x = 300, .y = 300},
              .dir = {.x = 1, .y = 0},
              .vel = {.x = 0, .y = 0},
              .rad = 0};
  return c;
}
static void draw_grid(void) {
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

static void draw_map() {
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

static void debug_draw(Vec2 hit, color col) {
  SDL_FRect h = {(float)hit.x, (float)hit.y, 8, 8};
  SDL_SetRenderDrawColor(gr, col.r, col.g, col.b, col.a);
  SDL_RenderFillRect(gr, &h);
}

static int sign_of(double k) { return (k > 0) - (k < 0); }

static Vec2 find_next_intersection(Vec2 delta, Ray *ray) {
  return (Vec2){ray->pos.x + delta.x, ray->pos.y + delta.y};
}

static Hit hit_from_dy(double dy, Ray *ray) {
  double dx = dy / ray->slope;
  return (Hit){(Vec2){dx, dy}, (dx * dx) + (dy * dy)};
}

static Hit hit_from_dx(double dx, Ray *ray) {
  double dy = dx * ray->slope;
  return (Hit){(Vec2){dx, dy}, (dx * dx) + (dy * dy)};
}
static Ray init_ray(Camera *cam, double radian_raydeg) {
  Ray ray;

  ray.dir = (Vec2){cos(radian_raydeg), sin(radian_raydeg)};
  ray.pos = cam->pos;
  ray.slope = ray.dir.y / ray.dir.x;  // tan(deg)
  ray.depth = 0;
  ray.relative_pos = cam->relative_pos;

  return ray;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
static double get_delta_from_pos(const double pos, const int sign) {
  if (pos == 0) {
    return sign * CELL;
  }

  if (sign == -1) {
    return -pos;
  }
  if (sign == 1) {
    return (CELL - pos);
  }
  return 0;
}

static Vec2 get_closer_delta(Ray *ray) {
  double x = ray->relative_pos.x;
  double y = ray->relative_pos.y;
  double throwaway_dx;
  double throwaway_dy;
  throwaway_dx = get_delta_from_pos(x, sign_of(ray->dir.x));
  Hit hit_f_dx = hit_from_dx(throwaway_dx, ray);
  throwaway_dy = get_delta_from_pos(y, sign_of(ray->dir.y));
  Hit hit_f_dy = hit_from_dy(throwaway_dy, ray);

  Hit closer_hit =
      (fabs(hit_f_dx.dist) < fabs(hit_f_dy.dist)) ? hit_f_dx : hit_f_dy;

  Vec2 delta = closer_hit.delta;
  return delta;
}

static void advance_ray(Ray *ray) {
  Vec2 delta = get_closer_delta(ray);

  ray->pos = find_next_intersection(delta, ray);

  debug_draw(ray->pos, colors.red);

  ray->relative_pos.x = fmod(ray->pos.x, CELL);
  ray->relative_pos.y = fmod(ray->pos.y, CELL);
}

static void cast_ray(Camera *cam, double deg) {
  Ray ray = init_ray(cam, deg);

  for (int depth = 0; depth < MAX_RAY_DEPTH; depth++) {
    advance_ray(&ray);
  }
}

static void cast_rays(Camera *cam) {
  if (g_debug == 1) {
    cast_ray(cam, cam->rad);
    return;
  }

  double radian_FOV = (FOV / 360.0) * 2 * M_PI;
  int ray_count;
  double radian_step = 0.001;
  ray_count = (int)(radian_FOV / radian_step);

  double radian_raydeg = cam->rad - (radian_FOV / 2.0);

  for (int i = 0; i < ray_count; i++) {
    cast_ray(cam, radian_raydeg);
    radian_raydeg += radian_step;
  }
}

static void update_player(Camera *cam) {
  cam->pos.x += cam->vel.x;
  cam->pos.y += cam->vel.y;

  cam->relative_pos.x = fmod(cam->pos.x, CELL);
  cam->relative_pos.y = fmod(cam->pos.y, CELL);

  cam->vel.x /= 2;
  cam->vel.y /= 2;
}

static void draw_player(Camera *cam) {
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

static void handle_input(Input *input){

  input->turn_dir = 0.0;
  input->ax = 0.0;
  input->ay = 0.0;

  const double acceleration = 9.0;
  const double turn_rate = 0.02 * M_PI;

  const bool *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W]) {
      input->ay -= acceleration;
    }
    if (keys[SDL_SCANCODE_S]) {
      input->ay += acceleration;
    }
    if (keys[SDL_SCANCODE_A]) {
      input->ax -= acceleration;
    }
    if (keys[SDL_SCANCODE_D]) {
      input->ax += acceleration;
    }
    if (keys[SDL_SCANCODE_J]) {
      input->turn_dir -= turn_rate;
    }
    if (keys[SDL_SCANCODE_K]) {
      input->turn_dir += turn_rate;
    }


}
static void turn(double direction, Camera *cam) {
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



  const Uint64 FRAME_NS = SDL_NS_PER_SECOND / TARGET_FPS;
  const Uint64 TICK_NS = SDL_NS_PER_SECOND / TICK_HZ;

  Uint64 accumulator = 0;
  Uint64 prev = SDL_GetTicksNS();

  while (running) {
    Uint64 frame_start = SDL_GetTicksNS();

    accumulator += frame_start - prev;
    prev = frame_start;

    if (accumulator > SDL_NS_PER_SECOND / 4) {
      accumulator = SDL_NS_PER_SECOND / 4;
    }

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }
    
    Input input;
    handle_input(&input);


    while (accumulator >= TICK_NS) {
      if (input.turn_dir != 0.0) {
        turn(input.turn_dir, &cam);
      }

      cam.vel.x += input.ax;
      cam.vel.y += input.ay;

      update_player(&cam);
      accumulator -= TICK_NS;
    }

    SDL_SetRenderDrawColor(gr, 30, 60, 120, 255);
    SDL_RenderClear(gr);

    draw_map();
    draw_player(&cam);
    cast_rays(&cam);

    SDL_RenderPresent(gr);

    Uint64 elapsed = SDL_GetTicksNS() - frame_start;
    if (elapsed < FRAME_NS) {
      SDL_DelayPrecise(FRAME_NS - elapsed);
    }
  }

  SDL_DestroyRenderer(gr);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
