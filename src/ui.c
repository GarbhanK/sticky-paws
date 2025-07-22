#include <string.h>
#include <raylib.h>
#include <stdbool.h>

#include "ui.h"
#include "game.h"

extern int TOTAL_SPEED, TIMER;
extern bool DEBUG;
extern bool SHOW_TUTORIAL;
extern const float TOTAL_SPEED_MAX;

void drawUI(UserInterface *ui, bool warning, int barWidth)
{
  // draw the old man in the corner
  DrawTextureEx(ui->wakeStates[getOldManState()], (Vector2){0,HEIGHT-250}, 0, 1, WHITE);

  // speed bar update logic
  ui->barWidth = TOTAL_SPEED;

  // put max limit on the width
  if (ui->barWidth > ui->barMax) {
    ui->barWidth = ui->barMax;
  }

  // set flag for warning message
  (TOTAL_SPEED >= ui->barMax / 2) ? (warning = true) : (warning = false);

  // draw speed indicator in top left
  DrawRectangleRec(ui->infoBox, WHITE);                     // background box
  DrawRectangleLinesEx(ui->infoBox, 5, RED);                // red outline
  DrawRectangleGradientH(20, 20, barWidth, 30, GREEN, RED); // moving total bar

  int scaledSpeed = TOTAL_SPEED / 4; // becuase max is 400.0f (TODO: FIX)
  if (DEBUG) {
    Vector2 mousePos = GetMousePosition();
    DrawText(TextFormat("TOTAL_SPEED: %d", scaledSpeed), 20, 60, 20, RED);
    DrawText(TextFormat("Mouse Pos: x=%.2f, y=%.2f", mousePos.x, mousePos.y), 650, 10, 20, BLACK);
  }
  if (!DEBUG) {
    DrawText(TextFormat("TIMER: %d", TIMER), 20, 60, 20, RED);
  }

  if (warning) {
    DrawText("TOO FAST!", MeasureText("TOTAL_SPEED: 000", 20) + 30, 60, 20, RED);
  }
}

void drawButton(char *msg, Rectangle area)
{
  if (CheckCollisionPointRec(GetMousePosition(), area)) {
    DrawRectangleRec(area, (Color){225, 186, 47, 255});
  } else {
    DrawRectangleRec(area, BLACK);
  }
  float buttonTextLen = (float)MeasureText(msg, 50);
  DrawText(msg, area.x + (area.width / 2 - buttonTextLen / 2), area.y + (area.height / 2 - 25),
           50, WHITE);
}

void drawCenterText(char *msg, Color colour, int fsize, Vector2 pos)
{
  float msgLen = (float)MeasureText(msg, fsize);
  DrawText(msg, (pos.x - msgLen / 2), pos.y, fsize, colour);
}

bool isButtonPressed(Rectangle button)
{
    if (IsMouseButtonPressed(0) &&
        CheckCollisionPointRec(GetMousePosition(), button)) {
            return true;
    }
    return false;
}

void unloadTextures(UserInterface *ui, Target *jar, Bear *paw)
{
  // clean up resources
  UnloadTexture(ui->background);
  UnloadTexture(ui->failScreen);
  UnloadTexture(ui->splashScreen);
  UnloadTexture(ui->title);
  UnloadTexture(ui->winScreen);
  UnloadTexture(jar->tex);
  UnloadTexture(paw->tex);
  UnloadTexture(paw->nose);

  for (int i = 0; i < ui->wakeStatesLen; i++)
    UnloadTexture(ui->wakeStates[i]);
}

// reset fade-in, can be called any time
void StartFadeIn(FadeEffect *fade)
{
  fade->alpha = 1.0f;
  fade->active = true;
}

// call every frame
void UpdateFadeIn(FadeEffect *fade, float fadeSpeed)
{
  if (fade->active) {
    fade->alpha -= fadeSpeed;
    if (fade->alpha <= 0.0f) {
      fade->alpha = 0.0f;
      fade->alpha = false; // stop sending once complete
    }
  }
}

void drawTutorial()
{
  // create variables for the box
  Vector2 boxSize = {610.0f, 150.0f};
  Vector2 boxPos = {WIDTH/2 - boxSize.x/2, 200};
  Rectangle tutorialBox = {boxPos.x, boxPos.y, boxSize.x, boxSize.y};

  // create tutorial text
  char tut[1024];
  strcpy(tut, "* Control the Bear paw using the mouse\n");
  strcat(tut, "* Take as many snacks as you can before time runs out\n");
  strcat(tut, "* Don't wake up walt.");

  // draw tutorial box elements
  DrawRectangleRec(tutorialBox, GRAY);
  DrawText("How to Play", boxPos.x + 15, boxPos.y + 15, 30, BLACK);
  DrawText(tut, boxPos.x + 15, boxPos.y + 50, 20, BLACK);
}
