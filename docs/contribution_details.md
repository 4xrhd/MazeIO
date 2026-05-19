# MazeIO - Team Contribution & Presentation Guide
**Course:** CSE 358: Computer Graphics and Multimedia

This document provides a perfectly balanced breakdown for each team member on what to explain to the lab instructor (Any Chowdhury / Audity Ghosh). Every member has a significant portion of OpenGL, C++ logic, and mathematical concepts to present.

---

## 1. Kazi Md Azhar Uddin Abir (0432320005101120)
**Role:** OpenGL Context, Render Loop Architecture & Vertex Shaders

**What you need to explain:**
You are responsible for initializing the engine and passing physical coordinates to the GPU. You handle the foundation of the graphics pipeline.

**Key Talking Points & Logic:**
* **OpenGL Pipeline & Main Loop:** Explain that you used GLFW to create the window context and GLAD to load OpenGL function pointers. You designed the `while (!glfwWindowShouldClose(window))` rendering loop that acts as the heartbeat of the game.
* **Vertex Shader Compilation:** You wrote the shader that translates 3D coordinate space into 2D screen space. It takes the `aPos` vector and assigns it to `gl_Position`, while forwarding `FragPos` so the fragment shader knows exactly where each pixel is.
* **NDC Geometry Mapping:** Explain that screen coordinates in OpenGL are Normalized Device Coordinates (NDC) ranging from -1.0 to 1.0. You calculated how a 10x10 grid is mapped mathematically using formulas like `ndcX = -1.0f + (x * cellSize)`.

**Code to point out in `main.cpp`:**
```cpp
// Point out the Vertex Shader (Lines 111-120)
gl_Position = vec4(aPos, 1.0);
FragPos = vec2(aPos.x, aPos.y);

// Point out the Grid Mapping math (Lines 293-294)
float ndcX = -1.0f + (x * cellSize);
```

---

## 2. Ahamad Abdali Khan (0432320005101118)
**Role:** Fragment Shaders, Proximity Lighting Math & Build Engine

**What you need to explain:**
You are responsible for the core visual mechanic (the flashlight effect) calculated on the GPU and the cross-platform compilation architecture.

**Key Talking Points & Logic:**
* **Fragment Shader Pipeline:** Explain that while the vertex shader handles position, your fragment shader handles every individual pixel's color on the screen.
* **Proximity Lighting Mathematics:** Explain the "flashlight" cone. The CPU sends the player's center position as a `uniform vec2 playerPos`. 
  * You calculated Euclidean distance: `float dist = distance(FragPos, playerPos);`.
  * To make it fade smoothly into darkness (instead of a hard circle), you applied Hermite interpolation using `smoothstep(0.0, radius, dist)`.
* **Multi-platform Build System:** You wrote the `Makefile` utilizing compiler directives (`ifeq ($(OS),Windows_NT)`) to link `-lopengl32` on Windows and `-lGL` on Linux, ensuring the game compiles instantly regardless of the instructor's OS.

**Code to point out in `main.cpp`:**
```cpp
// Point out the Fragment Shader distance math (Lines 131-133)
float dist = distance(FragPos, playerPos);
float intensity = 1.0 - smoothstep(0.0, radius, dist);
```

---

## 3. Sabikun Nahar Alina (0432320005101016)
**Role:** Collision Physics, Movement Mechanics & Dynamic VBO Memory

**What you need to explain:**
You are responsible for the physical rules of the game environment (collision detection) and managing how data is streamed dynamically to the GPU memory.

**Key Talking Points & Logic:**
* **Collision Detection Engine:** Explain how players are blocked by walls. 
  * Before a player moves, the logic looks at `nextX` and `nextY` coordinates.
  * It checks the 2D array: `if (maze[nextY][nextX] == 0 || maze[nextY][nextX] == 2)`. If it's `1` (a wall), the move is blocked.
* **Input Mechanics & Cooldown:** Since modern CPUs execute loops millions of times a second, tapping 'W' would instantly teleport the player. You implemented a time-delta physics constraint: `if (currentTime - lastMoveTime > 0.15f)` to enforce a 150-millisecond step delay.
* **Dynamic VBO Buffering:** Walls are static, but the player moves. Instead of destroying and recreating the Vertex Buffer Object (VBO) every frame (which is very slow), you used `glBufferSubData` to rapidly stream new coordinates into the existing GPU memory buffer.

**Code to point out in `main.cpp`:**
```cpp
// Point out the collision and cooldown logic (Lines 523-541)
if (currentTime - lastMoveTime > 0.15f) {
    if ((maze[nextY][nextX] == 0 || maze[nextY][nextX] == 2)) { ... }
}

// Point out the dynamic buffer streaming (Line 389)
glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(playerVertices), playerVertices);
```

---

## 4. Faria Chowdhury (0432220005101042)
**Role:** Trigonometric Light Math, Vector History & Trail Decay

**What you need to explain:**
You are responsible for the advanced mathematical aesthetic effects, combining trigonometry for lighting manipulation and sequence containers for visual trails.

**Key Talking Points & Logic:**
* **Trigonometric Light Flickering:** The flashlight needs to look organic, like a flickering torch. 
  * You used mathematical waves driven by the application's runtime. 
  * *Formula:* `flicker = 1.0 + 0.05 * sin(time * 15.0) * cos(time * 10.0);`
  * This multiplies a high-frequency sine wave with a lower-frequency cosine wave to create unpredictable peak interference, randomly dimming the light by ±5%.
* **Vector History Structure (Trails):** Explain how the fading trail works. 
  * You utilized a C++ Standard Template Library `std::vector` to store the historical positions. When a move happens, the new position pushes to the front, and if the size exceeds 8, the back is popped off.
* **Alpha Decay Linear Interpolation:** To make the trail visually fade away, you applied a linear decay function: `float alpha = 1.0f - ((float)(i + 1) / 9.0f);`. 
  * The most recent coordinate (i=0) has 88% visibility, while the oldest coordinate (i=7) has only 11% visibility, achieving a perfect neon fade out.

**Code to point out in `main.cpp`:**
```cpp
// Point out the Flicker wave math in the Shader (Line 136)
float flicker = 1.0 + 0.05 * sin(time * 15.0) * cos(time * 10.0);

// Point out the Alpha decay math in the rendering loop (Line 416)
float alpha = 1.0f - ((float)(i + 1) / (MAX_TRAIL_SIZE + 1));
```
