#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "sound.h"
#include "ui.h"
#include "utils.h"

#define MAX_SOUNDS SOUND_COUNT
#define FADE_SPEED 0.005f
#define BOB_AMPLITUDE 10.0f
#define DURATION 1.0f
#define FREQUENCY 0.5f // 1.0f / (duration * 2);
#define BOB_SPEED 3.0f


static void handleStartState(GameContext *ctx, UserInterface *ui) {
  if (isButtonPressed(ui->startButton)) {
    PlaySound(sounds[SELECT]);
    resetObjects(ctx);
    ctx->state = PLAY;
  }

  if (isButtonPressed(ui->tutorialButton)) {
    PlaySound(sounds[SELECT]);
    ctx->showTutorial = !ctx->showTutorial;
  }
}

static void handlePlayState(GameContext *ctx, UserInterface *ui) {
  // get references to game entities
  Bear       *player = ctx->player;
  Target        *jar = ctx->jar;
  ObstacleArray *obs = ctx->obs;

  if (IsKeyPressed(KEY_TAB)) { ctx->debug = !ctx->debug; }

  // mouse position diff used to stuck object movement
  Vector2 mouseDelta = GetMouseDelta();
  double currentTime = GetTime();

  // decrease timer every second (1.0 = 1 sec)
  if ((currentTime - ctx->timerPrev) >= 1.0) {
    ctx->timerPrev = currentTime;
    ctx->timer = ctx->timer - 1;
  }

  // transition to fail state when timer runs out
  if (ctx->timer == 0) {
    ctx->state = FAIL;
  }

  // update paw movement
  float paw_x = GetMouseX() - (float)player->tex.width /2;
  float paw_y = GetMouseY();
  player->pos.x = paw_x;
  player->pos.y = paw_y;
  player->hitbox = (Rectangle){
    .x = paw_x,
    .y = paw_y,
    .width = player->tex.width,
    .height = player->tex.height
  };

  // update Jar hitbox
  jar->hitbox =
    (Rectangle){jar->pos.x + 10, jar->pos.y + 15, jar->tex.width - 20, jar->tex.height - 25};

  // handle sticky logic
  handleStickyJar(ctx, player, jar, sounds);
  handleStickyObstacle(ctx, player, obs, sounds);

  // handle pushing logic
  handleObjectPushing(obs, jar, &mouseDelta);
  handleSpeed(ctx);

  // decrease speed total each frame
  if (ctx->totalSpeed > 0 && ((currentTime - ctx->lastTime) >= TIME_INTERVAL)) {
    ctx->lastTime = currentTime;
    float speedDecrease = fabs(ctx->totalSpeed - DECAY);
    ctx->totalSpeed = (int)speedDecrease;
  }

  //  === SFX TRIGGERS ===
  if (player->pos.y > HEIGHT * 0.75)
    PlaySound(sounds[GROWL3]);

  if ((player->pos.y + HEIGHT * 0.5) <= (HEIGHT - player->nose.height))
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
  bool is_paw_moving = (mouseDelta.x + mouseDelta.y) != 0;
  const Sound cloth = sounds[CLOTH_RUSTLE];
  if (is_paw_moving) {
    ( IsSoundPlaying(cloth) ) ? ResumeSound(cloth) : PlaySound(cloth);
  } else {
    if (IsSoundPlaying(cloth)) { PauseSound(cloth); }
  }

  // win game logic (win condition different for fullscreen because mouse can't go below HEIGHT)
  const bool winConditionMet = (ctx->jar->stuck && GetMouseY() >= HEIGHT-15);

  if (winConditionMet) {
    // build the stuck obstacles array once before entering win state
    ctx->stuckObstacles = buildStuckObstacles(obs);
    ctx->state = WIN;
  }
}

static void handleFailState(GameContext *ctx, UserInterface *ui) {
  // check if we've just entered the fail state
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
    resetObjects(ctx);
    ctx->state = PLAY;
    ctx->failStateEntered = false;
  }
}

static void handleWinState(GameContext *ctx, UserInterface *ui) {
  if (!ctx->winStateEntered) {
    stopAllSounds(sounds);
    PlaySound(sounds[FANFARE]);
    ctx->winStateEntered = true;
  }

  // reset game
  if (isButtonPressed(ui->resetButton)) {
    stopAllSounds(sounds);
    PlaySound(sounds[SELECT]);
    resetObjects(ctx);
    ctx->state = START;
    ctx->winStateEntered = false;
  }
}

static void renderCurrentState(GameContext *ctx, UserInterface *ui) {
  Bear *paw          = ctx->player;
  Target *jar        = ctx->jar;
  ObstacleArray *obs = ctx->obs;

  if (ctx->state == START) {
    float bobOffset = sinf(GetTime() * BOB_SPEED) * BOB_AMPLITUDE;
    Vector2 titlePos = {0, 0-bobOffset};

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
      Obstacle *arr = obs->items;
      Obstacle obs = arr[i];
      Vector2 obs_pos = (Vector2){obs.hitbox.x, obs.hitbox.y};
      DrawTextureEx(obs.tex, obs_pos, 0.0f, 1.0f, WHITE);

      // NOTE: hitbox testing
      if (ctx->debug)
        DrawRectangleLinesEx(rectToHitbox(obs.hitbox, HITBOX_SHRINK_PERC), 2, GREEN);
    }

    // draw honey Jar
    DrawTexture(jar->tex, jar->pos.x, jar->pos.y, WHITE);
    if (ctx->debug)
      DrawRectangleLinesEx(jar->hitbox, 2, GREEN); // DEBUG HONEY HITBOX

    drawBear(paw);
    if (ctx->debug)
      DrawRectangleLinesEx(rectToHitbox(paw->hitbox, HITBOX_SHRINK_PERC), 2, GREEN);

    drawUI(ctx, ui, ctx->showWarning, ui->barWidth);

    if (ui->fade.active) {
      DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, ui->fade.alpha));
    }
  }

  if (ctx->state == FAIL) {
    DrawTexture(ui->failScreen, 0, 0, WHITE);
    drawButton("RESTART", ui->startButton);
    if (ui->fade.active) {
      DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, ui->fade.alpha));
    }
  }

  if (ctx->state == WIN) {
    DrawTextureEx(ui->winScreen, (Vector2){0, 0}, 0, 1.1f, WHITE);

    // draw the honey jar to the win screen
    DrawTextureEx(ctx->jar->tex, (Vector2){290, 570}, 0, 0.75f, WHITE);

    // draw stuck obstacles
    ObstacleArray *stuck = ctx->stuckObstacles;
    float x_index_offset = 300;

    for (int i = 0; i < stuck->length; i++) {
      Obstacle o = stuck->items[i];
      x_index_offset += o.tex.width;

      // calculate position based on obstacle index and texture width
      Vector2 pos = (Vector2){x_index_offset, 570};

      DrawTextureEx(o.tex, pos, 0, 0.5f, WHITE);
    }
    drawButton("RESTART", ui->resetButton);
  }
}

int main()
{
  InitWindow(WIDTH, HEIGHT, "Sticky Paws");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
  InitAudioDevice();
  loadSounds(sounds);

  Image icon = LoadImage(getAssetPath("honey.png"));
  SetWindowIcon(icon);

  GameContext ctx;
  initGameContext(&ctx);

  // additional obsacles, e.g other picnic items
  // NOTE: the `hitbox` field gets set to the `init` value when the game starts
  Obstacle obstacles[] = {
    {
      .tex = LoadTexture(getAssetPath("grapes.png")),
      .init = {200, HEIGHT / 2, 150, 150},
      .hitbox = {},
      .winPos = {290, 570},
      .value = 10,
      .stuck = false,
    },
    {
      .tex = LoadTexture(getAssetPath("baguette.png")),
      .init = {500, HEIGHT / 3, 110, 300},
      .hitbox = {},
      .winPos = {290, 570},
      .value = 10,
      .stuck = false,
    },
    {
      .tex = LoadTexture(getAssetPath("cheese.png")),
      .init = {300, 250, 115, 100},
      .hitbox = {},
      .winPos = {320, 570},
      .value = 10,
      .stuck = false,
    },
    {
      .tex = LoadTexture(getAssetPath("cigs.png")),
      .init = {800, 450, 90, 130},
      .hitbox = {},
      .winPos = {360, 570},
      .value = 10,
      .stuck = false,
    },
    {
      .tex = LoadTexture(getAssetPath("wine.png")),
      .init = {80, 115, 400, 100},
      .hitbox = {},
      .winPos = {390, 570},
      .value = 10,
      .stuck = false,
    },
  };
  int count = sizeof(obstacles) / sizeof(Obstacle);

  // allocate heap memory for the obstacles array and add it to the context
  ObstacleArray *obsArray = malloc(sizeof(ObstacleArray));
  obsArray->length = count;
  obsArray->capacity = count;
  obsArray->items = malloc(count * sizeof(Obstacle));
  memcpy(obsArray->items, obstacles, count * sizeof(Obstacle));

  ctx.obs = obsArray;

  // add player and target entities to the context
  ctx.player = &(Bear){
    .tex = LoadTexture(getAssetPath("sticky_paw.png")),
    .nose = LoadTexture(getAssetPath("bear_nose.png")),
  };

  ctx.jar = &(Target){
    .tex = LoadTexture(getAssetPath("honey.png")),
    .pos = {WIDTH / 2 - 200, 100},
    .stuck = false,
    .value = 50,
  };

  // Hazard Gun = {
  //   .hitbox = { 0, 0, 100, 100 },
  //   .tex = LoadTexture(getAssetPath("gun.png")),
  //   .pos = { 0, 0 },
  // };

  UserInterface GameUI = {
    .fade = {1.0f, true},
    .infoBox = {0, 0, 400, 100},
    .barMax = (360),  // calculate directly (infoBox.width - 40)
    .startButton = {WIDTH / 2 - 200, HEIGHT - 120, 350, 80},
    .resetButton = {WIDTH / 2 - 200, 50, 350, 80},
    .tutorialButton = {WIDTH /2 + 165, HEIGHT - 120, 80, 80},
    .background = LoadTexture(getAssetPath("picnic_blanket_grass.png")),
    .splashScreen = LoadTexture(getAssetPath("bear_splash.jpg")),
    .failScreen = LoadTexture(getAssetPath("bear_jail.png")),
    .title = LoadTexture(getAssetPath("title_card.png")),
    .winScreen = LoadTexture(getAssetPath("sunrise_bear.png")),
    .wakeStates = {
      LoadTexture(getAssetPath("tv_asleep.png")),
      LoadTexture(getAssetPath("tv_1.png")),
      LoadTexture(getAssetPath("tv_2.png")),
      LoadTexture(getAssetPath("tv_3.png")),
    },
    .wakeStatesLen = 4
  };

  // reset mouse so bear paw isn't in top right
  SetMousePosition(WIDTH / 2, HEIGHT - 50);

  while (!WindowShouldClose()) {
    UpdateFadeIn(&GameUI.fade, FADE_SPEED);

    switch (ctx.state) {
      case START: handleStartState(&ctx, &GameUI); break;
      case PLAY: handlePlayState(&ctx, &GameUI); break;
      case FAIL: handleFailState(&ctx, &GameUI); break;
      case WIN: handleWinState(&ctx, &GameUI); break;
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);
    renderCurrentState(&ctx, &GameUI);

    EndDrawing();
  }

  unloadSounds(sounds);
  unloadTextures(&GameUI, ctx.jar, ctx.player);

  // free allocated obstacle array
  freeObstacleArray(&ctx.obs);

  CloseAudioDevice();
  CloseWindow();
  return 0;
}
