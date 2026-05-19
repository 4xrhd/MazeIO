# MazeIO - Team Contribution & Presentation Guide
**Course:** CSE 358: Computer Graphics and Multimedia

This document provides a detailed breakdown for each team member on what to explain to the lab instructor (Any Chowdhury / Audity Ghosh). It includes the underlying logic, specific code concepts, and how your implemented features work.

---

## 1. Kazi Md Azhar Uddin Abir (0432320005101120)
**Role:** Core OpenGL Setup & Shader Programming (Proximity Lighting)

**What you need to explain:**
You are responsible for the core visual mechanic of the game: the proximity lighting/flashlight effect. You need to explain how the graphics pipeline works and how the Fragment Shader calculates light based on distance.

**Key Talking Points & Logic:**
* **OpenGL Pipeline Initialization:** Explain that the game uses OpenGL 3.3 Core Profile. We use GLFW to create the window context and GLAD to load the OpenGL function pointers.
* **Vertex Shader:** Explain that the vertex shader is simple. It takes the 3D vertex positions (`aPos`), sets `gl_Position`, and passes the 2D coordinates to the fragment shader as `FragPos`.
* **Fragment Shader & Euclidean Distance:** This is your main contribution. You need to explain how the light fades out. 
  * The CPU calculates the player's exact center on the screen and sends it to the shader as a `uniform vec2 playerPos;`.
  * For every pixel on the screen, the shader calculates the distance between that pixel (`FragPos`) and the player using: `float dist = distance(FragPos, playerPos);`.
* **Smoothstep Interpolation:** Explain how the light smoothly fades. Instead of a hard cut-off, you used `smoothstep(0.0, radius, dist)` which creates a smooth gradient from the center (1.0) to the edge of the radius (0.0).

**Code to point out in `main.cpp`:**
```cpp
// Point to the Fragment Shader code (Lines 48-71)
float dist = distance(FragPos, playerPos);
float intensity = 1.0 - smoothstep(0.0, radius, dist);
FragColor = vec4(baseColor * intensity, 1.0);
```

---

## 2. Ahamad Abdali Khan (0432320005101118)
**Role:** Collision Detection, Input Handling & Build Engine

**What you need to explain:**
You are responsible for the game's physics (collision boundaries), player movement, and ensuring the project compiles across different operating systems.

**Key Talking Points & Logic:**
* **Movement Logic & Cooldown:** Explain that movement isn't tied directly to frame rate. If we didn't have a cooldown, tapping a key would move the player across the screen instantly because the game runs at 60+ FPS. 
  * You used `glfwGetTime()` to implement a 0.15-second cooldown delay.
  * *Formula:* `if (currentTime - lastMoveTime > 0.15f)`
* **Collision Detection System:** Explain how the game checks if a move is valid *before* updating the player's position. 
  * The game looks ahead to `nextX` and `nextY`.
  * It checks the 2D `maze` array: `if (maze[nextY][nextX] == 0 || maze[nextY][nextX] == 2)`. 
  * `0` means it's an empty path, and `2` means it's the goal. If it's `1` (a wall), the move is rejected.
* **Makefile & Cross-Platform Build:** Explain that you wrote a Makefile that detects the OS (`ifeq ($(OS),Windows_NT)`). On Windows, it links `-lopengl32` and `-lglfw3`, while on Linux it links `-lGL` and X11 libraries. This makes the project portable.

**Code to point out in `main.cpp`:**
```cpp
// Point to processInput function (Lines 348-381)
if (currentTime - lastMoveTime > 0.15f) {
    // ... input mapping ...
    if ((nextX != playerGridX || nextY != playerGridY) &&
        (maze[nextY][nextX] == 0 || maze[nextY][nextX] == 2)) {
        // Move accepted
    }
}
```

---

## 3. Sabikun Nahar Alina (0432320005101016)
**Role:** Grid Coordinate Mapping, Trail Implementation & Dynamic VBOs

**What you need to explain:**
You are responsible for transforming the abstract 2D array into physical screen coordinates and implementing the visual trail system that follows the player.

**Key Talking Points & Logic:**
* **NDC Mapping:** Explain that OpenGL's screen coordinates (Normalized Device Coordinates or NDC) go from -1.0 to +1.0. 
  * Since our grid is 10x10, each cell takes up `0.2` space (`cellSize = 0.2f`). 
  * You mapped array indices `(x, y)` to screen coordinates by calculating: `ndcX = -1.0f + (x * cellSize)` and `ndcY = 1.0f - (y * cellSize)`.
* **Dynamic Trail Vector:** Explain the data structure behind the glowing trail. 
  * You used a `std::vector<Position> trail;`.
  * Every time the player makes a valid move, their *old* position is inserted at the front of the vector: `trail.insert(trail.begin(), {playerGridX, playerGridY});`.
  * If the vector size exceeds `MAX_TRAIL_SIZE` (which is 8), the oldest position is popped off the back.
* **Dynamic VBO Streaming:** Explain that while walls are static (uploaded to the GPU once), the player and trails move. You used `glBufferSubData` to dynamically stream the updated vertex positions to the GPU on every frame without destroying the buffer.

**Code to point out in `main.cpp`:**
```cpp
// Point to the Trail fading loop in the render section (Lines 274-307)
float tX = -1.0f + (trail[i].x * cellSize);
float tY = 1.0f - (trail[i].y * cellSize);
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(trailVertices), trailVertices);
```

---

## 4. Faria Chowdhury (0432220005101042)
**Role:** UI Colors, Flicker Modulation & Documentation

**What you need to explain:**
You are responsible for the aesthetic feel of the game (the neon cyberpunk vibe), the math behind the flickering light, and the overall structuring of the project documentation.

**Key Talking Points & Logic:**
* **Color Tuning:** Explain the choice of colors. You used an RGB color space passed as a `uniform vec3 baseColor` to the fragment shader. 
  * Walls are Neon Cyan `(0.0f, 0.8f, 1.0f)`.
  * The Goal is Neon Green `(0.0f, 1.0f, 0.4f)`.
  * The Player and Trails are Neon Magenta `(1.0f, 0.0f, 1.0f)`.
* **Flickering Light Mathematics:** Explain how you made the flashlight look realistic and unstable. 
  * You used trigonometric functions to create an oscillating wave.
  * *Formula:* `float flicker = 1.0 + 0.05 * sin(time * 15.0) * cos(time * 10.0);`
  * This multiplies a fast sine wave with a slightly slower cosine wave to create randomized-looking flicker peaks that modulate the light intensity by ±5%.
* **Trail Alpha Decay:** Explain how you made the trail visually fade out. 
  * You calculated an alpha multiplier based on the trail segment's index: `float alpha = 1.0f - ((float)(i + 1) / (MAX_TRAIL_SIZE + 1));`.
  * The most recent step gets ~88% opacity, and the 8th step gets ~11% opacity, creating a perfect linear fade-out effect.

**Code to point out in `main.cpp`:**
```cpp
// Point to the Flicker calculation in the Fragment Shader (Line 63)
float flicker = 1.0 + 0.05 * sin(time * 15.0) * cos(time * 10.0);

// Point to the Trail Alpha calculation in the render loop (Line 276)
float alpha = 1.0f - ((float)(i + 1) / (MAX_TRAIL_SIZE + 1));
```
