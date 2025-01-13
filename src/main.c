#include <math.h>
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

typedef struct Bear {
    Rectangle hitbox;
    Texture2D tex;
    Vector2 pos;
} Bear;

typedef struct Honey {
    Texture2D tex;
    Vector2 pos;
    Vector2 hitbox;
    float radius;
    bool stuck;
    int value;
} Honey;

typedef struct Obstacle {
    Rectangle rect;
    bool stuck;
    int value;
} Obstacle;

bool DEBUG = true;
int SCORE = 0;
float TOTAL_SPEED = 0.0f;
float TOTAL_SPEED_MAX = 100.0f;
float DECAY = 20.0f;
double TIME_INTERVAL = 0.5;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

// declare functions
void handleStickyJar(Bear *paw, Honey *jar, Vector2 *dt);
void handleStickyObstacle(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt);

void handlePawPushing(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt);
void handleObjectPushing(Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt);


int main()
{
	InitWindow(WIDTH, HEIGHT, "Sticky Paws");
    Texture2D picnicBlanket = LoadTexture("assets/picnic_blanket_grass.png");

    double currentTime = GetTime();
    double lastTime;
    float sensitivity = 1.5f;
    float mouseSpeed;
    float absMouseDelta;
    Rectangle infoBox = { 0, 0, 400, 100};

    // additional obsacles, e.g other picnic items
    Obstacle obstacles[] = {
        { { 30, HEIGHT/2, 150, 150 }, false, 10 },
        { { 500, HEIGHT/3, 70, 200 }, false, 10 },
        { { 300, 250, 30, 50 }, false, 10 },
        { { 800, 500, 80, 110 }, false, 10 },
    };

    int obstaclesLen = sizeof(obstacles)/sizeof(obstacles[0]);

    Bear Paw = {
        .tex = LoadTexture("assets/sticky_paw.png"),
    };

    Honey Jar = {
        .tex = LoadTexture("assets/honey.png"),
        .pos = { WIDTH/2 + 50, 100 },
        .radius = 50.0f,
        .stuck = false,
        .value = 50,
    };

    // reset mouse so bear paw insn't in top right
    SetMousePosition(HEIGHT-50, WIDTH/2);

	while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_TAB))
        {
            if (DEBUG) { DEBUG = false; } else { DEBUG = true; }
        }

        // mouse position diff used to stuck object movement
        Vector2 mouseDelta = GetMouseDelta();

        // debug mouse delta
        if (DEBUG && (mouseDelta.x != 0 && mouseDelta.y != 0) )
        {
            printf("mouse dx, xy: %0.2f, %0.2f \n", mouseDelta.x, mouseDelta.y);
            printf("mouse total: %0.2f \n", fabs(mouseDelta.x + mouseDelta.y));
            printf("TOTAL_SPEED: %0.2f \n", TOTAL_SPEED);
        }

        // update paw movement
        Paw.pos.x = GetMouseX() - Paw.tex.width/2;
        Paw.pos.y = GetMouseY();
        Paw.hitbox = (Rectangle){ Paw.pos.x, Paw.pos.y, Paw.tex.width, Paw.tex.height };

        // update Jar
        Jar.hitbox = (Vector2){ Jar.pos.x - Jar.radius, Jar.pos.y - Jar.radius };

        // handle sticky logic
        handleStickyJar(&Paw, &Jar, &mouseDelta);
        handleStickyObstacle(&Paw, obstacles, obstaclesLen, &mouseDelta);

        // handle pushing logic
        // handlePawPushing(&Paw, obstacles, obstaclesLen, &mouseDelta);
        handleObjectPushing(obstacles, obstaclesLen, &Jar, &mouseDelta);

        // speed/scoring
        if (mouseDelta.x != 0 && mouseDelta.y != 0)
        {
            absMouseDelta = fabs(mouseDelta.x) + fabs(mouseDelta.y);
            mouseSpeed = absMouseDelta;
            TOTAL_SPEED = TOTAL_SPEED + mouseSpeed;
        }

        // decrease speed total each frame
        double currentTime = GetTime();
        if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL) )
        {
            lastTime = currentTime;
            TOTAL_SPEED = fabs(TOTAL_SPEED - DECAY);
        }

        // limit the total speed
        if (TOTAL_SPEED > TOTAL_SPEED_MAX)
        {
            TOTAL_SPEED = TOTAL_SPEED_MAX;
        }

		BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawTexture(picnicBlanket, 0, 0, WHITE);

            // draw obstacles
            for (int i=0; i <= obstaclesLen; i++)
            {
                Obstacle obs = obstacles[i];
                DrawRectangleRec(obs.rect, BLACK);
            }

            // draw honey jar texture
            DrawTextureV(Jar.tex, Jar.hitbox, WHITE);

            // draw bear Paw texture
            DrawTexture(Paw.tex, Paw.pos.x, Paw.pos.y, WHITE);

            // draw speed indicator in top left
            DrawRectangleRec(infoBox, WHITE);       // background box
            DrawRectangleLinesEx(infoBox, 5, RED);  // red outline

            // draw speed bar
            // bar height is total % of the bar max
            int barMax = infoBox.width - (5 + 25);
            int barWidth = (int)(TOTAL_SPEED);

            // put max limit on the width
            if (barWidth > barMax) { barWidth = barMax; }

            // the moving total bar
            // DrawRectangle(20, 20, barWidth, 30, RED);

            if ( mouseSpeed >= sensitivity )
            {
                // DrawText("TOO FAST!", WIDTH/2, HEIGHT/2, 70, RED);
            }

            if (DEBUG)
                for (int i=0; i < 5; i++) {
                    DrawRectangleLinesEx(20*i, 20, 20, 20, BLACK);
                    DrawRectangle(20*i, 20, 20, 20, RED);
                }
                DrawText(TextFormat("TOTAL_SPEED: %0.2f", TOTAL_SPEED), 20, 60, 20, RED);

		EndDrawing();
	}

    // clean up resources
    UnloadTexture(picnicBlanket);
    UnloadTexture(Jar.tex);
    UnloadTexture(Paw.tex);
	CloseWindow();
	return 0;
}

void handleStickyJar(Bear *paw, Honey *jar, Vector2 *dt)
{
        // Jar sticky logic
        if (!jar->stuck)
        {
            if ( CheckCollisionCircleRec(jar->pos, jar->radius, paw->hitbox) )
            {
                jar->stuck = true;
                SCORE += jar->value;
                printf("SCORE: %d\n", SCORE);
            }
        } else {
            // update Jar pos by adding mouse delta
            jar->pos.x = jar->pos.x + dt->x;
            jar->pos.y = jar->pos.y + dt->y;
        }
}

void handleStickyObstacle(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt)
{
    for (int i=0; i <= arrLen; i++)
    {
        Obstacle *subject = &obs[i];

        // obstacle sticky logic
        if (!subject->stuck)
        {
            if ( CheckCollisionRecs(subject->rect, paw->hitbox) )
            {
                subject->stuck = true;
                SCORE += subject->value;
                printf("SCORE: %d\n", SCORE);
            }
        } else {
            // update Jar pos by adding mouse delta
            subject->rect.x = subject->rect.x + dt->x;
            subject->rect.y = subject->rect.y + dt->y;
        }
    }
}


void handleObjectPushing(Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt)
{
    for (int i=0; i <= arrLen; i++)
    {
        Obstacle *actor = &obs[i];

        // object on object pushing logic
        for (int j=0; j <= arrLen; j++)
        {
            // if ( (i == j) || (actor->stuck) ) {
            if ( i == j ) {
                // skip if same object or object already stuck to paw
                continue;
            }

            Obstacle *subject = &obs[j];

            if ( CheckCollisionRecs(actor->rect, subject->rect) )
            {
                subject->rect.x = subject->rect.x + dt->x;
                subject->rect.y = subject->rect.y + dt->y;
            }
        }

        // object on honey jar logic
        if ( CheckCollisionCircleRec(jar->pos, jar->radius, actor->rect) )
        {
            jar->pos.x = jar->pos.x + dt->x;
            jar->pos.y = jar->pos.y + dt->y;
        }
    }
}

void handlePawPushing(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt)
{
    // paw on object pushing logic
    for (int i=0; i <= arrLen; i++)
    {
        Obstacle *subject = &obs[i];

        if ( CheckCollisionRecs(paw->hitbox, subject->rect) )
        {
            subject->rect.x = subject->rect.x + dt->x;
            subject->rect.y = subject->rect.y + dt->y;
        }
    }
}

