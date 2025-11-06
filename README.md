# Snake Game - Cross-Platform Terminal Implementation


## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Technical Implementation](#technical-implementation)
- [Installation & Compilation](#installation--compilation)
- [Usage](#usage)
- [Design Patterns & Best Practices](#design-patterns--best-practices)
- [Performance Considerations](#performance-considerations)
- [Future Enhancements](#future-enhancements)
- [Team Contributors](#team-contributors)
- [License](#license)

---

## 🎮 Overview

This project presents a sophisticated implementation of the classic Snake game, developed as a cross-platform terminal application in C++. The implementation emphasizes clean code architecture, platform abstraction, and optimal rendering performance while maintaining the nostalgic essence of the original game.

### Project Context
Developed as a collaborative effort by a team of four graduate students, this project demonstrates advanced software engineering principles including:
- Cross-platform compatibility through abstraction layers
- Object-oriented design with clear separation of concerns
- Real-time input handling and efficient rendering
- Persistent data storage for game state

---

## ✨ Features

### Core Gameplay
- **Classic Snake Mechanics**: Navigate the snake to consume food and grow longer
- **Collision Detection**: Boundary and self-collision detection with immediate feedback
- **Progressive Difficulty**: Snake speed and length increase with score
- **Scoring System**: Points awarded for each food item consumed

### Technical Features
- **Cross-Platform Support**: Native implementation for Windows, Linux, and macOS
- **Terminal-Based UI**: Rich text-based interface with Unicode box-drawing characters
- **Emoji Support**: UTF-8 encoded emojis for enhanced visual appeal (🐍 🍎 🔵)
- **Persistent High Score**: Automatic saving and loading of high scores
- **Responsive Controls**: Support for both WASD and arrow key inputs
- **Flicker-Free Rendering**: Optimized double-buffering technique for smooth display
- **Graceful State Management**: Clean game over screen with restart functionality

---

## 🏗️ System Architecture

### Component Overview

```
┌─────────────────────────────────────────────┐
│            Main Game Loop                    │
│  ┌─────────────────────────────────────┐   │
│  │         GameBoard                    │   │
│  │  ┌──────────┐      ┌────────────┐   │   │
│  │  │  Snake   │      │    Food    │   │   │
│  │  └──────────┘      └────────────┘   │   │
│  │                                      │   │
│  │  ┌────────────────────────────────┐ │   │
│  │  │   Rendering Engine             │ │   │
│  │  └────────────────────────────────┘ │   │
│  └─────────────────────────────────────┘   │
│                                             │
│  ┌─────────────────────────────────────┐   │
│  │   Platform Abstraction Layer        │   │
│  │  (Windows API / POSIX)              │   │
│  └─────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

### Class Structure

#### 1. **Position Structure**
```cpp
struct Position {
    int x, y;
    bool operator==(const Position& other) const;
}
```
- Encapsulates 2D coordinate data
- Implements equality operator for collision detection
- Lightweight value type for efficient copying

#### 2. **Snake Class**
**Responsibilities:**
- Manages snake body segments using `std::deque`
- Handles directional movement logic
- Prevents illegal reverse movements
- Growth mechanics implementation

**Key Methods:**
- `setDirection()`: Validates and updates movement direction
- `move()`: Updates snake position based on current direction
- `grow()`: Extends snake length on food consumption
- `checkSelfCollision()`: Detects self-intersection
- `isOnPosition()`: Checks if position overlaps with snake body

#### 3. **Food Class**
**Responsibilities:**
- Random food generation within game boundaries
- Position management

**Key Methods:**
- `spawn()`: Generates new food at random valid position
- `getPosition()`: Returns current food coordinates

#### 4. **GameBoard Class**
**Responsibilities:**
- Game state management
- Input processing
- Update loop coordination
- Rendering pipeline
- Score tracking and persistence

**Key Methods:**
- `handleInput()`: Processes user keyboard input
- `update()`: Updates game state (movement, collisions, scoring)
- `render()`: Generates and displays game frame
- `loadHighScore()`/`saveHighScore()`: Persistent data management
- `reset()`: Reinitializes game state

---

## 🔧 Technical Implementation

### Platform Abstraction Layer

The implementation uses conditional compilation to provide platform-specific functionality:

#### Windows Implementation
- **Windows Console API**: Direct console manipulation
- **`conio.h`**: Non-blocking keyboard input via `_kbhit()` and `_getch()`
- **Code Page Management**: UTF-8 support for emoji rendering
- **Cursor Control**: HANDLE-based cursor visibility management

#### Unix/Linux/macOS Implementation
- **POSIX Terminal Control**: `termios.h` for raw mode terminal
- **ANSI Escape Sequences**: Cursor positioning and screen clearing
- **File Descriptors**: Non-blocking I/O via `ioctl` and `FIONREAD`
- **Signal Handling**: Clean terminal restoration via `atexit()`

### Rendering Optimization

**Double-Buffering Technique:**
```cpp
ostringstream buffer;
// ... build entire frame in memory ...
cout << buffer.str();
cout.flush();
```

**Benefits:**
- Eliminates screen flicker
- Reduces system calls from O(n²) to O(1) per frame
- Improves rendering consistency across platforms
- Minimizes cursor movement overhead

### Input Handling

**Non-Blocking Input Architecture:**
- Polling-based input checking via `kbhit()`
- Arrow key sequence detection and normalization
- WASD and arrow key support with unified processing
- Input buffering to prevent key drops

### Collision Detection Algorithm

**Boundary Collision:** O(1) complexity
```cpp
if (head.x < 0 || head.x >= GRID_SIZE || 
    head.y < 0 || head.y >= GRID_SIZE)
```

**Self-Collision:** O(n) complexity where n = snake length
```cpp
for (size_t i = 1; i < body.size(); i++) {
    if (body[i] == head) return true;
}
```

### Data Persistence

**High Score Storage:**
- Simple file I/O using `fstream`
- Automatic save on game over
- Graceful handling of missing files
- ASCII format for human readability

---

## 📦 Installation & Compilation

### Prerequisites
- C++11 compatible compiler (GCC 4.8+, Clang 3.3+, MSVC 2015+)
- Terminal with UTF-8 support (for emoji rendering)

### Compilation Instructions

#### Windows (MinGW/MSVC)
```bash
# Using g++
g++ -std=c++11 main.cpp -o snake.exe

# Using MSVC
cl /EHsc /std:c++11 main.cpp /Fe:snake.exe
```

#### Linux/macOS
```bash
# Using g++
g++ -std=c++11 main.cpp -o snake

# Using clang
clang++ -std=c++11 main.cpp -o snake

# Make executable
chmod +x snake
```

### CMake Build (Optional)

Create a `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(snake main.cpp)
```

Build:
```bash
mkdir build && cd build
cmake ..
make
```

---

## 🎯 Usage

### Running the Game
```bash
# Windows
snake.exe

# Linux/macOS
./snake
```

### Controls

| Input | Action |
|-------|--------|
| **W** / **↑** | Move Up |
| **S** / **↓** | Move Down |
| **A** / **←** | Move Left |
| **D** / **→** | Move Right |
| **R** | Restart Game (after game over) |
| **Q** | Quit Game |

### Gameplay Instructions

1. **Objective**: Guide the snake (🐍) to consume apples (🍎)
2. **Growth**: Each apple increases snake length and adds 10 points
3. **Avoid**: Colliding with walls or your own body (🔵)
4. **Win Condition**: Achieve the highest score possible
5. **High Score**: Automatically saved between sessions

---

## 🎨 Design Patterns & Best Practices

### Design Patterns Implemented

#### 1. **Separation of Concerns**
- Each class has a single, well-defined responsibility
- Game logic separated from rendering
- Platform-specific code isolated in abstraction layer

#### 2. **Encapsulation**
- Private data members with public interfaces
- Const correctness for read-only operations
- Information hiding principle enforced

#### 3. **Strategy Pattern (Implicit)**
- Platform-specific implementations selected at compile-time
- Uniform interface across different operating systems

#### 4. **State Pattern**
- Game state management (playing, game over)
- State-dependent rendering and input handling

### Code Quality Practices

- **RAII Principles**: Automatic terminal cleanup via `atexit()`
- **Const Correctness**: Read-only methods marked as `const`
- **Memory Safety**: No raw pointers, uses STL containers
- **Enum for Type Safety**: Direction enumeration prevents invalid states
- **Defensive Programming**: Input validation and bounds checking

---

## ⚡ Performance Considerations

### Time Complexity Analysis

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Snake Movement | O(1) | Deque push/pop operations |
| Self-Collision Check | O(n) | n = snake length |
| Food Position Check | O(n) | Linear search through body |
| Rendering | O(w×h) | w×h = grid dimensions (20×20 = 400) |

### Space Complexity
- **Snake Storage**: O(n) where n = snake length (max 400)
- **Render Buffer**: O(w×h) for frame buffer
- **Overall**: O(n + w×h)

### Optimization Strategies

1. **String Stream Buffering**: Single I/O operation per frame
2. **Minimal System Calls**: Platform-specific optimizations
3. **Fixed Grid Size**: Compile-time constant enables optimizations
4. **Deque Data Structure**: O(1) front/back operations

---

## 🚀 Future Enhancements

### Proposed Features

#### Gameplay Enhancements
- [ ] Multiple difficulty levels with varying speeds
- [ ] Power-ups (speed boost, slow motion, invincibility)
- [ ] Obstacles and maze modes
- [ ] Two-player competitive mode
- [ ] AI-controlled opponent

#### Technical Improvements
- [ ] Spatial hashing for O(1) collision detection
- [ ] Configuration file for customizable settings
- [ ] Sound effects using cross-platform audio library
- [ ] Graphical version using SDL2 or SFML
- [ ] Online leaderboard with network functionality
- [ ] Replay system for reviewing gameplay

#### Code Improvements
- [ ] Unit testing framework (Google Test)
- [ ] Continuous Integration setup
- [ ] Code coverage analysis
- [ ] Profiling and performance benchmarking
- [ ] Documentation generation (Doxygen)

---

## 👥 Team Contributors

This project was developed collaboratively by a team of four graduate students:

| Name | ID |
|------|------|
| **[Manushri Thakkar]** | 202512122 |  
| **[Ruchita Patadiya]** | 202512070 |  
| **[Piyush Tanwani]** | 202512021 |  
| **[Aum Parmar]** | 202512004 |  

### Individual Contributions

- **Architecture & Design**: Object-oriented structure, class hierarchy
- **Platform Abstraction**: Windows and Unix/Linux compatibility layers
- **Rendering System**: Flicker-free double-buffering implementation
- **Game Logic**: Snake mechanics, collision detection, scoring
- **Testing & Documentation**: Cross-platform testing, README documentation

