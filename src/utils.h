#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
// #include "game.h"

const char *getAssetPath(const char *filename);
Rectangle rectToHitbox(Rectangle rect, float shrinkFactor);

#endif // UTILS_H
