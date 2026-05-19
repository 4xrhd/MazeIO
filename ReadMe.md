<div align="center">

# 🔦 MazeIO

**An atmospheric, grid-based 2D exploration game built with modern OpenGL 3.3 Core Profile.**

[![Platform: Linux & Windows](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-blue.svg)](#)
[![OpenGL: 3.3 Core](https://img.shields.io/badge/OpenGL-3.3%20Core-5586A4.svg)](#)
[![Language: C++](https://img.shields.io/badge/Language-C++17-00599C.svg)](#)
[![Course: CSE 358](https://img.shields.io/badge/Course-CSE%20358-FF6F00.svg)](#)

<!-- 📸 MAIN DEMO VIDEO / GIF PLACEHOLDER -->
> *Replace the image below with your gameplay demo gif or video*
> <br>
> `![Gameplay Demo](docs/assets/gameplay_demo.gif)`

</div>

---

## 📖 Overview

**MazeIO** is a top-down maze explorer that challenges players by severely limiting their visibility. Unlike traditional maze games where the entire map is visible, MazeIO casts the player in total darkness. The player emits a localized "flashlight" cone that reveals walls only when in close proximity. 

The game was developed as part of the **Computer Graphics and Multimedia (CSE 358)** course at the **University of Information Technology and Sciences (UITS)** to demonstrate proficiency in modern shader-based graphics programming.

---

## ✨ Features & Visuals

### 1. Dynamic Proximity Lighting
The game uses a custom **Fragment Shader** to calculate the Euclidean distance between the player and surrounding walls. By applying a `smoothstep` interpolation, the light fades out naturally into the darkness.

<!-- 📸 LIGHTING IMAGE PLACEHOLDER -->
> `![Proximity Lighting](docs/assets/lighting_screenshot.png)`

### 2. Atmospheric Flicker Effect
To make the light feel organic (like an unstable torch), the shader modulates light intensity using mathematical trigonometric waves ($\sin$ and $\cos$ over time), producing a subtle $\pm 5\%$ flicker.

### 3. Fading Light Trails
As the player moves, the system records historical coordinates and streams them to the GPU. Older positions fade out using a mathematical linear alpha decay ($\alpha_i = 1.0 - \frac{i+1}{N+1}$), creating a neon trail that conveys a sense of speed.

<!-- 📸 TRAIL IMAGE PLACEHOLDER -->
> `![Trail Effect](docs/assets/trail_effect_screenshot.png)`

### 4. Grid-Based Collision Physics
A $10 \times 10$ spatial array governs the map. The CPU performs look-ahead bounds checking to prevent movement through walls, alongside a $0.15\text{s}$ movement cooldown to normalize input velocity independently of the frame rate.

---

## 🛠️ Technical Stack

- **Language:** Modern C++
- **Graphics API:** OpenGL 3.3 Core Profile
- **Window Management & Input:** [GLFW 3](https://www.glfw.org/)
- **OpenGL Loader:** [GLAD](https://glad.dav1d.de/)
- **Build System:** GNU Make

---

## 🎮 Controls

| Key | Action |
| :---: | :--- |
| **`W`** | Move Up |
| **`A`** | Move Left |
| **`S`** | Move Down |
| **`D`** | Move Right |
| **`ESC`** | Quit Game |

---

## 🚀 Setup & Installation

### Windows (MinGW/MSYS2)

1. **Prerequisites**: Ensure you have MSYS2 installed with `base-devel` and `gcc` packages, along with OpenGL 3.3 compatible graphics drivers.
2. **Libraries**: Verify that `glfw3.dll` is present in both the `build/` and `lib/` directories.
3. **Compile**: Open a terminal in the project root and run:
   ```cmd
   mingw32-make
   ```
   *(Alternatively, execute the provided `make.bat` script)*
4. **Run**:
   ```cmd
   mingw32-make run
   ```

### Linux (GCC)

1. **Prerequisites**: Install the necessary development tools.
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential libglfw3-dev
   ```
2. **Compile**:
   ```bash
   make
   ```
3. **Run**:
   ```bash
   make run
   ```

---

## 📂 Project Structure

```text
MazeIO/
├── src/                  # Source files (.cpp, .c)
│   ├── main.cpp          # Main game loop, rendering, and logic
│   └── glad.c            # OpenGL loader implementation
├── include/              # Header files (.h)
│   ├── glad.h
│   └── glfw3.h
├── lib/                  # Compiled external libraries (GLFW)
├── build/                # Output directory for executables
├── docs/                 # Documentation and assets (Images/GIFs go here)
├── Makefile              # Cross-platform build script
├── MazeIO_Project_Report.md # Formal Academic Project Report
└── contribution_details.md  # Team Presentation Script
```

---

## 👥 Meet the Team

Developed by students of the **Department of CSE**, Spring 2026 Intake:

* **Kazi Md Azhar Uddin Abir** *(0432320005101120)* – Core OpenGL & Shader Programming
* **Ahamad Abdali Khan** *(0432320005101118)* – Physics, Collisions & Build Engine
* **Sabikun Nahar Alina** *(0432320005101016)* – Grid Math & Trail Arrays
* **Faria Chowdhury** *(0432220005101042)* – UI Neon Tuning, Modulation & Docs

*Supervised by Lab Instructors: Any Chowdhury (AC) & Audity Ghosh*

---
<div align="center">
  <sub>Built with ❤️ for CSE 358: Computer Graphics and Multimedia</sub>
</div>
