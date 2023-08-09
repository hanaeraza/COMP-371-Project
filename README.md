# COMP 371 Project - Procedurally Generated Terrain 

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

## Features
### Transformations
- Up arrow = translate model up
- Left arrow = translate model left
- Down arrow = translate model down
- Right arrow = translate model right
- U = scale model up
- J = scale model down
- Q = rotate model counterclockwise
- E = rotate model clockwise
- R = reset model back to origin
### Movement
- W = move camera forward
- A = move camera left
- S = move camera backwards
- D = move camera right
### Toggles
- X = toggle textures 
- Z = toggle shadows
- 0 = free camera (ground)
- 1 = fixed camera (birds eye)

