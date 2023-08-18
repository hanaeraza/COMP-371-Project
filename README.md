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
- LEFT  = rotate car left
- RIGHT = rotate car right
- M = Zoom camera in
- N = Zoom camera out
- J = Reset camera zoom level
### Movement
- W = Move the camera and car forward
- A = Move the camera and car left
- S = Move the camera backwards
- D = Move the camera right
### Toggles
- C = Swap between camera views
- L = Toggle the headlights on and off
- 1 - 5 = Swap between different sky textures


## Credits

- Skybox Creation: [LearnOpenGL Tutorial](https://learnopengl.com/Advanced-OpenGL/Cubemaps)
- Skybox Textures: [Space 3D](https://tools.wwwtyro.net/space-3d/index.html)
- Labs provided on the COMP-371 course Moodle page

