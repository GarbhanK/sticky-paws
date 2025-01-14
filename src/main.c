#include <math.h>
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

/*
TODOs:
    - majorly clean up and finalise the SPEED logic, works now but old code left over that needs cleaning
    - Vectorise the dy/dx mouse movement speed, I think moving diagonals is way faster than across
    - add logic for the old man, different stages of awake
    - add art for old man, maybe have him framed in a cool way? Old school TV with the antennas? courage the cowardly dog
    - proper debug toggling logic
    - get all of the text in the right place
    - figure out what i'm going to do with the bar in the top right
    - some kind of narrative framing around the bear seeing the guy? Periscope?
    - add title screen with real image of a bear (make sure start button places paw at the bottom)
*/

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

typedef struct UserInterface {
    Rectangle infoBox;
    int barWidth;
    int barMax;
    Rectangle startButton;
    Texture2D splashScreen;
    Texture2D wakeStates[4];
} UserInterface;

enum GAMESTATE {
    START,
    PLAY,
    FAIL,
    WIN
} GAMESTATE;

bool DEBUG = false;
int SCORE = 0;
int TOTAL_SPEED = 0;
float TOTAL_SPEED_MAX = 100.0f;
float DECAY = 5.0f;
double TIME_INTERVAL = 0.1f;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

// declare functions
void handleStickyJar(Bear *paw, Honey *jar, Vector2 *dt);
void handleStickyObstacle(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt);

void handlePawPushing(Bear *paw, Obstacle obs[], int arrLen, Vector2 *dt);
void handleObjectPushing(Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt);

void DrawUI(UserInterface *ui, bool warning, int barWidth);


int main()
{
	InitWindow(WIDTH, HEIGHT, "Sticky Paws");
    Texture2D picnicBlanket = LoadTexture("assets/picnic_blanket_grass.png");
    Texture2D bearNose = LoadTexture("assets/bear_nose.png");

    double currentTime, lastTime;
    float mouseSpeed, absMouseDelta, speedDecrease;
    int sensitivity = 50;
    bool warning = false;

    GAMESTATE = START;

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

    UserInterface GameUI = {
        .infoBox = { 0, 0, 400, 100 },
        .barMax = GameUI.infoBox.width - (40),
        .startButton = { WIDTH/2 - 200, HEIGHT-120, 400, 100 },
        .splashScreen = LoadTexture("assets/bear_splash.jpg"),
        .wakeStates = {
            LoadTexture("assets/tv_asleep.png"),
            LoadTexture("assets/tv_1.png"),
            LoadTexture("assets/tv_2.png"),
            LoadTexture("assets/tv_3.png"),
        }
    };

    // reset mouse so bear paw isn't in top right
    SetMousePosition(HEIGHT-50, WIDTH/2);

	while (!WindowShouldClose()) {
        // mouse position diff used to stuck object movement
        Vector2 mouseDelta = GetMouseDelta();

        if (IsKeyPressed(KEY_TAB))
        {
            (DEBUG) ? (DEBUG = false) : (DEBUG = true);
        }

        // debug printing
        if (DEBUG && (mouseDelta.x != 0 && mouseDelta.y != 0) )
        {
            printf("mouse dx, xy: %0.2f, %0.2f \n", mouseDelta.x, mouseDelta.y);
            printf("mouse total: %0.2f \n", fabs(mouseDelta.x + mouseDelta.y));
            printf("TOTAL_SPEED: %d \n", TOTAL_SPEED);
        }

        // start screen
        if (GAMESTATE == START) {
            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                printf("Game started!\n");
                GAMESTATE = PLAY;
            }
        }

        if (GAMESTATE == PLAY) {

            // update paw movement
            Paw.pos.x = GetMouseX() - (float)Paw.tex.width/2;
            Paw.pos.y = GetMouseY();
            Paw.hitbox = (Rectangle){ Paw.pos.x, Paw.pos.y, Paw.tex.width, Paw.tex.height };

            // update Jar hitbox
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
                TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed;
            }

            // decrease speed total each frame
            currentTime = GetTime();
            if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL) )
            {
                lastTime = currentTime;
                speedDecrease = fabs(TOTAL_SPEED - DECAY);
                TOTAL_SPEED = (int)speedDecrease;
            }

            // limit the total speed
            if (TOTAL_SPEED > TOTAL_SPEED_MAX)
            {
                TOTAL_SPEED = TOTAL_SPEED_MAX;
                if (!DEBUG) // TEMP: removes fail state for testing
                    GAMESTATE = FAIL;
            }

            // speed bar update logic
            GameUI.barWidth = TOTAL_SPEED * 4; // *4 because TOTAL_SPEED is out of 100, bar width is 400. TODO: make work with x=n

            // put max limit on the width
            if (GameUI.barWidth > GameUI.barMax)
            {
                GameUI.barWidth = GameUI.barMax;
            }

            // set flag for warning message
            ( TOTAL_SPEED >= sensitivity ) ? (warning = true) : (warning = false);
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (GAMESTATE == START) {
                DrawTexture(GameUI.splashScreen, 0, 0, WHITE);

                if (CheckCollisionPointRec(GetMousePosition(), GameUI.startButton)) {
                    DrawRectangleRec(GameUI.startButton, GRAY);
                } else {
                    DrawRectangleRec(GameUI.startButton, BLACK);
                }
                float startButtonTextLen = (float)MeasureText("PLAY", 50);
                DrawText("PLAY",
                    GameUI.startButton.x + (GameUI.startButton.width/2 - startButtonTextLen/2),
                    GameUI.startButton.y + (GameUI.startButton.height/2 - 25),
                    50, WHITE);
            }

            if (GAMESTATE == PLAY) {
                DrawTexture(picnicBlanket, 0, 0, WHITE);            // draw background image
                // draw obstacles
                for (int i=0; i <= obstaclesLen; i++)
                {
                    Obstacle obs = obstacles[i];
                    DrawRectangleRec(obs.rect, BLACK);              // draw obstacles
                }

                DrawTextureV(Jar.tex, Jar.hitbox, WHITE);           // draw honey Jar
                DrawTexture(Paw.tex, Paw.pos.x, Paw.pos.y, WHITE);  // draw bear Paw
                DrawUI(&GameUI, warning, GameUI.barWidth);          // draw UI

                // TODO: bear nose from top-down when paw is past 75% of Y value, follow the mouse movement
                Vector2 nosePos = { (WIDTH/2)-150, Paw.pos.y + HEIGHT*0.75 };
                if (nosePos.y >= HEIGHT) { nosePos.y = HEIGHT; };
                printf("HEIGHT: %0.2f\n", HEIGHT);
                printf("nosePos.y: %0.2f\n", nosePos.y);
                printf("nosePos.x: %0.2f\n", nosePos.x);
                DrawTextureV(bearNose, nosePos, WHITE);
            }

            if (GAMESTATE == FAIL) {
                DrawTexture(GameUI.splashScreen, 0, 0, WHITE);
                DrawText("FAIL", WIDTH/2, HEIGHT/2, 200, RED);
                // paw movement paused, Loud sound, old man angry
                // picture of bear being arrested (fade in)
                // restart button
                if (IsKeyPressed(KEY_ENTER)) {
                    GAMESTATE = START;
                }
             }
            if (GAMESTATE == WIN) {
                // picture of a happy bear, sympathy for the devil
                // restart button
            }

		EndDrawing();
	}

    // clean up resources
    UnloadTexture(picnicBlanket);
    UnloadTexture(Jar.tex);
    UnloadTexture(Paw.tex);

    for (int i=0; i <= 3; i++) {
        printf("unloading: %d\n", GameUI.wakeStates[i].id);
        UnloadTexture(GameUI.wakeStates[i]);
    }

	CloseWindow();
	return 0;
}

void DrawUI(UserInterface *ui, bool warning, int barWidth)
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

    // Draw old man in the corner
    if (TOTAL_SPEED >= 0) { DrawTexture(ui->wakeStates[0], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 30) { DrawTexture(ui->wakeStates[1], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 50) { DrawTexture(ui->wakeStates[2], 0, HEIGHT-250, WHITE); }
    if (TOTAL_SPEED >= 80) { DrawTexture(ui->wakeStates[3], 0, HEIGHT-250, WHITE); }
}

void handleStickyJar(Bear *paw, Honey *jar, Vector2 *dt)
{
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
            if ( i == j ) { continue; } // skip if same object or object already stuck to paw

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
