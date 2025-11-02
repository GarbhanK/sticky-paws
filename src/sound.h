/**
 * @file sound.h
 * @brief Sound management and audio effects for Sticky Paws
 *
 * This file manages all game sound effects including loading,
 * unloading, and playing various audio clips.
 */
#ifndef SOUND_H
#define SOUND_H

#include "raylib.h"

/**
 * @brief Sound effect identifiers
 */
typedef enum {
  GROWL1,       // Bear growl variant 1
  GROWL2,       // Bear growl variant 2
  GROWL3,       // Bear growl variant 3 (when paw near bottom)
  DOOR_SLAM,    // Door slam on fail
  SNIFF,        // Bear sniffing
  FANFARE,      // Victory music
  SELECT,       // UI button selection
  SNORE,        // Old man snoring
  HUH,          // Old man waking up
  MIRANDA,      // Miranda rights (fail state)
  SIREN,        // Police siren (fail state)
  CLOTH_RUSTLE, // Picnic blanket rustling
  SOUND_COUNT,  // Total number of sounds (must be last)
} SoundID;

#define MAX_SOUNDS SOUND_COUNT

// Global array of sounds
extern Sound sounds[SOUND_COUNT];

/**
 * @brief Play a random bear growl sound
 * @param sounds Array of loaded sounds
 */
void randomBearSound(Sound sounds[]);

/**
 * @brief Load all game sounds from disk
 * @param sounds Array to load sounds into
 */
void loadSounds(Sound sounds[]);

/**
 * @brief Unload all game sounds from memory
 * @param sounds Array of sounds to unload
 */
void unloadSounds(Sound sounds[]);

/**
 * @brief Stop all currently playing sounds
 * @param sounds Array of sounds to stop
 */
void stopAllSounds(Sound sounds[]);

#endif // SOUND_H
