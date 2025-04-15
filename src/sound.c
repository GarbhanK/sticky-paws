#include "sound.h"

#include "raylib.h"
#include <stdio.h>

void randomBearSound(Sound sounds[])
{
  switch (GetRandomValue(1, 2)) {
  case 1:
    PlaySound(sounds[GROWL1]);
    break;
  case 2:
    PlaySound(sounds[GROWL2]);
    break;
  }
}

void loadSounds(Sound sounds[])
{
  sounds[GROWL1] = LoadSound("assets/sfx/growl1.mp3");
  sounds[GROWL2] = LoadSound("assets/sfx/growl2.mp3");
  sounds[GROWL3] = LoadSound("assets/sfx/growl3.mp3");
  sounds[DOOR_SLAM] = LoadSound("assets/sfx/door_slam.mp3");
  sounds[SNIFF] = LoadSound("assets/sfx/sniff.mp3");
  sounds[FANFARE] = LoadSound("assets/sfx/ultraman_dyna_fanfare.mp3");
  sounds[SELECT] = LoadSound("assets/sfx/blip_select.wav");
  sounds[SNORE] = LoadSound("assets/sfx/base_snore.mp3");
  sounds[HUH] = LoadSound("assets/sfx/huh.mp3");
  sounds[MIRANDA] = LoadSound("assets/sfx/miranda.mp3");
  sounds[SIREN] = LoadSound("assets/sfx/police_siren.mp3");
  sounds[CLOTH_RUSTLE] = LoadSound("assets/sfx/cloth_rustle.mp3");

  // validate loaded sounds
  int valid_count = 0;
  for (int i=0; i < SOUND_COUNT; i++) {
    bool valid = IsSoundValid(sounds[i]);
    if (!valid) {
      printf("WARNING :: Invalid sound loaded!\n");
    } else {
      valid_count++;
    }
  }

  if (valid_count != SOUND_COUNT)
    printf("ERROR :: AAHHHHHHHHH!\n");
}

void unloadSounds(Sound sounds[])
{
  for (int i = 0; i < SOUND_COUNT; i++) {
    UnloadSound(sounds[i]);
  }
}

void stopAllSounds(Sound sounds[])
{
  for (int i = 0; i < SOUND_COUNT; i++) {
    if (IsSoundPlaying(sounds[i]))
      StopSound(sounds[i]);
    }
}
