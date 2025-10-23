#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stddef.h>

extern bool DEBUG;
extern bool SHOW_TUTORIAL;
extern int SCORE;
extern int TIMER;
extern int TOTAL_SPEED;
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
  bool stuck;
  int value;
} Target;

typedef struct Bear {
  Rectangle hitbox;
  Texture2D tex;
  Texture2D nose;
  Vector2 pos;
} Bear;

// TODO: Add stage hazards, instant fail when touched
// typedef struct Hazard {
//   Texture2D tex;
//   Rectangle hitbox;
//   Vector2 pos;
// } Hazard;

typedef struct Obstacle {
  Rectangle rect;
  bool stuck;
  int value;
  Texture2D tex;
} Obstacle;

// https://youtu.be/_KSKH8C9Gf0?si=mmUkxPDIZce6YNlD
typedef struct {
  Rectangle *init;  // array of rect positions to restart/init the game
  Obstacle *items;  // the Obstacles array
  size_t length;    // current no. items
  size_t capapcity; // total arr capacity
} ObstacleArray;

enum GameState { START, PLAY, FAIL, WIN } GAMESTATE;

// the starting positions of the obstacles
Rectangle obstacleInit[5];

// declare functions
void drawBear(Bear *b);
void handleStickyJar(Bear *paw, Target *jar, Sound sb[]);
void handleStickyObstacle(Bear *paw, ObstacleArray *obs, Sound sb[]);
void handlePawPushing(Bear *b, ObstacleArray *obs, Vector2 *dt);
void handleObjectPushing(ObstacleArray *obs, Target *jar, Vector2 *dt);
void resetObjects(Target *jar, ObstacleArray *obs);
void handleSpeed();
int getOldManState();
Rectangle rectToHitbox(Obstacle obs, float shrinkFactor);
