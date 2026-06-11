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

typedef struct Target {
  Texture2D tex;
  Vector2 pos;
  Rectangle hitbox;
  int value;
  bool stuck;
} Target;

typedef struct Bear {
  Texture2D tex;
  Vector2 pos;
  Rectangle hitbox;
  Texture2D nose;
} Bear;

// TODO: Add stage hazards, instant fail when touched
// typedef struct Hazard {
//   Texture2D tex;
//   Rectangle hitbox;
//   Vector2 pos;
// } Hazard;

typedef struct Obstacle {
  Texture2D tex;
  Rectangle init;    // Initial pos for game reset
  Rectangle hitbox;  // Current position and size
  Vector2 winPos;    // Position on win screen
  int value;         // Score value when collided
  bool stuck;        // Whether stuck to the paw
} Obstacle;

// https://youtu.be/_KSKH8C9Gf0?si=mmUkxPDIZce6YNlD
typedef struct {
  Obstacle *items;  // the Obstacles array
  size_t length;    // current no. items
  size_t capacity; // total arr capacity
} ObstacleArray;

typedef enum {
  START,
  PLAY,
  FAIL,
  WIN
} GameState;

typedef struct {
  // entities
  Bear *player;
  Target *jar;
  ObstacleArray *obs;
  ObstacleArray *stuckObstacles;

  // time tracking
  double currentTime;
  double lastTime;
  double timerPrev;

  // game state
  GameState state;
  int score;
  int timer;
  int totalSpeed;

  // flags
  bool failStateEntered;
  bool winStateEntered;
  bool isSnoring;
  bool showTutorial;
  bool showWarning;
  bool debug;
} GameContext;

// initialisation
void initGameContext(GameContext *ctx);
ObstacleArray *buildStuckObstacles(ObstacleArray *obs);
void freeObstacleArray(ObstacleArray **arr);

// game logic
void handleStickyJar(GameContext *ctx, Bear *paw, Target *jar, Sound sb[]);
void handleStickyObstacle(GameContext *ctx, Bear *paw, ObstacleArray *obs, Sound sb[]);
void handlePawPushing(Bear *b, ObstacleArray *obs, Vector2 *dt);
void handleObjectPushing(ObstacleArray *obs, Target *jar, Vector2 *dt);
void resetObjects(GameContext *ctx);
void handleSpeed(GameContext *ctx);

// queries
int getOldManState(int speed);

#endif // GAME_H
