#include <stddef.h>
#include <raylib.h>

typedef struct UserInterface {
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

typedef struct {
  float alpha;
  bool active;
} FadeEffect;


void drawUI(UserInterface *ui, bool warning, int barWidth);
void drawButton(char *msg, Rectangle area);
void drawCenterText(char *msg, Color colour, int fsize, Vector2 pos);
bool isButtonPressed(Rectangle button);
void StartFadeIn(FadeEffect *fade);
void UpdateFadeIn(FadeEffect *fade, float fadeSpeed);
void drawTutorial();
