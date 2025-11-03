#include "sound.h"
#include "utils.h"

#include "raylib.h"
#include <stdio.h>

Sound sounds[SOUND_COUNT];

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
  sounds[GROWL1] = LoadSound(getAssetPath("sfx/growl1.mp3"));
  sounds[GROWL2] = LoadSound(getAssetPath("sfx/growl2.mp3"));
  sounds[GROWL3] = LoadSound(getAssetPath("sfx/growl3.mp3"));
  sounds[DOOR_SLAM] = LoadSound(getAssetPath("sfx/door_slam.mp3"));
  sounds[SNIFF] = LoadSound(getAssetPath("sfx/sniff.mp3"));
  sounds[FANFARE] = LoadSound(getAssetPath("sfx/ultraman_dyna_fanfare.mp3"));
  sounds[SELECT] = LoadSound(getAssetPath("sfx/blip_select.wav"));
  sounds[SNORE] = LoadSound(getAssetPath("sfx/base_snore.mp3"));
  sounds[HUH] = LoadSound(getAssetPath("sfx/huh.mp3"));
  sounds[MIRANDA] = LoadSound(getAssetPath("sfx/miranda.mp3"));
  sounds[SIREN] = LoadSound(getAssetPath("sfx/police_siren.mp3"));
  sounds[CLOTH_RUSTLE] = LoadSound(getAssetPath("sfx/cloth_rustle.mp3"));

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
