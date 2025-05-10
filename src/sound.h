#include "raylib.h"

#define MAX_SOUNDS SOUNDS_COUNT


typedef enum {
  GROWL1,
  GROWL2,
  GROWL3,
  DOOR_SLAM,
  SNIFF,
  FANFARE,
  SELECT,
  SNORE,
  HUH,
  MIRANDA,
  SIREN,
  CLOTH_RUSTLE,
  // STICK,
  SOUND_COUNT,
} SoundID;

// array of sounds
Sound sounds[SOUND_COUNT];

void randomBearSound(Sound sounds[]);
void loadSounds(Sound sounds[]);
void unloadSounds(Sound sounds[]);
void stopAllSounds(Sound sounds[]);
