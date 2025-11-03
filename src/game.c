#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

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


void resetObjects(GameContext *ctx, Target *jar, ObstacleArray *obs)
{
  // reset scores
  ctx->totalSpeed = 0;
  ctx->timer = 30;
  ctx->score = 0;

  // reset honey jar
  jar->stuck = false;
  jar->pos = (Vector2){WIDTH / 2 + 50, 100};
  jar->hitbox = (Rectangle){jar->pos.x, jar->pos.y, jar->hitbox.width, jar->hitbox.height};

  // loop through obstacles and reset position, rect, and stuck attribute
  for (int i = 0; i < obs->length; i++) {
    Obstacle *o = &obs->items[i];
    o->stuck = false;
    o->rect = o->init;
  }
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
      if (CheckCollisionRecs(rectToHitbox(*subject, HITBOX_SHRINK_PERC), paw->hitbox)) {
        randomBearSound(sb);
        subject->stuck = true;
        ctx->score += subject->value;
        printf("SCORE: %d\n", ctx->score);
      }
    } else {
      // update Jar pos by adding mouse delta
      subject->rect.x = subject->rect.x + dt.x;
      subject->rect.y = subject->rect.y + dt.y;
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
      if (CheckCollisionRecs(rectToHitbox(*actor, HITBOX_SHRINK_PERC), subject->rect)) {
        if (actor->stuck) {
          subject->rect.x = subject->rect.x + dt->x;
          subject->rect.y = subject->rect.y + dt->y;
        }
      }
    }

    // object on honey jar logic
    if (CheckCollisionRecs(jar->hitbox, actor->rect)) {
      if (!jar->stuck) {
        jar->pos.x = jar->pos.x + dt->x;
        jar->pos.y = jar->pos.y + dt->y;
      } else {
        actor->rect.x = actor->rect.x + dt->x;
        actor->rect.y = actor->rect.y + dt->y;
      }
    }
  }
}

void handlePawPushing(Bear *b, ObstacleArray *obs, Vector2 *dt)
{
  for (int i = 0; i < obs->length; i++) {
    Obstacle *subject = &obs->items[i];

    if (CheckCollisionRecs(b->hitbox, subject->rect)) {
      subject->rect.x = subject->rect.x + dt->x;
      subject->rect.y = subject->rect.y + dt->y;
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
