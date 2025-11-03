#ifndef UTILS_H
#define UTILS_H

#include "raylib.h"
#include "game.h"

const char *getAssetPath(const char *filename);
Rectangle rectToHitbox(Obstacle obs, float shrinkFactor);

#endif // UTILS_H
