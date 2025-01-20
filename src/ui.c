#include <stdbool.h>
#include <raylib.h>
#include "ui.h"

extern int TOTAL_SPEED, TIMER;
extern float WIDTH, HEIGHT, TOTAL_SPEED_MAX;
extern bool DEBUG;


void drawUI(UserInterface *ui, bool warning, int barWidth)
{
    // speed bar update logic
    ui->barWidth = TOTAL_SPEED;

    // put max limit on the width
    if (ui->barWidth > ui->barMax) { ui->barWidth = ui->barMax; }

    // set flag for warning message
    ( TOTAL_SPEED >= ui->barMax/2 ) ? (warning = true) : (warning = false);

    // draw speed indicator in top left
    DrawRectangleRec(ui->infoBox, WHITE);       // background box
    DrawRectangleLinesEx(ui->infoBox, 5, RED);  // red outline
    DrawRectangleGradientH(20, 20, barWidth, 30, GREEN, RED); // moving total bar

    int scaledSpeed = TOTAL_SPEED/4;    // becase max is 400.0f
    if (DEBUG) { DrawText(TextFormat("TOTAL_SPEED: %d", scaledSpeed), 20, 60, 20, RED); }
    if (!DEBUG) { DrawText(TextFormat("TIMER: %d", TIMER), 20, 60, 20, RED); }

    if ( warning )
    {
        DrawText("TOO FAST!",
            MeasureText("TOTAL_SPEED: 000", 20)+30, 60, 20, RED);
    }

    // Draw old man in the corner
    if (TOTAL_SPEED >= 0) { DrawTexture(ui->wakeStates[0], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX*0.3) { DrawTexture(ui->wakeStates[1], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX*0.5) { DrawTexture(ui->wakeStates[2], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= TOTAL_SPEED_MAX*0.8) { DrawTexture(ui->wakeStates[3], 0, HEIGHT-250, WHITE); }
}

void drawButton(char *msg, Rectangle area)
{
    if (CheckCollisionPointRec(GetMousePosition(), area)) {
        DrawRectangleRec(area, GRAY);
    } else {
        DrawRectangleRec(area, BLACK);
    }
    float startButtonTextLen = (float)MeasureText(msg, 50);
    DrawText(msg,
        area.x + (area.width/2 - startButtonTextLen/2),
        area.y + (area.height/2 - 25),
        50, WHITE);
}
