#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

#include "game.h"
#include "sound.h"
#include "ui.h"

#define MAX_SOUNDS SOUNDS_COUNT

void unloadTextures(UserInterface *ui, Target *jar, Bear *paw);

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

  Target Jar = {
      .tex = LoadTexture("assets/honey.png"),
      .pos = {WIDTH / 2 - 200, 100},
      .stuck = false,
      .value = 50,
  };

  // Hazard Gun = {
  //   .hitbox = { 0, 0, 100, 100 },
  //   .tex = LoadTexture("assets/gun.png"),
  //   .pos = { 0, 0 },
  // };

  UserInterface GameUI = {.infoBox = {0, 0, 400, 100},
                          .barMax = (GameUI.infoBox.width - 40),
                          .startButton = {WIDTH / 2 - 200, HEIGHT - 120, 350, 80},
                          .resetButton = {WIDTH / 2 - 200, 50, 350, 80},
                          .tutorialButton = {WIDTH /2 + 165, HEIGHT - 120, 80, 80},
                          .background = LoadTexture("assets/picnic_blanket_grass.png"),
                          .splashScreen = LoadTexture("assets/bear_splash.jpg"),
                          .failScreen = LoadTexture("assets/bear_jail.png"),
                          .title = LoadTexture("assets/title_card.png"),
                          .winScreen = LoadTexture("assets/victory_bear.png"),
                          .wakeStates = {
                              LoadTexture("assets/tv_asleep.png"),
                              LoadTexture("assets/tv_1.png"),
                              LoadTexture("assets/tv_2.png"),
                              LoadTexture("assets/tv_3.png"),
                          },
                          .wakeStatesLen = 4};

  // reset mouse so bear paw isn't in top right
  SetMousePosition(HEIGHT - 50, WIDTH / 2);

  const float bobAmplitude = 10.0f;
  const float duration = 1.0f;
  const float frequency = 1.0f / (duration * 2);
  const float bobSpeed = 2 * PI * frequency;

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
      if (isButtonPressed(GameUI.tutorialButton)) {
        PlaySound(sounds[SELECT]);
        if (SHOW_TUTORIAL) {
          SHOW_TUTORIAL = false;
        } else {
          SHOW_TUTORIAL = true;
        }
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
      if (Jar.pos.y >= (HEIGHT-15) ) {
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
      if (isButtonPressed(GameUI.resetButton)) {
        stopAllSounds(sounds);
        PlaySound(sounds[SELECT]);
        resetObjects(&Jar, &Obs);
        GAMESTATE = START;
        winStateEntered = false;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (GAMESTATE == START) {
      float bobOffset = sinf(GetTime() * bobSpeed) * bobAmplitude;
      Vector2 titlePos = {0, 0-(int)bobOffset};

      DrawTextureEx(GameUI.splashScreen, (Vector2){0.0}, 0, 1.2, WHITE);
      DrawTextureEx(GameUI.title, titlePos, 0, 0.75, WHITE);
      drawButton("PLAY", GameUI.startButton);
      drawButton("?", GameUI.tutorialButton);
      if (SHOW_TUTORIAL)
        drawTutorial();
    }

    if (GAMESTATE == PLAY) {
      DrawTexture(GameUI.background, 0, 0, WHITE);

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
      DrawTextureEx(GameUI.winScreen, (Vector2){0, 0}, 0, 1.0f, WHITE);
      drawButton("RESTART", GameUI.resetButton);
    }

    EndDrawing();
  }

  unloadSounds(sounds);
  CloseAudioDevice();

  unloadTextures(&GameUI, &Jar, &Paw);
  CloseWindow();
  return 0;
}
