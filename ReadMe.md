# MazeIO - OpenGL 2D Maze Game

## Overview
**MazeIO** is a top-down maze explorer where the player "emits light" to see walls. Built with **OpenGL 3.3 Core Profile**, the game creates a dark atmosphere where walls only become visible when the player is close, creating a challenging and immersive experience.

## Key Features
- **Proximity Lighting**: Dynamic fragment shader calculations create a "flashlight" effect centered on the player.
- **Neon Aesthetics**: Vibrant neon colors and subtle flickering effects for an immersive experience.
- **Glowing Trails**: A fading trail system that tracks movement and adds a sense of speed.
- **Grid-Based Collision**: Efficient collision handling using a 2D array representation of the maze.
- **Cross-Platform**: Runs on both Windows and Linux.

## Controls
- **W**: Move Up
- **S**: Move Down
- **A**: Move Left
- **D**: Move Right
- **ESC**: Exit Game

## Technical Specifications
- **Language**: C++
- **Graphics API**: OpenGL 3.3 Core Profile
- **Windowing/Input**: GLFW
- **OpenGL Loader**: GLAD

## Setup & Installation

### Windows (MinGW)
1. **Prerequisites**:
   - [MSYS2](https://www.msys2.org/) with `base-devel` and `gcc` packages.
   - OpenGL 3.3 compatible graphics drivers.
2. **Setup**:
   - Ensure `glfw3.dll` is present in both `build/` and `lib/` directories.
3. **Build**:
   - Open a terminal in the project root.
   - Run `mingw32-make` (or use the provided `make.bat`).
4. **Run**:
   - Run `mingw32-make run` or manually run the executable `main.exe` in the `build/` directory.

### Linux
1. **Prerequisites**:
   - `libglfw3-dev` and basic build tools (`build-essential`).
2. **Build**:
   - Run `make` in the project root.
3. **Run**:
   - Run `make run` or manually execute `./build/main`.

## Project Structure
- `src/`: Source files (`main.cpp`, `glad.c`)
- `include/`: Header files
- `lib/`: Compiled libraries (GLFW)
- `build/`: Executable and DLLs
- `Makefile`: Build instructions for multiple platforms
