#include "sound.h"

#include "raylib.h"

void randomBearSound(Sound sounds[]) {
  switch (GetRandomValue(1, 2)) {
    case 1:
      PlaySound(sounds[GROWL1]);
      break;
    case 2:
      PlaySound(sounds[GROWL2]);
      break;
  }
}

void loadSounds(Sound sounds[]) {
  sounds[GROWL1] =
      LoadSound("assets/sfx/zapsplat_animals_bear_grunt_001_17143.mp3");
  sounds[GROWL2] =
      LoadSound("assets/sfx/zapsplat_animals_bear_grunt_002_17144.mp3");
  sounds[GROWL3] = LoadSound("assets/sfx/animals_bear_growl_grunt_003.mp3");
  sounds[DOOR_SLAM] = LoadSound("assets/sfx/door_slam.mp3");
  sounds[SNIFF] = LoadSound("assets/sfx/sniff.mp3");
}

void unloadSounds(Sound sounds[]) {
  for (int i = 0; i < SOUND_COUNT; i++) {
    UnloadSound(sounds[i]);
  }
}
