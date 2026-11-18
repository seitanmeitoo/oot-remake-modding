// Cinema: Link's age, what he wears, and his health kept full.
// From Slopped Up Sloperina (sloperinaGZ 0.0.4.2 by PracticeROM), GPL-3.0-or-later: see link.c.
#pragma once

#include <stdint.h>

enum { AGE_CHILD, AGE_ADULT };
int age_current(void);               // the game's, or -1 if unknown
const char *age_reload(int age);     // reload the scene at this age; an error line, or null once started
void age_frame(void);                // every frame, after warp_frame: undoes an age change whose reload failed

enum { EQ_SWORD, EQ_SHIELD, EQ_TUNIC, EQ_BOOTS, EQ_COUNT };
enum { FOR_CHILD = 1, FOR_ADULT = 2 };
typedef struct { const char *game, *label; uint8_t slot, ages; } gear_t;   // game "" = nothing / the default
extern const char *const gear_slots[EQ_COUNT];
extern const gear_t gear[];
extern const int gear_count;
int gear_worn(int index);            // 1 if Link wears gear[index]
const char *gear_wear(int index);    // puts it on; a line for the panel, or gear_reload_needed
extern const char gear_reload_needed[];   // the equipment wasn't found: reload the scene to apply it
void link_seen(void *object, void *actor);   // Core's Link objects, from player_move

void health_refill(void);            // Link's health to its maximum
const char *link_scene(void);        // the scene's name (Sequence_CurrentBanc), or null
