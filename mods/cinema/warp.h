// Cinema: warps to any scene and arrival point.
// From Slopped Up Sloperina (sloperinaGZ 0.0.4.2 by PracticeROM), GPL-3.0-or-later: see warp.c.
#pragma once

#include <stdint.h>

enum { PC_CHILD, PC_ADULT, PC_DUNGEON, PC_BOSS, PC_OTHER, PC_COUNT };

// starter: an actor in the scene shares the spawn's name (an exit), which the standard arrival
// event needs as its event starter.
typedef struct { const char *label, *pos, *event; uint8_t starter; } spawn_t;
// A scene: its name in the game, a label, its category, whether it's a dungeon, and its spawns
// (spawns[first] to spawns[first + n - 1]).
typedef struct { const char *name, *label; uint8_t category, dungeon; uint16_t first, n; } place_t;

extern const char *const places_categories[PC_COUNT];
extern const place_t places[];
extern const int places_count;
extern const spawn_t spawns[];

void warp_go(const place_t *place, const spawn_t *spawn);   // the game's thread
void warp_frame(void);                                       // every frame, the game's thread
const char *warp_status(void);                               // a line for the panel
int warp_pending(void);                                      // requested or armed, not happened yet
int warp_abandoned(void);                                    // timed out, skipped or impossible
