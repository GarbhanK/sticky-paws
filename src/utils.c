#include "raylib.h"
// #include "game.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>


Rectangle rectToHitbox(Rectangle rect, float shrinkFactor)
{
  // Create a smaller hitbox centered within the obstacles visual bounds
  // This makes collision feel more forgiving an natural

  float newWidth = rect.width * (1.0f - shrinkFactor);
  float newHeight = rect.height * (1.0f - shrinkFactor);

  // Center the shrunken hitbox within original rectangle
  float newX = rect.x + (rect.width - newWidth) / 2.0f;
  float newY = rect.y + (rect.height - newHeight) / 2.0f;

  Rectangle hitbox = (Rectangle){newX, newY, newWidth, newHeight};
  return hitbox;
}

const char *getAssetPath(const char *filename)
{
  static char fullPath[PATH_MAX];
  static char basePath[PATH_MAX];
  static bool initialized = false;
  static bool inBundle = false;

  if (!initialized) {
#ifdef __APPLE__
    const char *appDir = GetApplicationDirectory();
    printf("appDir: %s\n", appDir);
    snprintf(basePath, sizeof(basePath), "%s../Resources/assets/", appDir);
    printf("snprintf: %s../Resources/assets/\n", appDir);
    inBundle = DirectoryExists(basePath);
#endif
    if (!inBundle) {
      strcpy(basePath, "assets/");
    }
    initialized = true;
  }

  snprintf(fullPath, sizeof(fullPath), "%s%s", basePath, filename);
  printf("fullpath: %s\n", fullPath);
  return fullPath;
}
