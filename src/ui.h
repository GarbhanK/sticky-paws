/**
 * @file ui.h
 * @brief User interface rendering and interaction for Sticky Paws
 *
 * This file handles all UI elements including buttons, text,
 * fade effects, and screen overlays.
 */
#ifndef UI_H
#define UI_H

#include <raylib.h>
#include <stddef.h>

#include "game.h"

/**
 * @brief Fade effect for screen transitions
 */
typedef struct {
  float alpha; // Alpha transparency value (0.0-1.0)
  bool active; // Whether fade is currently active
} FadeEffect;

/**
 * @brief Main user interface structure containing all UI elements
 */
typedef struct UserInterface {
  FadeEffect fade;
  Rectangle infoBox;
  int barWidth;
  int barMax;
  Rectangle startButton;
  Rectangle resetButton;
  Rectangle tutorialButton;
  Texture2D background;
  Texture2D splashScreen;
  Texture2D failScreen;
  Texture2D title;
  Texture2D winScreen;
  Texture2D wakeStates[4];
  size_t wakeStatesLen;
} UserInterface;

/**
 * @brief Draw the game UI overlay (timer, speed bar, old man)
 * @param ctx Game context
 * @param ui User interface structure
 * @param warning Whether to show speed warning
 * @param barWidth Current width of speed bar
 */
void drawUI(GameContext *ctx, UserInterface *ui, bool warning, int barWidth);

/**
 * @brief Draw a button with text
 * @param msg Button text
 * @param area Button rectangle area
 */
void drawButton(const char *msg, Rectangle area);

/**
 * @brief Draw centered text on screen
 * @param msg Text to draw
 * @param colour Text color
 * @param fsize Font size
 * @param pos Center position
 */
void drawCenterText(const char *msg, Color colour, int fsize, Vector2 pos);

/**
 * @brief Check if a button was clicked
 * @param button Button rectangle area
 * @return true if button was pressed, false otherwise
 */
bool isButtonPressed(Rectangle button);

/**
 * @brief Start a fade-in effect
 * @param fade Fade effect to start
 */
void StartFadeIn(FadeEffect *fade);

/**
 * @brief Update fade-in effect each frame
 * @param fade Fade effect to update
 * @param fadeSpeed Speed of fade (alpha change per frame)
 */
void UpdateFadeIn(FadeEffect *fade, float fadeSpeed);

/**
 * @brief Draw the tutorial overlay
 */
void drawTutorial(void);

#endif // UI_H
