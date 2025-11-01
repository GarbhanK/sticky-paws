#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

#include "game.h"
#include "sound.h"

int SCORE = 0;
const float TOTAL_SPEED_MAX = 400.0f;
const float SENSITIVITY = 3.0f;
const float DECAY = 15.0f;
const float HITBOX_SHRINK_PERC = 0.15f;
const double TIME_INTERVAL = 0.1f;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

Rectangle obstacleInit[] = {
  {200, HEIGHT / 2, 150, 150},  // grapes
  {500, HEIGHT / 3, 110, 300},  // baguette
  {300, 250, 115, 100},  // cheese
  {800, 450, 90, 130},  // cigs
  {80, 115, 400, 100},  // wine bottle
};

void drawBear(Bear *b)
{
  const float noseOffset = 150;
  const float noseMaxHeight = (HEIGHT - b->nose.height);
  const float leftLimit = (WIDTH * 0.4) - b->nose.width;
  const float rightLimit = WIDTH * 0.6;

  bool noseFollowing = true;
  float noseHeight = b->pos.y + (HEIGHT * 0.5);
  Vector2 nosePos = { b->pos.x-noseOffset, noseHeight };

  // draw bear paw
  DrawTexture(b->tex, b->pos.x, b->pos.y, WHITE);

  // check if paw pos is between limits
  if ( (b->pos.x - b->nose.width) < leftLimit) {
    noseFollowing = false;
    nosePos = (Vector2){ leftLimit, noseHeight };
  } else if ( (b->pos.x - b->nose.width) > rightLimit) {
    noseFollowing = false;
    nosePos = (Vector2){ rightLimit, noseHeight };
  }

  if (noseFollowing) {
    nosePos = (Vector2){ b->pos.x-noseOffset, noseHeight };
  }

  // limit nose position past the bottom of the texture
  if (nosePos.y <= noseMaxHeight) {
    nosePos.y = noseMaxHeight;
  };

  // draw the bear nose
  DrawTextureV(b->nose, nosePos, WHITE);
}

void resetObjects(GameContext *ctx, Target *jar, ObstacleArray *obs)
{
  // reset scores
  ctx->totalSpeed = 0;
  ctx->timer = 30;
  SCORE = 0;

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

void handleSpeed(GameContext *ctx)
{
  Vector2 dt = GetMouseDelta();
  float absMouseDelta, mouseSpeed;

  // speed increase
  if (dt.x != 0 && dt.y != 0) {
    absMouseDelta = fabs(dt.x) + fabs(dt.y);
    mouseSpeed = absMouseDelta;
    ctx->totalSpeed = ctx->totalSpeed + ((int)mouseSpeed * SENSITIVITY);
  } else if (dt.x == 0 && dt.y != 0) {
    absMouseDelta = fabs(dt.y);
    mouseSpeed = absMouseDelta;
    ctx->totalSpeed = ctx->totalSpeed + ((int)mouseSpeed * 2 * SENSITIVITY);
  } else if (dt.x != 0 && dt.y == 0) {
    absMouseDelta = fabs(dt.x);
    mouseSpeed = absMouseDelta;
    ctx->totalSpeed = ctx->totalSpeed + ((int)mouseSpeed * 2 * SENSITIVITY);
  }

  // limit the total speed
  if (ctx->totalSpeed > TOTAL_SPEED_MAX) {
    ctx->totalSpeed = TOTAL_SPEED_MAX;
    if (!ctx->debug)
      ctx->state = FAIL;
  }

  // get rid of that issue where score flashes back and forth at idle
  if (ctx->totalSpeed <= 3) {
    ctx->totalSpeed = 0;
  }
}

void handleStickyJar(Bear *paw, Target *jar, Sound sb[])
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
      if (CheckCollisionRecs(rectToHitbox(*subject, HITBOX_SHRINK_PERC), paw->hitbox)) {
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

void handleObjectPushing(ObstacleArray *obs, Target *jar, Vector2 *dt)
{
  for (int i = 0; i < obs->length; i++) {
    Obstacle *actor = &obs->items[i];

    // object on object pushing logic
    for (int j = 0; j < obs->length; j++) {
      if (i == j) {
        continue;
      } // skip if same object or object already stuck to paw

      Obstacle *subject = &obs->items[j];

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
    // Draw old man in the corner
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

Rectangle rectToHitbox(Obstacle obs, float shrinkFactor)
{
  // reduce the object's dimentions down by 25%
  Rectangle rect = obs.rect;

  float newWidth = rect.width * (1.0f - shrinkFactor);
  float newHeight = rect.height * (1.0f - shrinkFactor);
  float newX = rect.x + (rect.width - newWidth) / 2.0f;
  float newY = rect.y + (rect.height - newHeight) / 2.0f;

  Rectangle hitbox = (Rectangle){ newX, newY, newWidth, newHeight };
  return hitbox;
}
