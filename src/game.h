#include <stddef.h>
#include <stdbool.h>
#include <raylib.h>
#include <raymath.h>

#include "bear.h"

extern bool DEBUG;
extern int SCORE;
extern int TIMER;
extern int TOTAL_SPEED;
extern float TOTAL_SPEED_MAX;
extern float SENSITIVITY;
extern float DECAY;
extern double TIME_INTERVAL;

extern const float WIDTH;
extern const float HEIGHT;


typedef struct Honey {
    Texture2D tex;
    Vector2 pos;
    Rectangle hitbox;
    bool stuck;
    int value;
} Honey;

typedef struct Obstacle {
    Rectangle rect;
    bool stuck;
    int value;
    Texture2D tex;
} Obstacle;

// https://youtu.be/_KSKH8C9Gf0?si=mmUkxPDIZce6YNlD
typedef struct {
    Obstacle* items;    // the Obstacles array
    Rectangle* init;    // array of rect positions to restart/init the game
    size_t length;      // current no. items
    size_t capapcity;   // total arr capacity
} ObstacleArray;

enum GAMESTATE {
    START,
    PLAY,
    FAIL,
    WIN
} GAMESTATE;

// the starting positions of the obstacles
Rectangle obstacleInit[4];

// declare functions
void handleStickyJar(Bear *paw, Honey *jar, Sound sb[]);
void handleStickyObstacle(Bear *paw, Obstacle obs[], size_t arrLen, Sound sb[]);
void handlePawPushing(Bear *b, Obstacle obs[], size_t arrLen, Vector2 *dt);
void handleObjectPushing(Obstacle obs[], size_t arrLen, Honey *jar, Vector2 *dt);
void resetObjects(Honey *jar, Obstacle obs[], size_t arrLen);
void handleSpeed();
