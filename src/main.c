#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

#include "game.h"
#include "sound.h"
#include "ui.h"

#define MAX_SOUNDS SOUNDS_COUNT

void unloadTextures(UserInterface *ui, Honey *jar, Bear *paw);
void flashHappyBear(UserInterface *ui);


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
  bool winStateEntered = false;
  bool isSnoring = false;
  // bool paw_moving = false;

  GAMESTATE = START;

  // additional obsacles, e.g other picnic items
  Obstacle obstacles[] = {
      {obstacleInit[0], false, 10, LoadTexture("assets/grapes.png")},
      {obstacleInit[1], false, 10, LoadTexture("assets/baguette.png")},
      {obstacleInit[2], false, 10, LoadTexture("assets/cheese.png")},
      {obstacleInit[3], false, 10, LoadTexture("assets/cigs.png")},
      {obstacleInit[4], false, 10, LoadTexture("assets/wine.png")},
  };

  // TODO: change for existing obstacles[] array
  ObstacleArray Obs = {
      .init = obstacleInit,
      .items = obstacles,
      .length = sizeof(obstacles) / sizeof(Obstacle),
  };

  Bear Paw = {.tex = LoadTexture("assets/sticky_paw.png"),
              .nose = LoadTexture("assets/bear_nose.png")};

  Honey Jar = {
      .tex = LoadTexture("assets/honey.png"),
      .pos = {WIDTH / 2 - 200, 100},
      .stuck = false,
      .value = 50,
  };

  UserInterface GameUI = {.infoBox = {0, 0, 400, 100},
                          .barMax = (GameUI.infoBox.width - 40),
                          .startButton = {WIDTH / 2 - 200, HEIGHT - 120, 350, 80},
                          .background = LoadTexture("assets/picnic_blanket_grass.png"),
                          .splashScreen = LoadTexture("assets/bear_splash.jpg"),
                          .failScreen = LoadTexture("assets/bear_jail.png"),
                          .title = LoadTexture("assets/title_card.png"),
                          .winScreen = LoadTexture("assets/winner_bear.jpg"),
                          .wakeStates = {
                              LoadTexture("assets/tv_asleep.png"),
                              LoadTexture("assets/tv_1.png"),
                              LoadTexture("assets/tv_2.png"),
                              LoadTexture("assets/tv_3.png"),
                          },
                          .wakeStatesLen = 4};

  // reset mouse so bear paw isn't in top right
  SetMousePosition(HEIGHT - 50, WIDTH / 2);

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

    UpdateFadeIn(&fade, 0.005f);

    // start screen
    if (GAMESTATE == START) {
      if (isButtonPressed(GameUI.startButton)) {
        PlaySound(sounds[SELECT]);
        resetObjects(&Jar, &Obs);
        GAMESTATE = PLAY;
      }
    }

    if (GAMESTATE == PLAY) {
      currentTime = GetTime();

      // decrease timer every second (1.0 = 1 sec)
      if ((currentTime - timerPrev) >= 1.0) {
        timerPrev = currentTime;
        TIMER = TIMER - 1;
      }

      if (TIMER == 0) {
        GAMESTATE = FAIL;
      }

      // update paw movement
      Paw.pos.x = GetMouseX() - (float)Paw.tex.width / 2;
      Paw.pos.y = GetMouseY();
      Paw.hitbox = (Rectangle){Paw.pos.x, Paw.pos.y, Paw.tex.width, Paw.tex.height};

      // update Jar hitbox
      Jar.hitbox =
          (Rectangle){Jar.pos.x + 10, Jar.pos.y + 15, Jar.tex.width - 20, Jar.tex.height - 25};

      // handle sticky logic
      handleStickyJar(&Paw, &Jar, sounds);
      handleStickyObstacle(&Paw, &Obs, sounds);

      // handle pushing logic
      // handlePawPushing(&Paw, obstacles, obstaclesLen, &mouseDelta);
      handleObjectPushing(&Obs, &Jar, &mouseDelta);
      handleSpeed();

      // decrease speed total each frame
      if (TOTAL_SPEED > 0 && ((currentTime - lastTime) >= TIME_INTERVAL)) {
        lastTime = currentTime;
        speedDecrease = fabs(TOTAL_SPEED - DECAY);
        TOTAL_SPEED = (int)speedDecrease;
      }

      //  === SFX TRIGGERS ===
      if (Paw.pos.y > HEIGHT * 0.75)
        PlaySound(sounds[GROWL3]);

      if ((Paw.pos.y + HEIGHT * 0.5) <= (HEIGHT - Paw.nose.height))
        if (!IsSoundPlaying(sounds[SNIFF])) {
          PlaySound(sounds[SNIFF]);
        }

      if (!isSnoring && getOldManState() <= 2) {
          isSnoring = true;
          SetSoundVolume(sounds[SNORE], 0.2);
          SetSoundPan(sounds[SNORE], 0.25);
          PlaySound(sounds[SNORE]);
      } else if (isSnoring && getOldManState() == 3) {
          isSnoring = false;
          StopSound(sounds[SNORE]);
          PlaySound(sounds[HUH]);
      }

      // picnic blanket rustling when paw moving (TODO: change is so when obj is moving?)
      bool paw_moving = (mouseDelta.x + mouseDelta.y) != 0;
      const Sound cloth = sounds[CLOTH_RUSTLE];
      if (paw_moving) {
        ( IsSoundPlaying(cloth) ) ? ResumeSound(cloth) : PlaySound(cloth);
      } else {
        if (IsSoundPlaying(cloth)) { PauseSound(cloth); }
      }

      // win game logic
      if (Jar.pos.y >= HEIGHT) {
        GAMESTATE = WIN;
      }
    }

    if (GAMESTATE == FAIL) {
      // check if we've just eentered the fail state
      if (!failStateEntered) {
        StartFadeIn(&fade);
        stopAllSounds(sounds);
        PlaySound(sounds[DOOR_SLAM]);
        PlaySound(sounds[SIREN]);
        PlaySound(sounds[MIRANDA]);
        failStateEntered = true;
      }

      if (IsKeyPressed(KEY_ENTER)) {
        GAMESTATE = START;
        failStateEntered = false;
      }

      if (isButtonPressed(GameUI.startButton)) {
        stopAllSounds(sounds);
        PlaySound(sounds[SELECT]);
        resetObjects(&Jar, &Obs);
        GAMESTATE = PLAY;
        failStateEntered = false;
      }
    } else {
      failStateEntered = false;
    }

    if (GAMESTATE == WIN) {
      if (!winStateEntered) {
        stopAllSounds(sounds);
        PlaySound(sounds[FANFARE]);
        winStateEntered = true;
      }

      // reset game
      if (isButtonPressed(GameUI.startButton)) {
        // StopSound(sounds[FANFARE]);
        stopAllSounds(sounds);
        PlaySound(sounds[SELECT]);
        resetObjects(&Jar, &Obs);
        GAMESTATE = PLAY;
        winStateEntered = false;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (GAMESTATE == START) {
      DrawTextureEx(GameUI.splashScreen, (Vector2){0.0}, 0, 1.2, WHITE);
      // DrawTextureEx(GameUI.title, (Vector2){0, 0}, 0, 0.75, WHITE);
      drawButton("PLAY", GameUI.startButton);
      DrawText("How to Play", 90, 45, 30, BLACK);
      DrawText(
          "* Control the Bear paw using the mouse\n* Take as many snacks as you can before time runs out\n* Don't wake up walt.",
          75, 80, 20, BLACK
      );
    }

    if (GAMESTATE == PLAY) {
      DrawTexture(GameUI.background, 0, 0, WHITE);
      // flashHappyBear(&GameUI);   // draw background image

      // draw obstacles
      for (int i = 0; i < Obs.length; i++) {
        Obstacle *obs = &Obs.items[i];
        Vector2 obs_pos = (Vector2){obs->rect.x, obs->rect.y};
        DrawTextureEx(obs->tex, obs_pos, 0.0f, 1.0f, WHITE);

        // NOTE: hitbox testing
        if (DEBUG)
          DrawRectangleLinesEx(rectToHitbox(*obs, HITBOX_SHRINK_PERC), 2, GREEN);
      }

      // draw honey Jar
      DrawTexture(Jar.tex, Jar.pos.x, Jar.pos.y, WHITE);
      if (DEBUG)
        DrawRectangleLinesEx(Jar.hitbox, 2, GREEN); // DEBUG HONEY HITBOX

      drawBear(&Paw);
      drawUI(&GameUI, warning, GameUI.barWidth);

      if (fade.active) {
        DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, fade.alpha));
      }
    }

    if (GAMESTATE == FAIL) {
      DrawTexture(GameUI.failScreen, 0, 0, WHITE);
      // drawCenterText("FAIL", RED, 200, (Vector2){WIDTH * 0.5, HEIGHT * 0.6});
      drawButton("RESTART", GameUI.startButton);
      if (fade.active) {
        DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, fade.alpha));
      }
    }

    if (GAMESTATE == WIN) {
      // DrawTextureEx(GameUI.winScreen, (Vector2){(0 - GameUI.winScreen.width*0.5),0} , 0, 1, WHITE);
      DrawTextureEx(GameUI.winScreen, (Vector2){0-200, 0}, 0, 1.15, WHITE);
      // drawCenterText("WIN", RED, 100, (Vector2){WIDTH * 0.5, HEIGHT * 0.3});
      drawButton("RESTART", GameUI.startButton);
    }

    EndDrawing();
  }

  unloadSounds(sounds);
  CloseAudioDevice();

  unloadTextures(&GameUI, &Jar, &Paw);
  CloseWindow();
  return 0;
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
    if (alpha < 0)
      alpha = 0; // clamp to 0
  }

  DrawTextureRec(image, (Rectangle){0, 0, image.width, image.height}, (Vector2){200, 150},
                 Fade(WHITE, alpha));
}
