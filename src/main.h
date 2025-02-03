#include <stddef.h>
#include <stdbool.h>
#include <raylib.h>
#include <raymath.h>

#include "bear.h"

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

typedef struct {
    Sound growl1;
    Sound growl2;
    Sound growl3;
    Sound door_slam;
    Sound stick;
    Sound drag;
} SoundBank;

typedef enum {
    GROWL1,
    GROWL2,
    GROWL3,
    DOOR_SLAM,
    STICK,
    DRAG,
    SOUND_COUNT,
} SoundID;

enum GAMESTATE {
    START,
    PLAY,
    FAIL,
    WIN
} GAMESTATE;

#define MAX_SOUNDS SOUNDS_COUNT

// declare functions
void handleStickyJar(Bear *paw, Honey *jar, Sound sb[]);
void handleStickyObstacle(Bear *paw, Obstacle obs[], size_t arrLen, Sound sb[]);
void handlePawPushing(Bear *b, Obstacle obs[], size_t arrLen, Vector2 *dt);
void handleObjectPushing(Obstacle obs[], size_t arrLen, Honey *jar, Vector2 *dt);
void resetObjects(Honey *jar, Obstacle obs[], size_t arrLen);
void handleSpeed();
void randomBearSound(Sound sb[]);
void loadSounds();
