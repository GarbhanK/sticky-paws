#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>

#include "game.h"
#include "sound.h"
#include "ui.h"

#define MAX_SOUNDS SOUNDS_COUNT
#define FADE_SPEED 0.005f
#define BOB_AMPLITUDE 10.0f
#define DURATION 1.0f
#define FREQUENCY 0.5f  // 1.0f / (duration * 2);
#define BOB_SPEED 3.0f

// Initialize with sensible defaults
void initGameContext(GameContext *ctx) {
    ctx->currentTime = 0.0;
    ctx->lastTime = 0.0;
    ctx->timerPrev = 0.0;
    ctx->state = START;
    ctx->timer = 60;  // or whatever your starting timer is
    ctx->totalSpeed = 0.0f;
    ctx->failStateEntered = false;
    ctx->winStateEntered = false;
    ctx->isSnoring = false;
    ctx->showTutorial = false;
    ctx->showWarning = false;
    ctx->debug = false;
}

void unloadTextures(UserInterface *ui, Target *jar, Bear *paw);

static void handleStartState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs);
static void handlePlayState(GameContext *ctx, UserInterface *ui, Bear *paw, Target *jar, ObstacleArray *obs);
static void handleFailState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs);
static void handleWinState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs);

static void handleStartState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs) {
  if (isButtonPressed(ui->startButton)) {
    PlaySound(sounds[SELECT]);
    resetObjects(ctx, jar, obs);
    ctx->state = PLAY;
  }

  if (isButtonPressed(ui->tutorialButton)) {
    PlaySound(sounds[SELECT]);
    if (ctx->showTutorial) {
      ctx->showTutorial = false;
    } else {
      ctx->showTutorial = true;
    }
  }

  if (IsKeyPressed(KEY_SPACE)) {
    ToggleFullscreen();
  }
}

static void handlePlayState(GameContext *ctx, UserInterface *ui, Bear *paw, Target *jar, ObstacleArray *obs) {
  if (IsKeyPressed(KEY_TAB)) {
    (ctx->debug) ? (ctx->debug = false) : (ctx->debug = true);
  }

  // mouse position diff used to stuck object movement
  Vector2 mouseDelta = GetMouseDelta();
  double currentTime = GetTime();

  // decrease timer every second (1.0 = 1 sec)
  if ((currentTime - ctx->timerPrev) >= 1.0) {
    ctx->timerPrev = currentTime;
    ctx->timer = ctx->timer - 1;
  }

  if (ctx->timer == 0) {
    ctx->state = FAIL;
  }

  // update paw movement
  paw->pos.x = GetMouseX() - (float)paw->tex.width / 2;
  paw->pos.y = GetMouseY();
  paw->hitbox = (Rectangle){paw->pos.x, paw->pos.y, paw->tex.width, paw->tex.height};

  // update Jar hitbox
  jar->hitbox =
      (Rectangle){jar->pos.x + 10, jar->pos.y + 15, jar->tex.width - 20, jar->tex.height - 25};

  // handle sticky logic
  handleStickyJar(paw, jar, sounds);
  handleStickyObstacle(paw, obs, sounds);

  // handle pushing logic
  // handlePawPushing(&Paw, obstacles, obstaclesLen, &mouseDelta);
  handleObjectPushing(obs, jar, &mouseDelta);
  handleSpeed(ctx);

  // decrease speed total each frame
  if (ctx->totalSpeed > 0 && ((currentTime - ctx->lastTime) >= TIME_INTERVAL)) {
    ctx->lastTime = currentTime;
    float speedDecrease = fabs(ctx->totalSpeed - DECAY);
    ctx->totalSpeed = (int)speedDecrease;
  }

  //  === SFX TRIGGERS ===
  if (paw->pos.y > HEIGHT * 0.75)
    PlaySound(sounds[GROWL3]);

  if ((paw->pos.y + HEIGHT * 0.5) <= (HEIGHT - paw->nose.height))
    if (!IsSoundPlaying(sounds[SNIFF])) {
      PlaySound(sounds[SNIFF]);
    }

  if (!ctx->isSnoring && getOldManState(ctx->totalSpeed) <= 2) {
      ctx->isSnoring = true;
      SetSoundVolume(sounds[SNORE], 0.2);
      SetSoundPan(sounds[SNORE], 0.25);
      PlaySound(sounds[SNORE]);
  } else if (ctx->isSnoring && getOldManState(ctx->totalSpeed) == 3) {
      ctx->isSnoring = false;
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

  // win game logic (win condition different for fullscreen because mouse can't go below HEIGHT)
  const bool winConditionMet = IsWindowFullscreen()
    ? (GetMouseY() >= HEIGHT - 5)
    : (jar->pos.y >= HEIGHT - 15);

  if (winConditionMet) {
    ctx->state = WIN;
  }
}

static void handleFailState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs) {
  // check if we've just eentered the fail state
  if (!ctx->failStateEntered) {
    // StartFadeIn(&fade);
    stopAllSounds(sounds);
    PlaySound(sounds[DOOR_SLAM]);
    PlaySound(sounds[SIREN]);
    PlaySound(sounds[MIRANDA]);
    ctx->failStateEntered = true;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    ctx->state = START;
    ctx->failStateEntered = false;
  }

  if (isButtonPressed(ui->startButton)) {
    stopAllSounds(sounds);
    PlaySound(sounds[SELECT]);
    resetObjects(ctx, jar, obs);
    ctx->state = PLAY;
    ctx->failStateEntered = false;
  }
}

static void handleWinState(GameContext *ctx, UserInterface *ui, Target *jar, ObstacleArray *obs) {
  if (!ctx->winStateEntered) {
    stopAllSounds(sounds);
    PlaySound(sounds[FANFARE]);
    ctx->winStateEntered = true;
  }

  // reset game
  if (isButtonPressed(ui->resetButton)) {
    stopAllSounds(sounds);
    PlaySound(sounds[SELECT]);
    resetObjects(ctx, jar, obs);
    ctx->state = START;
    ctx->winStateEntered = false;
  }
}

static void renderCurrentState(GameContext *ctx, UserInterface *ui, Bear *paw, Target *jar, ObstacleArray *obs) {
  if (ctx->state == START) {
    float bobOffset = sinf(GetTime() * BOB_SPEED) * BOB_AMPLITUDE;
    Vector2 titlePos = {0, 0-(int)bobOffset};

    DrawTextureEx(ui->splashScreen, (Vector2){0.0}, 0, 1.2, WHITE);
    DrawTextureEx(ui->title, titlePos, 0, 0.75, WHITE);
    drawButton("PLAY", ui->startButton);
    drawButton("?", ui->tutorialButton);
    if (ctx->showTutorial)
      drawTutorial();
  }

  if (ctx->state == PLAY) {
    DrawTexture(ui->background, 0, 0, WHITE);

    // draw obstacles
    for (int i = 0; i < obs->length; i++) {
      Obstacle* arr = obs->items;
      Obstacle obs = arr[i];
      Vector2 obs_pos = (Vector2){obs.rect.x, obs.rect.y};
      DrawTextureEx(obs.tex, obs_pos, 0.0f, 1.0f, WHITE);

      // NOTE: hitbox testing
      if (ctx->debug)
        DrawRectangleLinesEx(rectToHitbox(obs, HITBOX_SHRINK_PERC), 2, GREEN);
    }

    // draw honey Jar
    DrawTexture(jar->tex, jar->pos.x, jar->pos.y, WHITE);
    if (ctx->debug)
      DrawRectangleLinesEx(jar->hitbox, 2, GREEN); // DEBUG HONEY HITBOX

    drawBear(paw);
    drawUI(ctx, ui, ctx->showWarning, ui->barWidth);

    if (ui->fade.active) {
      DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, ui->fade.alpha));
    }
  }

  if (ctx->state == FAIL) {
    DrawTexture(ui->failScreen, 0, 0, WHITE);
    // drawCenterText("FAIL", RED, 200, (Vector2){WIDTH * 0.5, HEIGHT * 0.6});
    drawButton("RESTART", ui->startButton);
    if (ui->fade.active) {
      DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, ui->fade.alpha));
    }
  }

  if (ctx->state == WIN) {
    DrawTextureEx(ui->winScreen, (Vector2){0, 0}, 0, 1.0f, WHITE);
    drawButton("RESTART", ui->resetButton);
  }
}

int main()
{
  InitWindow(WIDTH, HEIGHT, "Sticky Paws");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  InitAudioDevice();
  loadSounds(sounds);

  Image icon = LoadImage("assets/honey.png");
  SetWindowIcon(icon);

  GameContext ctx;
  initGameContext(&ctx);

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

  Bear Paw = {
    .tex = LoadTexture("assets/sticky_paw.png"),
    .nose = LoadTexture("assets/bear_nose.png")
  };

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

  UserInterface GameUI = {
    .fade = {1.0f, true},
    .infoBox = {0, 0, 400, 100},
    .barMax = (360),  // calculate directly (infoBox.width - 40)
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
    .wakeStatesLen = 4
  };

  // reset mouse so bear paw isn't in top right
  SetMousePosition(WIDTH / 2, HEIGHT - 50);

  while (!WindowShouldClose()) {
    UpdateFadeIn(&GameUI.fade, FADE_SPEED);

    switch (ctx.state) {
      case START: handleStartState(&ctx, &GameUI, &Jar, &Obs); break;
      case PLAY: handlePlayState(&ctx, &GameUI, &Paw, &Jar, &Obs); break;
      case FAIL: handleFailState(&ctx, &GameUI, &Jar, &Obs); break;
      case WIN: handleWinState(&ctx, &GameUI, &Jar, &Obs); break;
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);
    renderCurrentState(&ctx, &GameUI, &Paw, &Jar, &Obs);

    EndDrawing();
  }

  unloadSounds(sounds);
  unloadTextures(&GameUI, &Jar, &Paw);

  CloseAudioDevice();
  CloseWindow();
  return 0;
}
