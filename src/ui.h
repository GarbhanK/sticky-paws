#include <raylib.h>

typedef struct UserInterface {
    Rectangle infoBox;
    int barWidth;
    int barMax;
    Rectangle startButton;
    Texture2D splashScreen;
    Texture2D failScreen;
    Texture2D wakeStates[4];
} UserInterface;

void greet();
void drawUI(UserInterface *ui, bool warning, int barWidth);
