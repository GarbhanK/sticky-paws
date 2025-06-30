
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "sound.h"

bool DEBUG = false;
int SCORE = 0;
int TIMER = 15;
int TOTAL_SPEED = 0;
const float TOTAL_SPEED_MAX = 400.0f;
const float SENSITIVITY = 3.0f;
const float DECAY = 15.0f;
const double TIME_INTERVAL = 0.1f;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

Rectangle obstacleInit[] = {
    {200, HEIGHT / 2, 150, 150},  // grapes
    {500, HEIGHT / 3, 110, 300},  // baguette
    {300, 250, 100, 90},  // cheese
    {800, 450, 90, 130},  // cigs
    {80, 125, 400, 100},  // wine bottle
};

void resetObjects(Honey *jar, ObstacleArray *obs)
{
  // reset scores
  TOTAL_SPEED = 0;
  SCORE = 0;
  TIMER = 30;

  // reset honey jar
  jar->stuck = false;
  jar->pos = (Vector2){WIDTH / 2 + 50, 100};
  jar->hitbox = (Rectangle){jar->pos.x, jar->pos.y, jar->hitbox.width, jar->hitbox.height};

  // loop through obstacles and set to original x/y
  for (int i = 0; i < obs->length; i++) {
    Obstacle *o = &obs->items[i];
    o->stuck = false;
    o->rect = obstacleInit[i];
  }
}

void handleSpeed()
{
  Vector2 dt = GetMouseDelta();
  float absMouseDelta, mouseSpeed;

  // speed increase
  if (dt.x != 0 && dt.y != 0) {
    absMouseDelta = fabs(dt.x) + fabs(dt.y);
    mouseSpeed = absMouseDelta;
    TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed * SENSITIVITY);
  } else if (dt.x == 0 && dt.y != 0) {
    absMouseDelta = fabs(dt.y);
    mouseSpeed = absMouseDelta;
    TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed * 2 * SENSITIVITY);
  } else if (dt.x != 0 && dt.y == 0) {
    absMouseDelta = fabs(dt.x);
    mouseSpeed = absMouseDelta;
    TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed * 2 * SENSITIVITY);
  }

  // limit the total speed
  if (TOTAL_SPEED > TOTAL_SPEED_MAX) {
    TOTAL_SPEED = TOTAL_SPEED_MAX;
    if (!DEBUG)
      GAMESTATE = FAIL;
  }

  // get rid of that issue where score flashes back and forth at idle
  if (TOTAL_SPEED <= 3) {
    TOTAL_SPEED = 0;
  }
}

void handleStickyJar(Bear *paw, Honey *jar, Sound sb[])
{
  Vector2 dt = GetMouseDelta();
  if (!jar->stuck) {
    if (CheckCollisionRecs(jar->hitbox, paw->hitbox)) {
      randomBearSound(sb);
      jar->stuck = true;
      SCORE += jar->value;
      printf("SCORE: %d\n", SCORE);
    }
  } else {
    // update Jar pos by adding mouse delta
    jar->pos.x = jar->pos.x + dt.x;
    jar->pos.y = jar->pos.y + dt.y;
  }
}

void handleStickyObstacle(Bear *paw, ObstacleArray *obs, Sound sb[])
{
  Vector2 dt = GetMouseDelta();
  for (int i = 0; i < obs->length; i++) {
    Obstacle *subject = &obs->items[i];

    // obstacle sticky logic
    if (!subject->stuck) {
      if (CheckCollisionRecs(subject->rect, paw->hitbox)) {
        randomBearSound(sb);
        subject->stuck = true;
        SCORE += subject->value;
        printf("SCORE: %d\n", SCORE);
      }
    } else {
      // update Jar pos by adding mouse delta
      subject->rect.x = subject->rect.x + dt.x;
      subject->rect.y = subject->rect.y + dt.y;
    }
  }
}

void handleObjectPushing(ObstacleArray *obs, Honey *jar, Vector2 *dt)
{
  for (int i = 0; i < obs->length; i++) {
    Obstacle *actor = &obs->items[i];

    // object on object pushing logic
    for (int j = 0; j < obs->length; j++) {
      if (i == j) {
        continue;
      } // skip if same object or object already stuck to paw

      Obstacle *subject = &obs->items[j];

      if (CheckCollisionRecs(actor->rect, subject->rect)) {
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

int getOldManState()
{
    int state = 0;
    // Draw old man in the corner
    if (TOTAL_SPEED >= 0) {
      state = 0;
    }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX * 0.3) {
      state = 1;
    }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX * 0.5) {
      state = 2;
    }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX * 0.8) {
      state = 3;
    }

    return state;
}
