#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>
#include <raymath.h>

#include "game.h"
#include "sound.h"
#include "ui.h"

#define MAX_SOUNDS SOUNDS_COUNT

// array of sound variables
Sound sounds[SOUND_COUNT];

void unloadTextures(UserInterface *ui, Honey *jar, Bear *paw);
void flashHappyBear(UserInterface *ui);
void FadeInFromBlack(float *alpha, float fadeSpeed);


int main()
{
    InitWindow(WIDTH, HEIGHT, "Sticky Paws");
    SetTargetFPS(60);
    InitAudioDevice();
    loadSounds(sounds);
    double currentTime, lastTime, timerPrev;
    float speedDecrease;
    bool warning = false;
    float alpha = 1.0f;

    GAMESTATE = START;

    // additional obsacles, e.g other picnic items
    Obstacle obstacles[] = {
        { obstacleInit[0], false, 10 },
        { obstacleInit[1], false, 10 },
        { obstacleInit[2], false, 10 },
        { obstacleInit[3], false, 10 },
    };

    // TODO: change for existing obstacles[] array
    ObstacleArray Obs = {
        .init = obstacleInit,
        .items = obstacles,
        .length = sizeof(obstacles)/sizeof(Obstacle),
    };

    Bear Paw = {
        .tex = LoadTexture("assets/sticky_paw.png"),
        .nose = LoadTexture("assets/bear_nose.png")
    };

    Honey Jar = {
        .tex = LoadTexture("assets/honey.png"),
        .pos = { WIDTH/2 - 200, 100 },
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
        .title = LoadTexture("assets/title_card.png"),
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
                resetObjects(&Jar, &Obs);
                GAMESTATE = PLAY;
            }
        }

        if (GAMESTATE == PLAY) {
            currentTime = GetTime();
            FadeInFromBlack(&alpha, 0.01f);

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
            handleStickyJar(&Paw, &Jar, sounds);
            handleStickyObstacle(&Paw, &Obs, sounds);

            // handle pushing logic
            // handlePawPushing(&Paw, obstacles, obstaclesLen, &mouseDelta);
            handleObjectPushing(&Obs, &Jar, &mouseDelta);
            handleSpeed();

            // decrease speed total each frame
            if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL) )
            {
                lastTime = currentTime;
                speedDecrease = fabs(TOTAL_SPEED - DECAY);
                TOTAL_SPEED = (int)speedDecrease;
            }

            // sfx triggers
            if (Paw.pos.y > HEIGHT*0.75) PlaySound(sounds[GROWL3]);

            if ( (Paw.pos.y + HEIGHT*0.5) <= (HEIGHT - Paw.nose.height) )
                PlaySound(sounds[SNIFF]);

            // win game logic
            if (Jar.pos.y >= HEIGHT) { GAMESTATE = WIN; }
        }

        if (GAMESTATE == FAIL) {
            PlaySound(sounds[DOOR_SLAM]);

            if (IsKeyPressed(KEY_ENTER)) {
                GAMESTATE = START;
            }

            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, &Obs);
                GAMESTATE = PLAY;
            }
        }

        if (GAMESTATE == WIN) {
            // printf("You Win!");
            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, &Obs);
                GAMESTATE = PLAY;
            }
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (GAMESTATE == START) {
                DrawTexture(GameUI.splashScreen, 0, 0, WHITE);
                DrawTexture(GameUI.title, 0, 0, WHITE);
                drawButton("PLAY", GameUI.startButton);
            }

            if (GAMESTATE == PLAY) {
                DrawTexture(GameUI.background, 0, 0, WHITE);
                DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, alpha));
                // flashHappyBear(&GameUI);   // draw background image

                for (int i=0; i <= Obs.length; i++) {
                    Obstacle *obs = &Obs.items[i];
                    DrawRectangleRec(obs->rect, BLACK);  // draw obstacles
                }

                // if (IsKeyPressed(KEY_D)) {
                //     printf("Pressed D!\n");
                //     flashHappyBear(&GameUI);
                // }

                DrawTexture(Jar.tex, Jar.pos.x, Jar.pos.y, WHITE);   // draw honey Jar
                if (DEBUG)
                    DrawRectangleRec(Jar.hitbox, GREEN);          // DEBUG HONEY HITBOX

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

    unloadTextures(&GameUI, &Jar, &Paw);
    unloadSounds(sounds);

    CloseWindow();
    return 0;
}

void unloadTextures(UserInterface *ui, Honey *jar, Bear *paw) {
    // clean up resources
    UnloadTexture(ui->background);
    UnloadTexture(ui->failScreen);
    UnloadTexture(ui->splashScreen);
    UnloadTexture(ui->title);
    UnloadTexture(jar->tex);
    UnloadTexture(paw->tex);
    UnloadTexture(paw->nose);

    for (int i=0; i <= sizeof(ui->wakeStates); i++)
        UnloadTexture(ui->wakeStates[i]);
}

void FadeInFromBlack(float *alpha, float fadeSpeed) {
    if (*alpha > 0.0f) {
        *alpha -= fadeSpeed;
        if (*alpha < 0.0f) *alpha = 0.0f;  // Clamp to 0
    }
}

void flashHappyBear(UserInterface *ui)
{
    Texture2D image = ui->splashScreen;
    float alpha = 1.0f;
    float fadeSpeed = 1.5f;
    bool fading = false;
    float timer = 0.5f;

    if (timer > 0) {
        timer -= GetFrameTime();
    } else {
        fading = true;
    }

    if (fading) {
        alpha -= fadeSpeed * GetFrameTime();
        if (alpha < 0) alpha = 0;   // clamp to 0
    }

    DrawTextureRec(
        image,
        (Rectangle){0, 0, image.width, image.height},
        (Vector2){200, 150},
        Fade(WHITE, alpha)
    );
}
