// Cinema: GameData String64 and Bool fields.
//
// Copyright (C) PracticeROM and sloperinaGZ's contributors; changes for Cinema by seitanmeitoo.
// This file is part of Cinema and is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version. See LICENSE.
//
// From Slopped Up Sloperina 0.4.0's gamedata.c (sloperinaGZ 0.0.4.2): the type entries, the hash
// lookup, String64 and Bool. Changed for Cinema: game 1.1.1 only (its offsets as constants, memory
// checked with kae_readable instead of sys.c); Int, Float and Enum left out.
#include "gamedata.h"

#define GD_MGR_PTR      0x4C7E180     // 1.1.1
#define VT_BOOL_TYPE    0x4C1F140     // 1.1.1: the Bool type entry's vtable
#define GD_STRING_TYPE  0x608         // manager+: the String64 type entry
#define GD_TYPES_SCAN   0x800         // other type entries are found by vtable within manager+0..this
#define STR_STRIDE      0x108
#define STR_VALUE       0x68          // char *value, followed by u32 capacity
#define BOOL_STRIDE     0x18
#define BOOL_VALUE      0x16          // the value, then its copy

uintptr_t ptr_at(uintptr_t addr) {
    if (!addr || (addr & 7) || !kae_readable((void *)addr, 8))
        return 0;
    return *(volatile uintptr_t *)addr;
}

int str_equal(const char *a, const char *b) {
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

uint64_t now_tick(void) {
    uint64_t now = 0;
#if defined(__aarch64__)
    register uint64_t x0 __asm__("x0");
    __asm__ volatile("svc #0x1e" : "=r"(x0) : : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "memory");
    now = x0;
#endif
    return now;
}

typedef struct {
    uintptr_t vtable;
    uint32_t  count, _0c;
    uint8_t  *objects;
    uint32_t *table;      // (hash, index) pairs, open addressing, hash 0 = empty
    uint32_t  _20, table_size;
} gd_type_t;

static uint8_t *manager(void) {
    uintptr_t holder = ptr_at(MAIN(GD_MGR_PTR));
    return (uint8_t *)(holder ? ptr_at(holder) : 0);
}

static int type_plausible(const gd_type_t *t) {
    if (!kae_readable((void *)t, sizeof(*t)))
        return 0;
    if (t->count == 0 || t->count >= 0x10000 || t->table_size == 0 || t->table_size >= 0x100000)
        return 0;
    return kae_readable(t->table, (size_t)t->table_size * 8) && kae_readable(t->objects, 8);
}

// The object index of `hash` in a type entry, probing like the game's getters; -1 if absent.
static int64_t lookup(const gd_type_t *t, uint32_t hash) {
    if (!hash)
        return -1;
    uint32_t start = hash % t->table_size, slot = start;
    do {
        const uint32_t *pair = t->table + 2 * slot;
        if (pair[0] == hash)
            return pair[1] < t->count ? pair[1] : -1;
        if (pair[0] == 0)
            return -1;
        slot = slot + 1 >= t->table_size ? 0 : slot + 1;
    } while (slot != start);
    return -1;
}

static char *string_value(uint32_t hash) {
    uint8_t *mgr = manager();
    if (!mgr)
        return 0;
    const gd_type_t *t = (const gd_type_t *)(mgr + GD_STRING_TYPE);
    if (!type_plausible(t))
        return 0;
    int64_t index = lookup(t, hash);
    if (index < 0)
        return 0;
    uintptr_t field = (uintptr_t)t->objects + (size_t)index * STR_STRIDE + STR_VALUE;
    uintptr_t text = ptr_at(field);
    if (!text || !kae_readable((void *)(field + 8), 4) || *(uint32_t *)(field + 8) < GD_STR_LEN ||
        !kae_readable((void *)text, GD_STR_LEN))
        return 0;
    return (char *)text;
}

const char *gd_get_str(uint32_t hash) { return string_value(hash); }

int gd_set_str(uint32_t hash, const char *s) {
    char *d = string_value(hash);
    if (!d)
        return 0;
    int i = 0;
    for (; i < GD_STR_LEN - 1 && s[i]; i++)
        d[i] = s[i];
    for (; i < GD_STR_LEN; i++)
        d[i] = 0;
    return 1;
}

// The Bool type entry, found by its vtable among the manager's first bytes (the offset is cached).
static uint32_t bool_entry_off;

static uint8_t *bool_value(uint32_t hash) {
    uint8_t *mgr = manager();
    if (!mgr || !kae_readable(mgr, GD_TYPES_SCAN + sizeof(gd_type_t)))
        return 0;
    uintptr_t vtable = MAIN(VT_BOOL_TYPE);
    gd_type_t *t = bool_entry_off ? (gd_type_t *)(mgr + bool_entry_off) : 0;
    if (!t || t->vtable != vtable || !type_plausible(t)) {
        t = 0;
        bool_entry_off = 0;
        for (uint32_t off = 0; off < GD_TYPES_SCAN; off += 8) {
            gd_type_t *c = (gd_type_t *)(mgr + off);
            if (c->vtable == vtable && type_plausible(c)) {
                t = c;
                bool_entry_off = off;
                break;
            }
        }
    }
    if (!t)
        return 0;
    int64_t index = lookup(t, hash);
    return index < 0 ? 0 : t->objects + (size_t)index * BOOL_STRIDE + BOOL_VALUE;
}

int gd_get_bool(uint32_t hash, int *out) {
    uint8_t *v = bool_value(hash);
    if (v)
        *out = v[0] != 0;
    return v != 0;
}

int gd_set_bool(uint32_t hash, int value) {
    uint8_t *v = bool_value(hash);
    if (v)
        v[0] = v[1] = (uint8_t)(value != 0);
    return v != 0;
}
