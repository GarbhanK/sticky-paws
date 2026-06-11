#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include "game.h"
#include "sound.h"
#include "utils.h"

// Game configuration constants
const float TOTAL_SPEED_MAX = 400.0f;
const float SENSITIVITY = 3.0f;
const float DECAY = 15.0f;
const float HITBOX_SHRINK_PERC = 0.15f;
const double TIME_INTERVAL = 0.1f;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

// Initialize with sensible defaults
void initGameContext(GameContext *ctx)
{
  ctx->currentTime = 0.0;
  ctx->lastTime = 0.0;
  ctx->timerPrev = 0.0;
  ctx->state = START;
  ctx->timer = 60;  // or whatever your starting timer is
  ctx->totalSpeed = 0.0f;
  ctx->failStateEntered = false;
  ctx->winStateEntered = false;
  ctx->isSnoring = false;
  ctx->showTutorial = false;
  ctx->showWarning = false;
  ctx->debug = false;
  ctx->player = NULL;
  ctx->jar = NULL;
  ctx->obs = NULL;
  ctx->stuckObstacles = NULL;
}

void resetObjects(GameContext *ctx)
{
  Target *jar = ctx->jar;
  ObstacleArray *obs = ctx->obs;

  // reset scores
  ctx->totalSpeed = 0;
  ctx->timer = 30;
  ctx->score = 0;

  // reset honey jar
  ctx->jar->stuck = false;
  ctx->jar->pos = (Vector2){WIDTH / 2 + 50, 100};
  ctx->jar->hitbox = (Rectangle){jar->pos.x, jar->pos.y, jar->hitbox.width, jar->hitbox.height};

  // loop through obstacles and reset position, rect, and stuck attribute
  for (int i = 0; i < obs->length; i++) {
    // get pointer to the obstacle at index i
    Obstacle *o = &obs->items[i];
    o->stuck = false;
    o->hitbox = o->init;  // reset hitbox to initial position
  }

  // free objects in the stuck obstacles array
  freeObstacleArray(&ctx->stuckObstacles);  // safe even if already NULL
}

void handleSpeed(GameContext *ctx)
{
  Vector2 dt = GetMouseDelta();

  // get the magnitude so diagonals aren't faster
  float mouseSpeed = Vector2Length(dt);

  // Scale by sensitivity and add to total speed
  ctx->totalSpeed += (int)(mouseSpeed * SENSITIVITY);

  // Clamp the total speed
  if (ctx->totalSpeed > TOTAL_SPEED_MAX) {
    ctx->totalSpeed = TOTAL_SPEED_MAX;
    if (!ctx->debug) ctx->state = FAIL;
  }

  // Prevent tiny jitter from registering as movement
  if (ctx->totalSpeed <= 3) {
    ctx->totalSpeed = 0;
  }
}

void handleStickyJar(GameContext *ctx, Bear *paw, Target *jar, Sound sb[])
{
  Vector2 dt = GetMouseDelta();
  if (!jar->stuck) {
    if (CheckCollisionRecs(jar->hitbox, paw->hitbox)) {
      randomBearSound(sb);
      jar->stuck = true;
      ctx->score += jar->value;
      printf("SCORE: %d\n", ctx->score);
    }
  } else {
    // update Jar pos by adding mouse delta
    jar->pos.x = jar->pos.x + dt.x;
    jar->pos.y = jar->pos.y + dt.y;
  }
}

void handleStickyObstacle(GameContext *ctx, Bear *paw, ObstacleArray *obs, Sound sb[])
{
  Vector2 dt = GetMouseDelta();
  for (int i = 0; i < obs->length; i++) {
    Obstacle *subject = &obs->items[i];

    // obstacle sticky logic
    if (!subject->stuck) {
      if (CheckCollisionRecs(rectToHitbox(subject->hitbox, HITBOX_SHRINK_PERC), paw->hitbox)) {
        randomBearSound(sb);
        subject->stuck = true;
        ctx->score += subject->value;
        printf("SCORE: %d\n", ctx->score);
      }
    } else {
      // update Jar pos by adding mouse delta
      subject->hitbox.x = subject->hitbox.x + dt.x;
      subject->hitbox.y = subject->hitbox.y + dt.y;
    }
  }
}

void handleObjectPushing(ObstacleArray *obs, Target *jar, Vector2 *dt)
{
  // check each obstacle against others to handle pushing interactions
  for (int i = 0; i < obs->length; i++) {
    Obstacle *actor = &obs->items[i];

    // object on object pushing logic
    for (int j = 0; j < obs->length; j++) {
      if (i == j) {
        continue;
      } // Skip self-collision check

      Obstacle *subject = &obs->items[j];

      // If a stuck object collides with another, push it
      if (CheckCollisionRecs(rectToHitbox(actor->hitbox, HITBOX_SHRINK_PERC), subject->hitbox)) {
        if (actor->stuck) {
          subject->hitbox.x = subject->hitbox.x + dt->x;
          subject->hitbox.y = subject->hitbox.y + dt->y;
        }
      }
    }

    // object on honey jar logic
    if (CheckCollisionRecs(jar->hitbox, actor->hitbox)) {
      if (!jar->stuck) {
        jar->pos.x = jar->pos.x + dt->x;
        jar->pos.y = jar->pos.y + dt->y;
      } else {
        actor->hitbox.x = actor->hitbox.x + dt->x;
        actor->hitbox.y = actor->hitbox.y + dt->y;
      }
    }
  }
}

void handlePawPushing(Bear *b, ObstacleArray *obs, Vector2 *dt)
{
  for (int i = 0; i < obs->length; i++) {
    Obstacle *subject = &obs->items[i];

    if (CheckCollisionRecs(b->hitbox, subject->hitbox)) {
      subject->hitbox.x = subject->hitbox.x + dt->x;
      subject->hitbox.y = subject->hitbox.y + dt->y;
    }
  }
}

int getOldManState(int speed)
{
    int state = 0;
    // Determine old man's wake state based on current speed
    // States: 0 = Asleep, 1 = Stirring, 2 = Alert, 3 = Awake
    if (speed >= 0) {
      state = 0;
    }
    if (speed >= TOTAL_SPEED_MAX * 0.3) {
      state = 1;
    }
    if (speed >= TOTAL_SPEED_MAX * 0.5) {
      state = 2;
    }
    if (speed >= TOTAL_SPEED_MAX * 0.8) {
      state = 3;
    }

    return state;
}

// Builds an ObstacleArray of stuck obstacles from the given array of obstacles
ObstacleArray *buildStuckObstacles(ObstacleArray *obs) {
  // Allocate memory for the stuck obstacle array
  ObstacleArray *stuck = malloc(sizeof(ObstacleArray));
  stuck->length = 0;
  stuck->capacity = obs->length;
  stuck->items = malloc(stuck->capacity * sizeof(Obstacle));

  // Copy stuck obstacles from the input array to the stuck obstacle array
  for (int i = 0; i < obs->length; i++) {
    Obstacle o = obs->items[i];
    if (o.stuck) {
      stuck->items[stuck->length++] = o;
    }
  }

  return stuck;
}

void freeObstacleArray(ObstacleArray **arr) {
    if (*arr == NULL) return;
    free((*arr)->items);
    free(*arr);
    *arr = NULL;  // prevents double-free
}
