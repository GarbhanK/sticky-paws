#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>
#include "ui.h"

extern int TOTAL_SPEED, TIMER;
extern float WIDTH, HEIGHT;
extern bool DEBUG;

void greet()
{
    printf("Hello from graphics!\n");
}

void drawUI(UserInterface *ui, bool warning, int barWidth)
{
    // draw speed indicator in top left
    DrawRectangleRec(ui->infoBox, WHITE);       // background box
    DrawRectangleLinesEx(ui->infoBox, 5, RED);  // red outline
    DrawRectangleGradientH(20, 20, barWidth, 30, GREEN, RED); // moving total bar

    if ( warning )
    {
        DrawText("TOO FAST!", 20, 60, 20, RED);
    }

    if (DEBUG) { DrawText(TextFormat("TOTAL_SPEED: %d", TOTAL_SPEED), 20, 60, 20, RED); }
    if (!DEBUG) { DrawText(TextFormat("TIMER: %d", TIMER), 20, 60, 20, RED); }

    // Draw old man in the corner
    if (TOTAL_SPEED >= 0) { DrawTexture(ui->wakeStates[0], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 30) { DrawTexture(ui->wakeStates[1], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 50) { DrawTexture(ui->wakeStates[2], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 80) { DrawTexture(ui->wakeStates[3], 0, HEIGHT-250, WHITE); }
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

// void drawBear(Bear *b)
// {
//     // draw bear paw
//     DrawTexture(b->tex, b->pos.x, b->pos.y, WHITE);

//     float noseThreshold = HEIGHT - b->nose.height;
//     Vector2 nosePos = { (WIDTH/2)-150, b->pos.y + HEIGHT*0.50 };
//     // TODO: maybe use below nose movement logic instead?
//     // Vector2 nosePos = { (Paw.pos.x-150), Paw.pos.y + HEIGHT*0.50 };

//     // limit nose position past the bottom of the texture
//     if ( nosePos.y <= noseThreshold ) {
//         nosePos.y = noseThreshold;
//     };
//     DrawTextureV(b->nose, nosePos, WHITE);
// }
