#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include <raylib.h>
#include <raymath.h>

#include "ui.h"
#include "bear.h"

/*
TODOs:
    - [x] majorly clean up and finalise the SPEED logic, works now but old code left over that needs cleaning
    - [x] Vectorise the dy/dx mouse movement speed, I think moving diagonals is way faster than across
    - [x] add logic for the old man, different stages of awake
    - [x] add art for old man, maybe have him framed in a cool way? Old school TV with the antennas? courage the cowardly dog
    - [x] proper debug toggling logic
    - [x] figure out what i'm going to do with the bar in the top right
    - [x] some kind of narrative framing around the bear seeing the guy? Periscope?
    - [x] add title screen with real image of a bear (make sure start button places paw at the bottom)
    - [x] Ensure sticky members are reset to false when restarted after fail
    - [x] Stuck obj acting on unstuck obj, stuck object acting like it's being moved too. need to investigate/fix
    - [x] Countdown timer until loss state
    - [x] Add bear nose and associated vars to Bear struct
    - [ ] get all of the text in the right place
    - [ ] possibly replace jar of honey with a salmon
    - [ ] add win animations of bear pics moving across screen overlapped
    - [ ] need lots and lots of sfx for final polish
        - sniffing nose
        - snoring/sleeping
        - obj movement
        - sticking noise
        - UI noises
*/

typedef struct Honey {
    Texture2D tex;
    Vector2 pos;
    Rectangle hitbox;
    bool stuck;
    int value;
} Honey;

typedef struct Obstacle{
    Rectangle rect;
    bool stuck;
    int value;
    Texture2D tex;
} Obstacle;

typedef struct {
    Obstacle* items;    // the Obstacles
    Rectangle* init;    // array of rect positions to restart/init the game
    int len;    // current no. items
    int cap;    // total arr capacity
} ObstacleArray;

typedef struct {
    Sound growl1;
    Sound growl2;
    Sound stick;
    Sound drag;
} SoundBank;

enum GAMESTATE {
    START,
    PLAY,
    FAIL,
    WIN
} GAMESTATE;

bool DEBUG = true;
int SCORE = 0;
int TIMER = 15;
int TOTAL_SPEED = 0;
float TOTAL_SPEED_MAX = 400.0f;
float DECAY = 15.0f;
float SENSITIVITY = 3.0f;
double TIME_INTERVAL = 0.1f;

const float WIDTH = 1024.0f;
const float HEIGHT = 768.0f;

// declare functions
void handleStickyJar(Bear *paw, Honey *jar, SoundBank *sb);
void handleStickyObstacle(Bear *paw, Obstacle obs[], int arrLen, SoundBank *sb);

void handlePawPushing(Bear *b, Obstacle obs[], int arrLen, Vector2 *dt);
void handleObjectPushing(Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt);

void resetObjects(Honey *jar, Obstacle obs[], int arrLen);
void handleSpeed();
void playBearSound(SoundBank *sb);


Rectangle obstacleInit[] = {
    { 200, HEIGHT/2, 150, 150 },
    { 500, HEIGHT/3, 70, 200 },
    { 300, 250, 30, 50 },
    { 800, 500, 80, 110 }
};


int main()
{
	InitWindow(WIDTH, HEIGHT, "Sticky Paws");
	InitAudioDevice();

    double currentTime, lastTime, timerPrev;
    float speedDecrease;
    bool warning = false;

    SoundBank sounds = {
        .growl1 = LoadSound("assets/sfx/zapsplat_animals_bear_grunt_001_17143.mp3"),
        .growl2 = LoadSound("assets/sfx/zapsplat_animals_bear_grunt_002_17144.mp3"),
    };

    GAMESTATE = START;

    // TODO: change for existing obstacles[] array
    // ObstacleArray Obs = {
    //     .init = obstacleInit,
    //     .items = obstacles,
    //     .len = sizeof(obstacles)/sizeof(obstacles[0]),
    // };

    // additional obsacles, e.g other picnic items
    Obstacle obstacles[] = {
        { obstacleInit[0], false, 10 },
        { obstacleInit[1], false, 10 },
        { obstacleInit[2], false, 10 },
        { obstacleInit[3], false, 10 },
    };
    int obstaclesLen = sizeof(obstacles)/sizeof(obstacles[0]);

    Bear Paw = {
        .tex = LoadTexture("assets/sticky_paw.png"),
        .nose = LoadTexture("assets/bear_nose.png")
    };

    Honey Jar = {
        .tex = LoadTexture("assets/honey.png"),
        .pos = { WIDTH/2 + 50, 100 },
        .stuck = false,
        .value = 50,
    };

    UserInterface GameUI = {
        .infoBox = { 0, 0, 400, 100 },
        .barMax = (GameUI.infoBox.width - 40),
        .startButton = { WIDTH/2 - 200, HEIGHT-120, 400, 100 },
        .background = LoadTexture("assets/picnic_blanket_grass.png"),
        .splashScreen = LoadTexture("assets/bear_splash.jpg"),
        .failScreen = LoadTexture("assets/bear_jail.png"),
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

        if (IsKeyPressed(KEY_TAB)) {
            (DEBUG) ? (DEBUG = false) : (DEBUG = true);
        }
        if (IsKeyPressed(KEY_R)) {
            GAMESTATE = PLAY;
        }

        // start screen
        if (GAMESTATE == START) {
            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, obstacles, obstaclesLen);
                GAMESTATE = PLAY;
            }
        }

        if (GAMESTATE == PLAY) {
            currentTime = GetTime();

            // decrease timer every second (1.0 = 1 sec)
            if ( (currentTime - timerPrev) >= 1.0 ) {
                timerPrev = currentTime;
                TIMER = TIMER - 1;
            }

            if (TIMER == 0) { GAMESTATE = FAIL; }

            // update paw movement
            Paw.pos.x = GetMouseX() - (float)Paw.tex.width/2;
            Paw.pos.y = GetMouseY();
            Paw.hitbox = (Rectangle){ Paw.pos.x, Paw.pos.y, Paw.tex.width, Paw.tex.height };

            // update Jar hitbox
            Jar.hitbox = (Rectangle){
                Jar.pos.x + 10,
                Jar.pos.y + 15,
                Jar.tex.width - 20,
                Jar.tex.height - 25
            };

            // handle sticky logic
            handleStickyJar(&Paw, &Jar, &sounds);
            handleStickyObstacle(&Paw, obstacles, obstaclesLen, &sounds);

            // handle pushing logic
            // handlePawPushing(&Paw, obstacles, obstaclesLen, &mouseDelta);
            handleObjectPushing(obstacles, obstaclesLen, &Jar, &mouseDelta);

            handleSpeed();

            // decrease speed total each frame
            if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL) )
            {
                lastTime = currentTime;
                speedDecrease = fabs(TOTAL_SPEED - DECAY);
                TOTAL_SPEED = (int)speedDecrease;
            }

            // win game logic
            if (Jar.pos.y >= HEIGHT) { GAMESTATE = WIN; }
        }

        if (GAMESTATE == FAIL)
        {
            if (IsKeyPressed(KEY_ENTER)) {
                GAMESTATE = START;
            }

            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, obstacles, obstaclesLen);
                GAMESTATE = PLAY;
            }
        }

        if (GAMESTATE == WIN) {
            // printf("You Win!");
            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, obstacles, obstaclesLen);
                GAMESTATE = PLAY;
            }
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (GAMESTATE == START) {
                DrawTexture(GameUI.splashScreen, 0, 0, WHITE);
                drawButton("PLAY", GameUI.startButton);
            }

            if (GAMESTATE == PLAY) {
                DrawTexture(GameUI.background, 0, 0, WHITE);    // draw background image

                for (int i=0; i <= obstaclesLen; i++)
                {
                    Obstacle obs = obstacles[i];
                    DrawRectangleRec(obs.rect, BLACK);      // draw obstacles
                }

                DrawTexture(Jar.tex, Jar.pos.x, Jar.pos.y, WHITE);   // draw honey Jar
                    DrawRectangleRec(Jar.hitbox, GREEN);               // DEBUG HONEY HITBOX

                drawUI(&GameUI, warning, GameUI.barWidth);  // draw UI
                drawBear(&Paw);
            }

            if (GAMESTATE == FAIL) {
                DrawTexture(GameUI.failScreen, 0, 0, WHITE);
                DrawText("FAIL", WIDTH/2, HEIGHT/2, 200, RED);
                drawButton("RESTART", GameUI.startButton);
            }

            if (GAMESTATE == WIN) {
                // picture of a happy bear, sympathy for the devil w/ restart button
                DrawText("WIN", WIDTH/2, HEIGHT/2, 100, RED);
                drawButton("RESTART", GameUI.startButton);
            }

		EndDrawing();
	}

    // clean up resources
    UnloadTexture(GameUI.background);
    UnloadTexture(GameUI.failScreen);
    UnloadTexture(GameUI.splashScreen);
    UnloadTexture(Jar.tex);
    UnloadTexture(Paw.tex);
    UnloadTexture(Paw.nose);

    for (int i=0; i <= sizeof(GameUI.wakeStates); i++) {
        printf("unloading: %d\n", GameUI.wakeStates[i].id);
        UnloadTexture(GameUI.wakeStates[i]);
    }

	CloseWindow();
	return 0;
}

void handleStickyJar(Bear *paw, Honey *jar, SoundBank *sb)
{
    Vector2 dt = GetMouseDelta();
    if (!jar->stuck)
    {
        if ( CheckCollisionRecs(jar->hitbox, paw->hitbox) )
        {
            playBearSound(sb);
            jar->stuck = true;
            SCORE += jar->value;
            printf("SCORE: %d\n", SCORE);
        }
    } else {
        // update Jar pos by adding mouse delta
        jar->pos.x = jar->pos.x + dt.x;
        jar->pos.y = jar->pos.y + dt.y;
    }
}

void handleStickyObstacle(Bear *paw, Obstacle obs[], int arrLen, SoundBank *sb)
{
    Vector2 dt = GetMouseDelta();
    for (int i=0; i <= arrLen; i++)
    {
        Obstacle *subject = &obs[i];

        // obstacle sticky logic
        if (!subject->stuck)
        {
            if ( CheckCollisionRecs(subject->rect, paw->hitbox) )
            {
                playBearSound(sb);
                subject->stuck = true;
                SCORE += subject->value;
                printf("SCORE: %d\n", SCORE);
            }
        } else {
            // update Jar pos by adding mouse delta
            subject->rect.x = subject->rect.x + dt.x;
            subject->rect.y = subject->rect.y + dt.y;
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
                if (actor->stuck) {
                    subject->rect.x = subject->rect.x + dt->x;
                    subject->rect.y = subject->rect.y + dt->y;
                }
            }
        }

        // object on honey jar logic
        if ( CheckCollisionRecs(jar->hitbox, actor->rect) )
        {
            if (!jar->stuck) {
                jar->pos.x = jar->pos.x + dt->x;
                jar->pos.y = jar->pos.y + dt->y;
            } else {
                actor->rect.x = actor->rect.x + dt->x;
                actor->rect.y = actor->rect.y + dt->y;
            }
        }
    }
}

void handlePawPushing(Bear *b, Obstacle obs[], int arrLen, Vector2 *dt)
{
    for (int i=0; i <= arrLen; i++)
    {
        Obstacle *subject = &obs[i];

        if ( CheckCollisionRecs(b->hitbox, subject->rect) )
        {
            subject->rect.x = subject->rect.x + dt->x;
            subject->rect.y = subject->rect.y + dt->y;
        }
    }
}

void resetObjects(Honey *jar, Obstacle obs[], int arrLen)
{
    // reset scores
    TOTAL_SPEED = 0;
    SCORE = 0;
    TIMER = 30;

    // reset honey jar
    jar->stuck = false;
    jar->pos = (Vector2){ WIDTH/2 + 50, 100 };
    // jar->hitbox = jar->pos;
    jar->hitbox = (Rectangle){ jar->pos.x, jar->pos.y, jar->hitbox.width, jar->hitbox.height };

    // loop through obstacles and set to original x/y
    for (int i = 0; i <= arrLen; i++) {
        Obstacle *o = &obs[i];
        o->stuck = false;
        o->rect = obstacleInit[i];
    }
}

void handleSpeed()
{
    Vector2 dt = GetMouseDelta();
    float absMouseDelta, mouseSpeed;

    // speed increase
    if (dt.x != 0 && dt.y != 0) {
        absMouseDelta = fabs(dt.x) + fabs(dt.y);
        mouseSpeed = absMouseDelta;
        TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed * SENSITIVITY);
    } else if (dt.x == 0 && dt.y != 0) {
        absMouseDelta = fabs(dt.y);
        mouseSpeed = absMouseDelta;
        // TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed*2;
        TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed*2 * SENSITIVITY);
    } else if (dt.x != 0 && dt.y == 0) {
        absMouseDelta = fabs(dt.x);
        mouseSpeed = absMouseDelta;
        // TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed*2;
        TOTAL_SPEED = TOTAL_SPEED + ((int)mouseSpeed*2 * SENSITIVITY);
    }

    // limit the total speed
    if (TOTAL_SPEED > TOTAL_SPEED_MAX) {
        TOTAL_SPEED = TOTAL_SPEED_MAX;
        if (!DEBUG) // TEMP: removes fail state for testing
            GAMESTATE = FAIL;
    }

    // get rid of that issue where score flashes back and forth at idle
    if (TOTAL_SPEED <= 3) {
        TOTAL_SPEED = 0;
    }
}

void playBearSound(SoundBank *sb) {
    if (GetRandomValue(1, 2) == 1) {
        PlaySound(sb->growl1);
    } else {
        PlaySound(sb->growl2);
    }
}
