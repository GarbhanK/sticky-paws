#ifndef SOUND_H
#define SOUND_H

#include "raylib.h"

#define MAX_SOUNDS SOUND_COUNT


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
  SOUND_COUNT,  // last SoundID element is the amount of enumerations
} SoundID;

// array of sounds
extern Sound sounds[SOUND_COUNT];

void randomBearSound(Sound sounds[]);
void loadSounds(Sound sounds[]);
void unloadSounds(Sound sounds[]);
void stopAllSounds(Sound sounds[]);

#endif // SOUND_H
