#include <raylib.h>

typedef struct Bear {
  Rectangle hitbox;
  Texture2D tex;
  Texture2D nose;
  Vector2 pos;
} Bear;

void drawBear(Bear *b);
