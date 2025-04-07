#include "bear.h"

#include "raylib.h"

extern float WIDTH, HEIGHT;

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
