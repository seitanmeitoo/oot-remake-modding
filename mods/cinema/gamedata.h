// Cinema: the game's live save data (GameData) and small memory helpers, for warp.c and link.c.
// From Slopped Up Sloperina (sloperinaGZ 0.0.4.2 by PracticeROM), GPL-3.0-or-later: see gamedata.c.
#pragma once

#include <kaegae.h>

#define MAIN(off)          (kae_game_base() + (uintptr_t)(off))
#define MAIN_FN(type, off) ((type)MAIN(off))

uintptr_t ptr_at(uintptr_t addr);           // the pointer at addr, or 0 if it isn't readable
int str_equal(const char *a, const char *b);
uint64_t now_tick(void);                    // svcGetSystemTick, 19.2 MHz
#define TICK_HZ 19200000ull

// String64 fields (MurmurHash3 of their names)
#define GD_STR_LEN      64
#define GD_STR_TUNIC    0x008D7F0Cu
#define GD_STR_BOOTS    0xD107A85Du
#define GD_STR_SHIELD   0xEE463235u
#define GD_STR_SWORD    0xA95A3521u
#define GD_STR_MAP      0x1D6189DAu         // Sequence_CurrentBanc
#define GD_STR_EVENT    0xA242CCC2u
#define GD_STR_STARTER  0x3AD1FD51u
#define GD_STR_NEXT_MAP 0xEE27BF71u         // Sequence_ReservedNextBanc
#define GD_STR_SPAWN    0x0B570AB2u         // Sequence_ReservedNextStartPosName

const char *gd_get_str(uint32_t hash);      // null if not found
int gd_set_str(uint32_t hash, const char *s);
int gd_get_bool(uint32_t hash, int *out);
int gd_set_bool(uint32_t hash, int value);
