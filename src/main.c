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

typedef struct {
    float alpha;
    bool active;
} FadeEffect;

void StartFadeIn(FadeEffect *fade);
void UpdateFadeIn(FadeEffect *fade, float fadeSpeed);

int main()
{
    InitWindow(WIDTH, HEIGHT, "Sticky Paws");
    SetTargetFPS(60);
    InitAudioDevice();
    loadSounds(sounds);
    double currentTime, lastTime, timerPrev;
    float speedDecrease;
    bool warning = false;
    FadeEffect fade = {1.0f, true}; // start active with full black
    bool failStateEntered = false;

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
        if (DEBUG && IsKeyPressed(KEY_SPACE)) {
            StartFadeIn(&fade);
        }

        UpdateFadeIn(&fade, 0.01f);

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
                if (!IsSoundPlaying(sounds[SNIFF])) {
                    PlaySound(sounds[SNIFF]);
                }

            // win game logic
            if (Jar.pos.y >= HEIGHT) { GAMESTATE = WIN; }
        }

        if (GAMESTATE == FAIL) {
            // check if we've just eentered the fail state
            if (!failStateEntered) {
                StartFadeIn(&fade);
                StopSound(sounds[SNIFF]); // it's a long sound
                PlaySound(sounds[DOOR_SLAM]);
                failStateEntered = true;
            }

            if (IsKeyPressed(KEY_ENTER)) {
                GAMESTATE = START;
                failStateEntered = false;
            }

            if ( IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), GameUI.startButton) )
            {
                resetObjects(&Jar, &Obs);
                GAMESTATE = PLAY;
                failStateEntered = false;
            }
        } else {
            failStateEntered = false;
        }

        if (GAMESTATE == WIN) {
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
                if (fade.active) {
                    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, fade.alpha));
                }
                // flashHappyBear(&GameUI);   // draw background image

                for (int i=0; i <= Obs.length; i++) {
                    Obstacle *obs = &Obs.items[i];
                    DrawRectangleRec(obs->rect, BLACK);  // draw obstacles
                }

                DrawTexture(Jar.tex, Jar.pos.x, Jar.pos.y, WHITE);   // draw honey Jar
                if (DEBUG)
                    DrawRectangleRec(Jar.hitbox, GREEN);          // DEBUG HONEY HITBOX

                drawUI(&GameUI, warning, GameUI.barWidth);  // draw UI
                drawBear(&Paw);
            }

            if (GAMESTATE == FAIL) {
                DrawTexture(GameUI.failScreen, 0, 0, WHITE);
                drawCenterText("FAIL", RED, 200, (Vector2){ WIDTH*0.5, HEIGHT*0.6 });
                drawButton("RESTART", GameUI.startButton);
                if (fade.active) {
                    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, fade.alpha));
                }
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

// reset fade-in, can be called any time
void StartFadeIn(FadeEffect *fade) {
    fade->alpha = 1.0f;
    fade->active = true;
}

// call every frame
void UpdateFadeIn(FadeEffect *fade, float fadeSpeed) {
    if (fade->active) {
        fade->alpha -= fadeSpeed;
        if (fade->alpha <= 0.0f ) {
            fade->alpha = 0.0f;
            fade->alpha = false;    // stop sending once complete
        }
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
