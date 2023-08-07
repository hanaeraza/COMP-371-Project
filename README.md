# Assignment 2 - Tennis Ball and Rendering

## Build Instructions
### Command Line

1. Download and Install CMake
2. Open a terminal
3. Run cmake:

```
cd <source_folder>
cmake -S . -B <build_folder>
cmake --build <build_folder> --target install
```

###Features
SPACE = translate model to random location on grid
U = scale model up
J = scale model down
W = translate model up
A = translate model left
S = translate model down
D = translate model right
Q = rotate model counterclockwise
E = rotate model clockwise
R = reset model back to origin
P = switch rendering mode to points
L = switch rendering mode to lines
T = switch rendering mode to triangles
Hold right mouse button and move mouse to pan camera left or right
Hold middle mouse button and move mouse to tilt camera up or down
Hold left mouse button and move mouse to zoom in or out (can also use M or N keys)
M = zoom camera out
N = zoom camera in
Left arrow = rotate world orientation Rx
Right arrow = rotate world orientation R-x
Up arrow = rotate world orientation Ry
Down arrow = rotate world orientation R-y

1 = fixed camera (default)
0 = free camera

F = move camera forward
C = move camera left
V = move camera backwards
B = move camera right

X = toggle textures 
Z = toggle shadows

