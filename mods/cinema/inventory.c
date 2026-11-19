// Cinema: what Link owns, and how far each quest is.
//
// Copyright (C) PracticeROM and sloperinaGZ's contributors; changes for Cinema by seitanmeitoo.
// This file is part of Cinema and is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version. See LICENSE.
//
// From Slopped Up Sloperina 0.4.0's menus.c (sloperinaGZ 0.0.4.2): field_get / field_set,
// get_owned / set_owned, get_trade / set_trade, the quest stages (get_stage / set_stage), and the
// ammo fill of cheats.c. Changed for Cinema: an item taken away is not taken off Link nor off the
// item buttons, and a trade or mask change doesn't update the item buttons (see the shortcut below);
// "every quest done / not started"; gear put on is owned too.
//
// shortcut: sloperinaGZ also clears the item buttons and takes worn gear off when an item goes
// (pouch.c's button watch and unequip). Without that, a removed item can stay on a button until the
// item screen is opened. Port pouch_clear_buttons if that gets in the way of a shot.
#include "gamedata.h"
#include "inventory.h"

static int field_get(field_t f, int32_t *value) {
    int flag;
    uint32_t choice;
    switch (f.type) {
    case FT_INT:
        return gd_get_int(f.hash, value);
    case FT_BOOL:
        if (!gd_get_bool(f.hash, &flag))
            return 0;
        *value = flag;
        return 1;
    case FT_ENUM:
        if (!gd_get_enum(f.hash, &choice))
            return 0;
        *value = (int32_t)choice;
        return 1;
    default:
        return 0;
    }
}

static void field_set(field_t f, int32_t value) {
    if (f.type == FT_INT)
        gd_set_int(f.hash, value);
    else if (f.type == FT_BOOL)
        gd_set_bool(f.hash, value != 0);
    else if (f.type == FT_ENUM)
        gd_set_enum(f.hash, (uint32_t)value);
}

// murmur3 of prefix + item + suffix ("PouchData.<item>.StockNum", "IsGet.<item>")
static uint32_t hash_of(const char *prefix, const char *item, const char *suffix) {
    char buf[160];
    int n = 0;
    for (const char *p = prefix; *p && n < 159; p++)
        buf[n++] = *p;
    for (const char *p = item; *p && n < 159; p++)
        buf[n++] = *p;
    for (const char *p = suffix; *p && n < 159; p++)
        buf[n++] = *p;
    buf[n] = 0;
    return murmur3(buf);
}
static uint32_t stock_hash(const char *item) { return hash_of("PouchData.", item, ".StockNum"); }
static uint32_t is_get_hash(const char *item) { return hash_of("IsGet.", item, ""); }

// ---- items and gear ----

int inv_owned(const inv_item_t *item) {
    int32_t v;
    int flag;
    if (item->kind == IK_ACQUIRED)
        return gd_get_bool(item->acquired, &flag) && flag;
    return field_get(item->owned, &v) && v > 0;
}

// Owned + acquired: the item screen needs both.
void inv_set_owned(const inv_item_t *item, int on) {
    if (item->kind == IK_ACQUIRED) {
        gd_set_bool(item->acquired, on);
        return;
    }
    field_set(item->owned, on ? 1 : 0);
    if (on && item->acquired)
        gd_set_bool(item->acquired, 1);
}

static int same(const char *a, const char *b) {
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

void inv_own_worn(const char *worn_item) {
    for (int i = 0; i < inv_n_gear; i++)
        if (inv_gear[i].worn_item && same(inv_gear[i].worn_item, worn_item) && !inv_owned(&inv_gear[i]))
            inv_set_owned(&inv_gear[i], 1);
}

// ---- masks and trade items: one at a time ----

int inv_one_chosen(const pouch_one_t *slot, int n) {
    for (int i = 0; i < n; i++) {
        int32_t stock;
        if (gd_get_int(stock_hash(slot[i].item), &stock) && stock > 0)
            return i + 1;
    }
    return 0;
}

void inv_one_choose(const pouch_one_t *slot, int n, int choice) {
    for (int i = 0; i < n; i++) {
        int32_t stock;
        if (i + 1 != choice && gd_get_int(stock_hash(slot[i].item), &stock) && stock > 0) {
            gd_set_int(stock_hash(slot[i].item), 0);
            gd_set_bool(is_get_hash(slot[i].item), 0);
        }
    }
    if (choice > 0 && choice <= n) {
        gd_set_int(stock_hash(slot[choice - 1].item), 1);
        gd_set_bool(is_get_hash(slot[choice - 1].item), 1);
    }
}

// ---- ammo (cheats.c): each count to its capacity field, or to a fixed number ----

void inv_fill_ammo(void) {
    for (int i = 0; i < inv_n_ammo; i++) {
        int32_t target = inv_ammo[i].fixed, value;
        if (inv_ammo[i].cap && !gd_get_int(inv_ammo[i].cap, &target))
            continue;
        if (target <= 0)
            continue;
        if (inv_ammo[i].acquired)
            gd_set_bool(inv_ammo[i].acquired, 1);
        if (gd_get_int(inv_ammo[i].count, &value) && value != target)
            gd_set_int(inv_ammo[i].count, target);
    }
}

// ---- quest items and songs ----

int inv_flag(const flag_t *flag) {
    int32_t v;
    return field_get(flag->f[0], &v) && v > 0;
}

void inv_set_flag(const flag_t *flag, int on) {
    for (int i = 0; i < 2; i++)
        if (flag->f[i].type != FT_NONE)
            field_set(flag->f[i], on ? 1 : 0);
    if (on && flag->acquired)
        gd_set_bool(flag->acquired, 1);
}

// ---- quests: an Enum field whose value is murmur3 of the stage's name ----

int quest_stage(int quest) {
    uint32_t value;
    if (quest < 0 || quest >= inv_n_quests || !gd_get_enum(inv_quests[quest].hash, &value))
        return -1;
    for (int s = 0; s < inv_quests[quest].n_stages; s++)
        if (murmur3(inv_quests[quest].stages[s]) == value)
            return s;
    return -1;
}

void quest_set_stage(int quest, int stage) {
    if (quest < 0 || quest >= inv_n_quests || stage < 0 || stage >= inv_quests[quest].n_stages)
        return;
    gd_set_enum(inv_quests[quest].hash, murmur3(inv_quests[quest].stages[stage]));
}

void quests_set_all(int complete) {
    for (int q = 0; q < inv_n_quests; q++)
        quest_set_stage(q, complete ? inv_quests[q].n_stages - 1 : 0);
}
