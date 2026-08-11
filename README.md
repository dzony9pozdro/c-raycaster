# raycasting engine using SDL3


## keybinds:
wasd to walk. 

j/k to turn

## debug keybinds:
Y to toggle fisheye correction
G to enable debug mode (topdown 2d view, showing all ray check points)
H to toggle single ray mode (only useful for debugging in 2d mode)



## Build

### Dependencies

- C23-compatible compiler
- SDL3
- SDL3_ttf
- pkg-config
- make


### Linux

Install SDL3, SDL3_ttf, pkg-config, make, and a C23-compatible compiler using your package manager.

Then:

```sh 
make
./ray
```

or:

```sh 
make run
```


### macOS

```sh
brew install sdl3 sdl3_ttf pkg-config
make
./ray
