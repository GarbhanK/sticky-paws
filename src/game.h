/**
 * @file game.h
 * @brief Core game logic and data structures for Sticky Paws
 *
 * This file contains the main game state management, physics handling,
 * and collision detection for the sticky paw mechanics.
 */
#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stddef.h>

// Game configuration constants
extern const float TOTAL_SPEED_MAX;
extern const float SENSITIVITY;
extern const float DECAY;
extern const float HITBOX_SHRINK_PERC;
extern const double TIME_INTERVAL;

extern const float WIDTH;
extern const float HEIGHT;

/**
 * @brief Represents the honey jar (target object to steal)
 */
typedef struct Target {
  Texture2D tex;
  Vector2 pos;
  Rectangle hitbox;
  bool stuck;
  int value;
} Target;

/**
 * @brief Represents the bear's paw controlled by the player
 */
typedef struct Bear {
  Rectangle hitbox;
  Texture2D tex;
  Texture2D nose;
  Vector2 pos;
} Bear;

/**
 * @brief Represents an obstacle on the picnic blanket
 */
typedef struct Obstacle {
  Rectangle init; // Initial position for game reset
  Rectangle rect; // Current position and size
  bool stuck;     // Whether stuck to the paw
  int value;      // Score value when collected
  Texture2D tex;
} Obstacle;

/**
 * @brief Dynamic array of obstacles
 */
typedef struct {
  Rectangle *init;  // Array of rect positions to restart/init the game
  Obstacle *items;  // The Obstacles array
  size_t length;    // Current number of items
  size_t capapcity; // Total array capacity (NOTE: typo in original, kept for compatibility)
} ObstacleArray;

/**
 * @brief Game state enumeration
 */
typedef enum { START, PLAY, FAIL, WIN } GameState;

/**
 * @brief Main game context holding all game state
 */
typedef struct {
  // Time tracking
  double currentTime;
  double lastTime;
  double timerPrev;

  // Game state
  GameState state;
  int score;
  int timer;
  int totalSpeed;

  // State flags
  bool failStateEntered;
  bool winStateEntered;
  bool isSnoring;
  bool showTutorial;
  bool showWarning;
  bool debug;
} GameContext;

/**
 * @brief Initialize game context with default values
 * @param ctx Pointer to game context to initialize
 */
void initGameContext(GameContext *ctx);

/**
 * @brief Draw the bear paw and nose on screen
 * @param b Pointer to Bear structure to render
 */
void drawBear(Bear *b);

/**
 * @brief Handle sticky collision between paw and honey jar
 * @param ctx Game context
 * @param paw Bear paw
 * @param jar Target honey jar
 * @param sb Sound buffer array
 */
void handleStickyJar(GameContext *ctx, Bear *paw, Target *jar, Sound sb[]);

/**
 * @brief Handle sticky collision between paw and obstacles
 * @param ctx Game context
 * @param paw Bear paw
 * @param obs Array of obstacles
 * @param sb Sound buffer array
 */
void handleStickyObstacle(GameContext *ctx, Bear *paw, ObstacleArray *obs, Sound sb[]);

/**
 * @brief Handle paw pushing obstacles (currently unused)
 * @param b Bear paw
 * @param obs Array of obstacles
 * @param dt Mouse delta movement
 */
void handlePawPushing(Bear *b, ObstacleArray *obs, Vector2 *dt);

/**
 * @brief Handle objects pushing each other
 * @param obs Array of obstacles
 * @param jar Target honey jar
 * @param dt Mouse delta movement
 */
void handleObjectPushing(ObstacleArray *obs, Target *jar, Vector2 *dt);

/**
 * @brief Reset all game objects to initial positions
 * @param ctx Game context
 * @param jar Target honey jar
 * @param obs Array of obstacles
 */
void resetObjects(GameContext *ctx, Target *jar, ObstacleArray *obs);

/**
 * @brief Update speed based on mouse movement
 * @param ctx Game context
 */
void handleSpeed(GameContext *ctx);

/**
 * @brief Get the old man's wake state based on speed
 * @param speed Current speed value
 * @return State index (0-3) representing wake level
 */
int getOldManState(int speed);

/**
 * @brief Convert obstacle rectangle to a smaller hitbox
 * @param obs Obstacle to convert
 * @param shrinkFactor Percentage to shrink (0.0-1.0)
 * @return Shrunken hitbox rectangle
 */
Rectangle rectToHitbox(Obstacle obs, float shrinkFactor);

#endif // GAME_H
