# Sticky Paws Architecture

## Overview

Sticky Paws is a simple 2D game built with Raylib in C. The player controls a bear's paw attempting to steal honey from a picnic while avoiding detection by an old man watching TV.

## Code Structure

### Core Modules

#### `main.c`
- Entry point and main game loop
- State machine implementation (START, PLAY, FAIL, WIN)
- Initialization and cleanup
- Frame-by-frame updates and rendering

#### `game.c/h`
- Core game logic and mechanics
- Physics calculations
- Collision detection (sticky mechanics)
- Object interactions
- Speed tracking and management

#### `ui.c/h`
- User interface rendering
- Button interactions
- Text overlays
- Fade effects
- Tutorial display

#### `sound.c/h`
- Audio management
- Sound loading and validation
- Playback control
- Random sound selection

## Game Loop Flow

```
Initialize Window & Audio
    ↓
Load Assets (Textures, Sounds)
    ↓
Initialize Game Context
    ↓
┌─────────────────────────────┐
│ Main Loop (until close)     │
│                             │
│  1. Update Fade Effect      │
│  2. Handle State            │
│     - START: Menu & Setup   │
│     - PLAY: Game Logic      │
│     - FAIL: Game Over       │
│     - WIN: Victory Screen   │
│  3. Render Current State    │
│  4. EndDrawing()            │
└─────────────────────────────┘
    ↓
Cleanup (Unload Assets)
    ↓
Close Window & Audio
```

## Key Game Mechanics

### Sticky Physics
- Objects stick to the bear's paw on collision
- Stuck objects move with mouse delta
- Objects can push each other when stuck
- Collision uses shrunken hitboxes for better feel

### Speed System
- Mouse movement accumulates speed
- Speed decays over time
- High speed triggers fail state
- Visual and audio feedback based on speed thresholds

### Timer System
- Countdown timer in PLAY state
- Timer expiration triggers fail state
- Reset on game restart

## Data Structures

### GameContext
Central structure holding all game state:
- Time tracking
- Game state enum
- Score and timer
- Boolean flags for state transitions

### Bear
Player-controlled paw:
- Position and texture
- Hitbox for collision
- Nose texture (follows paw)

### Target
Honey jar (win condition):
- Position and texture
- Sticky state
- Score value

### Obstacle
Picnic items that stick to paw:
- Initial and current positions
- Sticky state
- Collision rectangle
- Texture

### UserInterface
All UI elements and textures:
- Buttons
- Background images
- Wake state textures
- Fade effect

## Constants and Configuration

- `WIDTH` / `HEIGHT`: Window dimensions (1024x768)
- `TOTAL_SPEED_MAX`: Maximum speed before fail (400.0f)
- `SENSITIVITY`: Speed accumulation factor (3.0f)
- `DECAY`: Speed reduction per interval (15.0f)
- `HITBOX_SHRINK_PERC`: Collision box reduction (0.15f)

## Build System

The project uses CMake as the primary build system:
- Automatically fetches Raylib dependency
- Configures platform-specific linking
- Copies assets to build directory
- Supports Web, macOS, Linux, and Windows

Alternative Makefile provided for simpler local builds.

## Asset Organization

```
src/assets/
├── images/
│   ├── UI screens
│   ├── Game objects
│   └── Character sprites
└── sfx/
    ├── Bear sounds
    ├── UI sounds
    └── Ambient sounds
```

## Future Considerations

- Memory management improvements
- Additional obstacle types
- More complex physics interactions
- Gameplay options menu
- High score tracking
