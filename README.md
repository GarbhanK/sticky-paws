# Sticky Paws

> You are a bear. You need that honey.

![](.github/images/screenshot.png)

A small game made with [Raylib](https://www.raylib.com/), mainly a time killer over the winter months and a good excuse to play around with C and gamedev a little bit more.

**Development Environment:** macOS (M-series) with Zed editor and Clang compiler

## Features

- 🐻 Control a bear's paw with your mouse
- 🍯 Steal honey without getting caught
- 🎮 Sticky physics mechanics
- 🎵 Sound effects and music
- 📺 Dynamic wake-state system for the guard
- ⏱️ Time-based challenge

## How to Play

1. **Move**: Control the bear paw using your mouse cursor
2. **Objective**: Get the jar of honey to the bottom of the screen to win
3. **Challenge**: All objects will stick to your paw - plan your path carefully!
4. **Warning**: Don't wake up Walt! Moving too fast or running out of time will fail the mission

### Controls

- **Mouse**: Move the bear paw
- **SPACE**: Toggle fullscreen
- **TAB** (in-game): Toggle debug mode
- **ESC**: Exit game

## Building from Source

### Prerequisites

- CMake 3.5 or higher
- C compiler (Clang, GCC, or MSVC)
- Git

### Quick Start (Recommended)

CMake will automatically download Raylib for you:

```bash
# Clone the repository
git clone https://github.com/GarbhanK/sticky-paws.git
cd sticky-paws

# Configure the project
cmake -S . -B build

# Build the game
cmake --build build

# Run the game
./build/sticky-paws/sticky-paws
```

### Using Local Raylib

If you prefer to use a local Raylib installation:

```bash
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_RAYLIB=/path/to/raylib
cmake --build build
```

### Platform-Specific Builds

#### macOS
```bash
make build_osx
make run
```

#### Linux
```bash
make build_linux
```

#### Windows (Cross-compile)
```bash
make build_windows
```

## Development

### Code Formatting

Format all source files:
```bash
make format
```

### Linting

Run syntax check:
```bash
make lint
```

### Clean Build

Remove build artifacts:
```bash
make clean
```

## Project Structure

```
sticky-paws/
├── src/           # Source code
│   ├── main.c     # Entry point and game loop
│   ├── game.c/h   # Core game logic
│   ├── ui.c/h     # User interface
│   ├── sound.c/h  # Audio management
│   └── assets/    # Game assets (textures, sounds)
├── docs/          # Documentation
├── build/         # Build output (generated)
└── CMakeLists.txt # Build configuration
```

See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for detailed code documentation.

## License

See [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [Raylib](https://www.raylib.com/) - A simple and easy-to-use library to enjoy videogames programming
- Game assets and sound effects created for this project
