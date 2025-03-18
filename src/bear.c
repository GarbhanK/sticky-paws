#include "bear.h"

#include "raylib.h"

extern float WIDTH, HEIGHT;

void drawBear(Bear *b) {
  // draw bear paw
  DrawTexture(b->tex, b->pos.x, b->pos.y, WHITE);

  float noseThreshold = HEIGHT - b->nose.height;
  Vector2 nosePos = {(WIDTH / 2) - 150, b->pos.y + HEIGHT * 0.5};
  // TODO: maybe use below nose movement logic instead?
  // Vector2 nosePos = { (Paw.pos.x-150), Paw.pos.y + HEIGHT*0.50 };

  // limit nose position past the bottom of the texture
  if (nosePos.y <= noseThreshold) {
    nosePos.y = noseThreshold;
  };
  DrawTextureV(b->nose, nosePos, WHITE);
}
