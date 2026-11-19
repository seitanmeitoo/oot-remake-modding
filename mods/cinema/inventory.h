// Cinema: what Link owns (items, gear, masks, trade items, ammo, quest items, songs) and how far
// each quest is. From Slopped Up Sloperina (sloperinaGZ 0.0.4.2 by PracticeROM), GPL-3.0-or-later:
// see inventory.c.
#pragma once

#include <stdint.h>

typedef enum { FT_NONE, FT_INT, FT_BOOL, FT_ENUM, FT_FLOAT } field_type_t;
typedef struct { uint32_t hash; uint8_t type; } field_t;

typedef enum { IK_ACQUIRED, IK_OWNED, IK_TRADE_CHILD, IK_TRADE_ADULT, IK_OCARINA } item_kind_t;
typedef struct {
    const char *label;
    uint8_t     kind;
    field_t     owned;      // IK_OWNED: Int 0/1
    uint32_t    acquired;   // Bool "has been acquired" (what the item screen needs)
    const char *button;     // item-button choice name, or null
    uint32_t    worn_slot;  // GD_STR_* it's worn in, or 0
    const char *worn_item;  // the worn slot's value when worn
} inv_item_t;
typedef struct { const char *label, *item, *button; } pouch_one_t;   // one-off pouch items: one at a time
typedef struct { const char *label; uint32_t count, cap; int32_t fixed; uint32_t acquired; } ammo_t;
typedef struct { const char *label; field_t f[2]; uint32_t acquired; } flag_t;   // one or two fields set together
typedef struct { const char *label; uint32_t hash; uint8_t n_stages; const char *const *stages; } quest_t;

extern const inv_item_t inv_items[], inv_gear[];
extern const pouch_one_t inv_masks[], inv_trade_child[], inv_trade_adult[];
extern const ammo_t inv_ammo[];
extern const flag_t inv_quest_items[], inv_songs[];
extern const quest_t inv_quests[];
extern const int inv_n_items, inv_n_gear, inv_n_masks, inv_n_trade_child, inv_n_trade_adult, inv_n_ammo,
    inv_n_quest_items, inv_n_songs, inv_n_quests;

// All of these touch the game's data: call them on the game's thread, except the getters.
int inv_owned(const inv_item_t *item);
void inv_set_owned(const inv_item_t *item, int on);
int inv_one_chosen(const pouch_one_t *slot, int n);              // 1 + the item held, 0 for none
void inv_one_choose(const pouch_one_t *slot, int n, int choice);
void inv_fill_ammo(void);                                        // every count to its capacity
int inv_flag(const flag_t *flag);
void inv_set_flag(const flag_t *flag, int on);
void inv_own_worn(const char *worn_item);                        // gear just put on is owned too

int quest_stage(int quest);                                      // the stage index, or -1
void quest_set_stage(int quest, int stage);
void quests_set_all(int complete);                               // every quest done, or not started
