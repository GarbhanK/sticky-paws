#include "raylib.h"

#define MAX_SOUNDS SOUNDS_COUNT

typedef struct {
    Sound growl1;
    Sound growl2;
    Sound growl3;
    Sound door_slam;
    Sound stick;
    Sound drag;
} SoundBank;

typedef enum {
    GROWL1,
    GROWL2,
    GROWL3,
    DOOR_SLAM,
    STICK,
    DRAG,
    SOUND_COUNT,
} SoundID;

void randomBearSound(Sound sounds[]);
void loadSounds(Sound sounds[]);
void unloadSounds(Sound sounds[]);
