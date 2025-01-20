#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#include "ui.h"

/*
TODOs:
    - [x] majorly clean up and finalise the SPEED logic, works now but old code left over that needs cleaning
    - [x] Vectorise the dy/dx mouse movement speed, I think moving diagonals is way faster than across
    - [x] add logic for the old man, different stages of awake
    - [x] add art for old man, maybe have him framed in a cool way? Old school TV with the antennas? courage the cowardly dog
    - [x] proper debug toggling logic
    - [ ] get all of the text in the right place
    - [x] figure out what i'm going to do with the bar in the top right
    - [x] some kind of narrative framing around the bear seeing the guy? Periscope?
    - [x] add title screen with real image of a bear (make sure start button places paw at the bottom)
    - [x] Ensure sticky members are reset to false when restarted after fail
    - [ ] Stuck obj acting on unstuck obj, stuck object acting like it's being moved too. need to investigate/fix
    - [ ] possibly replace jar of honey with a salmon
    - [ ] add win animations of bear pics moving across screen overlapped
    - [ ] need lots and lots of sfx for final polish
        - sniffing nose
        - snoring/sleeping
        - obj movement
        - sticking noise
        - UI noises
    - [ ] Countdown timer until loss state
    - [ ] LERP smoothing
    - [x] Add bear nose and associated vars to Bear struct
*/

typedef struct Bear {
    Rectangle hitbox;
    Texture2D tex;
    Texture2D nose;
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

typedef struct {
    int actor;
    int subject;
} ObstaclePair;

typedef struct {
    ObstaclePair* items;
    int len;
    int cap;
} ObstaclePairArray;

enum GAMESTATE {
    START,
    PLAY,
    FAIL,
    WIN
} GAMESTATE;

bool DEBUG = false;
int OBS_ARR_SIZE = 32;
int SCORE = 0;
int TIMER = 30;
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
void handleObjectPushing(ObstaclePairArray *o, Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt);
bool checkSubjectActorPairs(ObstaclePairArray *o, int a, int s);
bool pairInArray(ObstaclePairArray *o, ObstaclePair pair);

void drawBear(Bear *b);
void resetObjects(Honey *jar, Obstacle obs[], int arrLen);
void handleSpeed(Vector2 *dt);


Rectangle obstacleInit[] = {
    { 30, HEIGHT/2, 150, 150 },
    { 500, HEIGHT/3, 70, 200 },
    { 300, 250, 30, 50 },
    { 800, 500, 80, 110 }
};


void insertPair(ObstaclePairArray *o, ObstaclePair *newPair)
{
    printf("Inserting: (%d, %d) @ %d\n", newPair->actor, newPair->subject, o->len);
    o->items[o->len] = *newPair;
    o->len = o->len + 1;
}

void printArray(ObstaclePairArray* arr)
{
    // for (int i=0; i <= arr->cap; i++) {
    for (int i=0; i <= arr->len; i++) {
        ObstaclePair* el = &arr->items[i];
        printf("idx %d: (%d, %d)\n", i, el->actor, el->subject);
    }
}


int main()
{
	InitWindow(WIDTH, HEIGHT, "Sticky Paws");
    Texture2D picnicBlanket = LoadTexture("assets/picnic_blanket_grass.png");

    greet();

    double currentTime, lastTime, timerPrev;
    // float mouseSpeed, absMouseDelta, speedDecrease;
    float speedDecrease;
    int sensitivity = 50;
    bool warning = false;

    GAMESTATE = START;

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
        .radius = 50.0f,
        .stuck = false,
        .value = 50,
    };

    UserInterface GameUI = {
        .infoBox = { 0, 0, 400, 100 },
        .barMax = GameUI.infoBox.width - (40),
        .startButton = { WIDTH/2 - 200, HEIGHT-120, 400, 100 },
        .splashScreen = LoadTexture("assets/bear_splash.jpg"),
        .failScreen = LoadTexture("assets/bear_jail.png"),
        .wakeStates = {
            LoadTexture("assets/tv_asleep.png"),
            LoadTexture("assets/tv_1.png"),
            LoadTexture("assets/tv_2.png"),
            LoadTexture("assets/tv_3.png"),
        }
    };

    ObstaclePairArray obsPairArr = {
        .items = {},
        .len = 0,
        .cap = OBS_ARR_SIZE,
    };
    obsPairArr.items = (ObstaclePair*)malloc(
        sizeof(ObstaclePair) * OBS_ARR_SIZE
    );

    // reset mouse so bear paw isn't in top right
    SetMousePosition(HEIGHT-50, WIDTH/2);

	while (!WindowShouldClose()) {
        // mouse position diff used to stuck object movement
        Vector2 mouseDelta = GetMouseDelta();

        if (IsKeyPressed(KEY_TAB))
        {
            (DEBUG) ? (DEBUG = false) : (DEBUG = true);
        }
        if (IsKeyPressed(KEY_R))
        {
            GAMESTATE = PLAY;
        }

        // debug printing
        // if (DEBUG && (mouseDelta.x != 0 && mouseDelta.y != 0) )
        // {
        //     printf("mouse dx, xy: %0.2f, %0.2f \n", mouseDelta.x, mouseDelta.y);
        //     printf("mouse total: %0.2f \n", fabs(mouseDelta.x + mouseDelta.y));
        //     printf("TOTAL_SPEED: %d \n", TOTAL_SPEED);
        // }

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
            handleObjectPushing(&obsPairArr, obstacles, obstaclesLen, &Jar, &mouseDelta);

            handleSpeed(&mouseDelta);

            // decrease speed total each frame
            if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL) )
            {
                lastTime = currentTime;
                speedDecrease = fabs(TOTAL_SPEED - DECAY);
                TOTAL_SPEED = (int)speedDecrease;
            }

            // speed bar update logic
            GameUI.barWidth = TOTAL_SPEED * 4; // *4 because TOTAL_SPEED is out of 100, bar width is 400. TODO: make work with x=n

            // put max limit on the width
            if (GameUI.barWidth > GameUI.barMax)
            {
                GameUI.barWidth = GameUI.barMax;
            }

            // win game logic
            if (Jar.pos.y >= HEIGHT) { GAMESTATE = WIN; }

            // set flag for warning message
            ( TOTAL_SPEED >= sensitivity ) ? (warning = true) : (warning = false);
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
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (GAMESTATE == START) {
                DrawTexture(GameUI.splashScreen, 0, 0, WHITE);
                drawButton("PLAY", GameUI.startButton);
            }

            if (GAMESTATE == PLAY) {
                DrawTexture(picnicBlanket, 0, 0, WHITE);    // draw background image

                for (int i=0; i <= obstaclesLen; i++)
                {
                    Obstacle obs = obstacles[i];
                    DrawRectangleRec(obs.rect, BLACK);      // draw obstacles
                }

                DrawTextureV(Jar.tex, Jar.hitbox, WHITE);   // draw honey Jar
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
            }

		EndDrawing();
	}

    // clean up resources
    UnloadTexture(picnicBlanket);
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

bool checkSubjectActorPairs(ObstaclePairArray *o, int a, int s)
{   // iterate through pair array and see if a/s exists
    for (int i=0; i <= o->len; i++) {

        ObstaclePair pair = o->items[i];

        if (pair.actor == s && pair.subject == a) {
            printf("pairs!: (%d, %d) (%d, %d)\n", a, s, pair.actor, pair.subject);
            return true;
        } else {
            // check if pair exists to avoid duplicate inserts
            if ( !pairInArray(o, (ObstaclePair){a, s} ) ) {
                printf("Inserting new Pair... (%d, %d)\n", a, s);
                insertPair(o, &(ObstaclePair){a, s});
            }
            return false;
        }
    }
    // check if the subject is already acting on the actor
    // i.e
    //  objA (Actor) -> objB (Subject)
    //      A pushes B
    //      add to pairs
    //  objB (Actor) -> objA (Subject)
    //      check pairs
    //      if (present)
    //          pass
    //      else
    //          push
    return false;
}

bool pairInArray(ObstaclePairArray *o, ObstaclePair pair)
{
    for (int i=0; i <= o->len; i++) {
        ObstaclePair p = o->items[i];
        if (p.actor == pair.actor && p.subject == pair.subject) {
            printf("Pair already in array! (%d, %d)\n", p.actor, p.subject);
            return true;
        }
    }
    return false;
}

void handleObjectPushing(ObstaclePairArray *o, Obstacle obs[], int arrLen, Honey *jar, Vector2 *dt)
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
                // obstaclePairs
                bool isAlreadyActing = checkSubjectActorPairs(o, i, j);
                // printArray(o);
                if (isAlreadyActing) {
                    subject->rect.x = subject->rect.x + dt->x;
                    subject->rect.y = subject->rect.y + dt->y;
                }
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

void resetObjects(Honey *jar, Obstacle obs[], int arrLen)
{
    // reset scores
    TOTAL_SPEED = 0;
    SCORE = 0;
    TIMER = 30;

    // reset honey jar
    jar->stuck = false;
    jar->pos = (Vector2){ WIDTH/2 + 50, 100 };
    jar->hitbox = jar->pos;

    // loop through obstacles and set to original x/y
    for (int i = 0; i <= arrLen; i++) {
        Obstacle *o = &obs[i];
        o->stuck = false;
        o->rect = obstacleInit[i];
    }
}

void handleSpeed(Vector2 *dt)
{
    float absMouseDelta, mouseSpeed;

    // speed increase
    if (dt->x != 0 && dt->y != 0)
    {
        absMouseDelta = fabs(dt->x) + fabs(dt->y);
        mouseSpeed = absMouseDelta;
        TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed;
    } else if (dt->x == 0 && dt->y != 0)
    {
        absMouseDelta = fabs(dt->y);
        mouseSpeed = absMouseDelta;
        TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed*2;
    } else if (dt->x != 0 && dt->y == 0)
    {
        absMouseDelta = fabs(dt->x);
        mouseSpeed = absMouseDelta;
        TOTAL_SPEED = TOTAL_SPEED + (int)mouseSpeed*2;
    }

    // limit the total speed
    if (TOTAL_SPEED > TOTAL_SPEED_MAX)
    {
        TOTAL_SPEED = TOTAL_SPEED_MAX;
        if (!DEBUG) // TEMP: removes fail state for testing
            GAMESTATE = FAIL;
    }
}

void drawBear(Bear *b)
{
    // draw bear paw
    DrawTexture(b->tex, b->pos.x, b->pos.y, WHITE);

    float noseThreshold = HEIGHT - b->nose.height;
    Vector2 nosePos = { (WIDTH/2)-150, b->pos.y + HEIGHT*0.50 };
    // TODO: maybe use below nose movement logic instead?
    // Vector2 nosePos = { (Paw.pos.x-150), Paw.pos.y + HEIGHT*0.50 };

    // limit nose position past the bottom of the texture
    if ( nosePos.y <= noseThreshold ) {
        nosePos.y = noseThreshold;
    };
    DrawTextureV(b->nose, nosePos, WHITE);
}
