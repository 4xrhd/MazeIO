// ============================================================================
// MazeIO - A top-down maze explorer where the player "emits light" to see walls
// ============================================================================
//
// TEAM MEMBERS & BALANCED CONTRIBUTIONS:
// 1. Kazi Md Azhar Uddin Abir (0432320005101120)
//    - OpenGL Context Creation, Window Initialization, & Window Resize
//    Callbacks
//    - Main Rendering Loop and State Controller Architecture
//    - Vertex Shader Compilation & Normalized Device Coordinate (NDC) Grid
//    Mapping
//
// 2. Ahamad Abdali Khan (0432320005101118)
//    - Fragment Shader Architecture & Pixel Pipeline Execution
//    - Proximity Lighting Mathematics (Euclidean Distance & smoothstep
//    attenuation)
//    - Multi-platform C++ Build Configuration (Linux & Windows makefile
//    automation)
//
// 3. Sabikun Nahar Alina (0432320005101016)
//    - Grid-Based Collision Detection Physics Engine
//    - Step Cooldown Input Constraint Mechanics (0.15s Delta Time Filter)
//    - GPU Dynamic Buffer streaming (VAO/VBO configurations & glBufferSubData)
//
// 4. Faria Chowdhury (0432220005101042)
//    - Advanced Shader Math (Sine/Cosine product spotlight flickering
//    modulation)
//    - Trail Vector History Management (C++ STL vector FIFO list)
//    - Alpha Fading Trail Decay Math (Linear Interpolation for trail opacity)
//
// ============================================================================

/**
 * @file main.cpp
 * @brief MazeIO - An OpenGL 3.3 Core Profile 2D Maze Game
 * @details Implements a dynamic fragment shader for proximity lighting,
 *          time-based flickering, fading movement trails, and collision
 * detection.
 */
// --- Library Includes ---
#include "glad.h"  // OpenGL function loader - must be included BEFORE glfw
#include "glfw3.h" // Window creation, input handling, and OpenGL context

#include <cmath>    // Math functions (used implicitly by shaders via sin/cos)
#include <iostream> // Console output for Linux welcome message and errors
#include <vector>   // Dynamic arrays for storing vertex data

// --- Platform-Specific Includes ---
// Windows uses MessageBox dialogs for welcome/win popups.
// Linux/Mac use console output instead (see #ifdef blocks below).
#ifdef _WIN32
#include <windows.h>
#endif

// --- Forward Declarations ---
// These functions are defined at the bottom of the file.
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// ============================================================================
// GLOBAL CONSTANTS & STATE
// ============================================================================

// --- Window Settings ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800; // Square window matches the square grid

// --- Maze Definition ---
// The maze is a 10x10 integer grid:
//   0 = Open path  (player can walk here)
//   1 = Wall        (blocks movement, rendered as neon blue)
//   2 = Goal        (win condition, rendered as neon green)
//
// Visual layout (S = start at [1][1], G = goal at [8][8]):
//   ██████████████████████
//   ██ S      ██         ██
//   ██   ██   ██   ████  ██
//   ██   ██            ██ ██
//   ██   ████████   ██   ██
//   ██            ██      ██
//   ████████   ████████   ██
//   ██               ██   ██
//   ██   ████████      G  ██
//   ██████████████████████
int maze[10][10] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 0, 1}, {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 0, 1}, {1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 0, 1, 1, 1, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 0, 2, 1}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

// --- Player State ---
struct Position {
  int x, y; // Grid coordinates (column, row)
};
int playerGridX = 1;          // Player starting column (maze[1][1])
int playerGridY = 1;          // Player starting row
std::vector<Position> trail;  // Stores recent positions for the trail effect
const int MAX_TRAIL_SIZE = 8; // Maximum number of trail segments to show
float lastMoveTime = 0.0f;    // Timestamp of last move (for input cooldown)
bool gameWon = false;         // Flag to prevent repeated win triggers

// ============================================================================
// SHADERS (GLSL code that runs on the GPU)
// ============================================================================

// --- Vertex Shader ---
// AUTHOR: Kazi Md Azhar Uddin Abir
// PURPOSE: Transforms each vertex position and passes it to the fragment
// shader. INPUT:   aPos (vec3) - the x,y,z position of each vertex OUTPUT:
// FragPos (vec2) - the x,y position sent to the fragment shader
//          gl_Position - the final screen position of the vertex
//
// Since we already use Normalized Device Coordinates (NDC: -1 to +1),
// there is no need for a projection matrix - positions pass through directly.
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "out vec2 FragPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos, 1.0);\n"
                                 "   FragPos = vec2(aPos.x, aPos.y);\n"
                                 "}\0";

// --- Fragment Shader ---
// CO-AUTHORS: Ahamad Abdali Khan (Lighting & Pipeline) & Faria Chowdhury
// (Advanced Trigonometric Modulation) PURPOSE: Calculates the color of each
// pixel based on proximity to the player.
//          This creates the "emitting light" effect - objects far from the
//          player appear dark, while nearby objects glow brightly.
//
// UNIFORMS (values sent from CPU each frame):
//   playerPos - the player's center position in NDC space
//   baseColor - the RGB color of the current object (blue for walls, etc.)
//   time      - elapsed time used for the flickering animation
//
// HOW THE LIGHTING WORKS:
//   1. Calculate distance from this pixel to the player (Ahamad Abdali Khan)
//   2. smoothstep() maps distance to intensity: 1.0 at center → 0.0 at edge
//   (Ahamad Abdali Khan)
//   3. A subtle sin/cos product flicker makes the light feel alive (Faria
//   Chowdhury)
//   4. Final color = baseColor * intensity (dark pixels become invisible)
const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 FragPos;\n"
    "uniform vec2 playerPos;\n"
    "uniform vec3 baseColor;\n"
    "uniform float time;\n"
    "void main()\n"
    "{\n"
    "   // Proximity Lighting Math by Ahamad Abdali Khan\n"
    "   float dist = distance(FragPos, playerPos);\n"
    "   float radius = 0.5f;\n"
    "   \n"
    "   // Trigonometric Flickering spotlight mathematics by Faria Chowdhury\n"
    "   float flicker = 1.0 + 0.05 * sin(time * 15.0) * cos(time * 10.0);\n"
    "   \n"
    "   float intensity = 1.0 - smoothstep(0.0, radius, dist);\n"
    "   intensity *= flicker;\n"
    "   \n"
    "   FragColor = vec4(baseColor * intensity, 1.0);\n"
    "}\n\0";

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main() {

  // --------------------------------------------------------------------------
  // PHASE 1: Initialize GLFW and create a window
  // AUTHOR: Kazi Md Azhar Uddin Abir
  // --------------------------------------------------------------------------
  // GLFW handles: window creation, OpenGL context, and keyboard input.
  // We request OpenGL 3.3 Core Profile (no deprecated fixed-function pipeline).
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  // macOS requires forward compatibility for OpenGL 3.3+
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Create the game window with the title "MazeIO"
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MazeIO", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // Register callback: automatically adjusts viewport when window is resized
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // --------------------------------------------------------------------------
  // PHASE 2: Load OpenGL functions via GLAD
  // --------------------------------------------------------------------------
  // GLAD dynamically loads all OpenGL function pointers at runtime.
  // Without this, calling any gl* function would crash.
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // --------------------------------------------------------------------------
  // PHASE 3: Show welcome message (platform-specific)
  // --------------------------------------------------------------------------
#ifdef _WIN32
  // Windows: show a native dialog popup
  MessageBoxA(NULL,
              "Welcome to MazeIO!\n"
              "A top-down maze explorer where you emit light to see walls.\n\n"
              "Controls:\n"
              "W/A/S/D - Move\n"
              "ESC - Quit\n\n"
              "Objective: Reach the Green Goal!",
              "MazeIO - Welcome", MB_OK | MB_ICONINFORMATION);
#else
  // Linux/Mac: print to the terminal
  std::cout << "Welcome to MazeIO!\n"
               "A top-down maze explorer where you emit light to see walls.\n\n"
               "Controls: W/A/S/D - Move | ESC - Quit\n"
               "Objective: Reach the Green Goal!"
            << std::endl;
#endif

  // --------------------------------------------------------------------------
  // PHASE 4: Compile and link the shader program
  // --------------------------------------------------------------------------
  // The shader program consists of a vertex shader and a fragment shader.
  // OpenGL compiles them into GPU-executable code, then links them together.
  // After linking, the individual shader objects can be deleted.

  // Compile vertex shader
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  // Compile fragment shader
  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  // Link both shaders into a single program
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Clean up: shader objects are no longer needed after linking
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // --------------------------------------------------------------------------
  // PHASE 5: Generate vertex data for walls and goal from the maze grid
  // --------------------------------------------------------------------------
  // We iterate the 10x10 maze and create a quad (2 triangles = 6 vertices)
  // for each wall cell (1) and goal cell (2).
  //
  // COORDINATE SYSTEM:
  //   OpenGL NDC ranges from -1.0 to +1.0 on both axes (total range = 2.0).
  //   With a 10x10 grid, each cell is 2.0 / 10 = 0.2 units wide and tall.
  //   Grid (0,0) maps to top-left corner (-1.0, +1.0) in NDC.

  std::vector<float> wallVertices; // Stores all wall triangle vertices
  std::vector<float> goalVertices; // Stores goal triangle vertices
  float cellSize = 0.2f;           // Each cell = 0.2 NDC units (2.0 / 10)

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      if (maze[y][x] == 1 || maze[y][x] == 2) {
        // Convert grid position to NDC top-left corner
        float ndcX = -1.0f + (x * cellSize); // Left edge of cell
        float ndcY = 1.0f - (y * cellSize);  // Top edge of cell

        // Each quad = 2 triangles = 6 vertices (x, y, z each)
        // Triangle 1: top-left, top-right, bottom-left
        // Triangle 2: top-right, bottom-right, bottom-left
        float quad[] = {
            ndcX,
            ndcY,
            0.0f, // top left
            ndcX + cellSize,
            ndcY,
            0.0f, // top right
            ndcX,
            ndcY - cellSize,
            0.0f, // bottom left

            ndcX + cellSize,
            ndcY,
            0.0f, // top right
            ndcX + cellSize,
            ndcY - cellSize,
            0.0f, // bottom right
            ndcX,
            ndcY - cellSize,
            0.0f // bottom left
        };

        // Sort vertices into the appropriate list
        if (maze[y][x] == 1)
          wallVertices.insert(wallVertices.end(), quad, quad + 18);
        else
          goalVertices.insert(goalVertices.end(), quad, quad + 18);
      }
    }
  }

  // --------------------------------------------------------------------------
  // PHASE 6: Create OpenGL buffer objects (VAO + VBO) for each drawable
  // AUTHOR: Sabikun Nahar Alina (Dynamic GPU buffer & layout attribute maps)
  // --------------------------------------------------------------------------
  // VAO (Vertex Array Object): remembers the vertex attribute configuration.
  // VBO (Vertex Buffer Object): holds the actual vertex data on the GPU.
  //
  // We create 3 separate VAO/VBO pairs:
  //   1. Walls  (static - data never changes)
  //   2. Goal   (static - data never changes)
  //   3. Player (dynamic - position updates every frame)

  // --- Wall buffers (GL_STATIC_DRAW = data uploaded once, never modified) ---
  unsigned int wallVAO, wallVBO;
  glGenVertexArrays(1, &wallVAO);
  glGenBuffers(1, &wallVBO);
  glBindVertexArray(wallVAO);
  glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
  glBufferData(GL_ARRAY_BUFFER, wallVertices.size() * sizeof(float),
               wallVertices.data(), GL_STATIC_DRAW);
  // Tell OpenGL: attribute 0 = 3 floats (x,y,z), tightly packed
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // --- Goal buffers (also static) ---
  unsigned int goalVAO, goalVBO;
  glGenVertexArrays(1, &goalVAO);
  glGenBuffers(1, &goalVBO);
  glBindVertexArray(goalVAO);
  glBindBuffer(GL_ARRAY_BUFFER, goalVBO);
  glBufferData(GL_ARRAY_BUFFER, goalVertices.size() * sizeof(float),
               goalVertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // --- Player buffers (GL_DYNAMIC_DRAW = data will change frequently) ---
  // We allocate space for 6 vertices (1 quad) but don't fill it yet.
  // The actual vertex data is uploaded each frame via glBufferSubData().
  unsigned int playerVAO, playerVBO;
  glGenVertexArrays(1, &playerVAO);
  glGenBuffers(1, &playerVBO);
  glBindVertexArray(playerVAO);
  glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
  glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // ==========================================================================
  // PHASE 7: Main render loop (runs every frame until window is closed)
  // AUTHOR: Kazi Md Azhar Uddin Abir (Render loop execution & buffer swapping)
  // ==========================================================================
  // Each iteration of this loop is ONE FRAME. The loop:
  //   1. Reads keyboard input and updates player position
  //   2. Calculates the player's NDC coordinates for the lighting shader
  //   3. Draws walls, goal, trail, and player (in that order)
  //   4. Swaps buffers (double buffering prevents flickering)
  //   5. Checks if the player reached the goal
  //
  while (!glfwWindowShouldClose(window)) {

    // --- Step 1: Process keyboard input and update player grid position ---
    processInput(window);

    // --- Step 2: Convert player grid position to NDC for the shader ---
    // The shader needs the player's CENTER position to calculate light
    // distance. We add half a cell size to get the center instead of the
    // top-left corner.
    float playerNdcX = -1.0f + (playerGridX * cellSize) + (cellSize / 2.0f);
    float playerNdcY = 1.0f - (playerGridY * cellSize) - (cellSize / 2.0f);

    // Calculate the player quad's TOP-LEFT corner in NDC
    float pX = -1.0f + (playerGridX * cellSize);
    float pY = 1.0f - (playerGridY * cellSize);

    // Padding shrinks the player block so it's visually smaller than the cell
    float padding = 0.04f;

    // Build 6 vertices (2 triangles) for the player quad
    float playerVertices[] = {pX + padding,
                              pY - padding,
                              0.0f,
                              pX + cellSize - padding,
                              pY - padding,
                              0.0f,
                              pX + padding,
                              pY - cellSize + padding,
                              0.0f,

                              pX + cellSize - padding,
                              pY - padding,
                              0.0f,
                              pX + cellSize - padding,
                              pY - cellSize + padding,
                              0.0f,
                              pX + padding,
                              pY - cellSize + padding,
                              0.0f};

    // Upload player vertices to GPU (updates the dynamic VBO)
    glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(playerVertices), playerVertices);

    // --- Step 3: Clear screen and prepare for drawing ---
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f); // Near-black with a blue tint
    glClear(GL_COLOR_BUFFER_BIT);

    // Activate our shader program for all subsequent draw calls
    glUseProgram(shaderProgram);

    // --- Step 4: Send uniform values to the shader ---
    // Uniforms are global variables accessible by the shader on the GPU.
    // We update them every frame with the latest player position and time.
    int playerPosLoc = glGetUniformLocation(shaderProgram, "playerPos");
    int baseColorLoc = glGetUniformLocation(shaderProgram, "baseColor");
    int timeLoc = glGetUniformLocation(shaderProgram, "time");

    glUniform2f(playerPosLoc, playerNdcX,
                playerNdcY);                    // Player center for lighting
    glUniform1f(timeLoc, (float)glfwGetTime()); // Elapsed time for flicker

    // --- Step 5: Draw all objects ---
    // Each object type gets a different baseColor uniform before drawing.
    // The fragment shader uses this color + proximity to determine brightness.

    // Draw Walls — Neon Blue (R=0.0, G=0.8, B=1.0)
    glUniform3f(baseColorLoc, 0.0f, 0.8f, 1.0f);
    glBindVertexArray(wallVAO);
    glDrawArrays(GL_TRIANGLES, 0, wallVertices.size() / 3);

    // Draw Goal — Neon Green (R=0.0, G=1.0, B=0.4)
    glUniform3f(baseColorLoc, 0.0f, 1.0f, 0.4f);
    glBindVertexArray(goalVAO);
    glDrawArrays(GL_TRIANGLES, 0, goalVertices.size() / 3);

    // Draw Trail segments — Fading Magenta
    // AUTHOR: Faria Chowdhury (Trail coordinate generation and linear alpha
    // decay) The trail is a list of the player's recent positions. Older trail
    // segments get smaller and dimmer (alpha decreases).
    for (size_t i = 0; i < trail.size(); i++) {
      // Calculate linear alpha decay fade: newer segments (lower i) are
      // brighter
      float alpha = 1.0f - ((float)(i + 1) / (MAX_TRAIL_SIZE + 1));
      glUniform3f(baseColorLoc, 1.0f * alpha, 0.0f, 1.0f * alpha);

      // Convert trail grid position to NDC
      float tX = -1.0f + (trail[i].x * cellSize);
      float tY = 1.0f - (trail[i].y * cellSize);

      // Older trail segments have more padding (appear smaller)
      float trailPadding = padding + 0.04f + (i * 0.01f);

      // Build trail quad vertices
      float trailVertices[] = {tX + trailPadding,
                               tY - trailPadding,
                               0.0f,
                               tX + cellSize - trailPadding,
                               tY - trailPadding,
                               0.0f,
                               tX + trailPadding,
                               tY - cellSize + trailPadding,
                               0.0f,

                               tX + cellSize - trailPadding,
                               tY - trailPadding,
                               0.0f,
                               tX + cellSize - trailPadding,
                               tY - cellSize + trailPadding,
                               0.0f,
                               tX + trailPadding,
                               tY - cellSize + trailPadding,
                               0.0f};

      // Reuse the player VBO for trail rendering (upload + draw)
      glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(trailVertices), trailVertices);
      glBindVertexArray(playerVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Draw Player — Bright Neon Magenta (R=1.0, G=0.0, B=1.0)
    glUniform3f(baseColorLoc, 1.0f, 0.0f, 1.0f);
    // Re-upload player vertices (trail rendering overwrote the VBO)
    glBindBuffer(GL_ARRAY_BUFFER, playerVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(playerVertices), playerVertices);
    glBindVertexArray(playerVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // --- Step 6: Present the frame and poll for events ---
    glfwSwapBuffers(window); // Swap front/back buffers (double buffering)
    glfwPollEvents();        // Process pending keyboard/window events

    // --- Step 7: Check win condition ---
    // If the player is standing on a goal cell (value 2), they win.
    if (maze[playerGridY][playerGridX] == 2 && !gameWon) {
      gameWon = true;
#ifdef _WIN32
      MessageBoxA(NULL, "Level Completed! You found the exit.",
                  "Congratulations", MB_OK | MB_ICONEXCLAMATION);
#else
      std::cout << "Congratulations: Level Completed! You found the exit."
                << std::endl;
#endif
      glfwSetWindowShouldClose(window, true); // Signal the loop to exit
    }
  }

  // ==========================================================================
  // PHASE 8: Cleanup — free all GPU resources before exiting
  // ==========================================================================
  // Delete all VAOs, VBOs, and the shader program to avoid memory leaks.
  glDeleteVertexArrays(1, &wallVAO);
  glDeleteBuffers(1, &wallVBO);
  glDeleteVertexArrays(1, &goalVAO);
  glDeleteBuffers(1, &goalVBO);
  glDeleteVertexArrays(1, &playerVAO);
  glDeleteBuffers(1, &playerVBO);
  glDeleteProgram(shaderProgram);

  // Destroy the GLFW window and release system resources
  glfwTerminate();
  return 0;
}

// ============================================================================
// INPUT HANDLING & COLLISION DETECTION
// ============================================================================
// Called once per frame to read keyboard state and move the player.
//
// MOVEMENT RULES:
//   - W/A/S/D keys move the player one grid cell at a time
//   - Movement is blocked if the target cell is a wall (value 1)
//   - A 150ms cooldown prevents the player from moving too fast
//   - Each move pushes the old position onto the trail (max 8 entries)

/**
 * @brief Processes keyboard inputs and manages player movement physics.
 * @author Sabikun Nahar Alina (Collision checks & step interval cooldown)
 * @param window The GLFW window context.
 * @details Implements a 0.15s cooldown mechanism to normalize grid movement
 *          speeds independent of the high hardware frame rates.
 */
void processInput(GLFWwindow *window) {
  // ESC key: immediately close the window
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Input cooldown physics: only allow a move every 150ms (Sabikun Nahar Alina)
  float currentTime = glfwGetTime();
  if (currentTime - lastMoveTime > 0.15f) {
    int nextX = playerGridX;
    int nextY = playerGridY;

    // Read directional input (only one direction per frame)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      nextY--; // Up (row decreases)
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      nextY++; // Down (row increases)
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      nextX--; // Left (column decreases)
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      nextX++; // Right (column increases)

    // Collision check: move only if target cell is a path (0) or goal (2)
    if ((nextX != playerGridX || nextY != playerGridY) &&
        (maze[nextY][nextX] == 0 || maze[nextY][nextX] == 2)) {
      // Save current position to the trail before moving
      trail.insert(trail.begin(), {playerGridX, playerGridY});
      if (trail.size() > MAX_TRAIL_SIZE) {
        trail.pop_back(); // Remove oldest trail segment if over limit
      }

      // Apply the move
      playerGridX = nextX;
      playerGridY = nextY;
      lastMoveTime = currentTime; // Reset the cooldown timer
    }
  }
}

// ============================================================================
// WINDOW RESIZE CALLBACK
// ============================================================================
// Called automatically by GLFW whenever the user resizes the window.
// Updates the OpenGL viewport to match the new window dimensions.
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window; // Silence unused parameter warning
  glViewport(0, 0, width, height);
}