# raycasting engine using SDL3

what it looks like with fisheye correction
<img width="1203" height="885" alt="image" src="https://github.com/user-attachments/assets/0765f170-ef58-432d-a87d-cd004de0aa6f" />

without fisheye correction:
<img width="1202" height="887" alt="image" src="https://github.com/user-attachments/assets/fb076ad9-5fbd-4e66-989a-65a72f3c5aee" />


debug mode:
<img width="1200" height="888" alt="image" src="https://github.com/user-attachments/assets/fdb671b1-8589-4c1e-9190-ebde4827b4af" />


single ray debug mode:
<img width="1197" height="889" alt="image" src="https://github.com/user-attachments/assets/2bcf635a-7d4a-4000-b994-924392002ae1" />



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
