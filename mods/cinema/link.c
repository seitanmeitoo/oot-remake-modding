// Cinema: Link's age, what he wears, and his health kept full.
//
// Copyright (C) PracticeROM and sloperinaGZ's contributors; changes for Cinema by seitanmeitoo.
// This file is part of Cinema and is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version. See LICENSE.
//
// From Slopped Up Sloperina 0.4.0 (sloperinaGZ 0.0.4.2): age.c (age and scene reload), from link.c
// link_actor, link_health, the equipment component and owner search, from pouch.c the way gear is
// taken off (the slot's String64, then the owner's "equip all"), from sys.c the memory regions.
// Changed for Cinema: game 1.1.1 only (offsets as constants); gear put on as well as taken off,
// from a list of what each age can wear; the heap search no more than once in 10 s after a miss,
// timed on the system tick, and Core's two Link objects tried too; a scene reload when the
// equipment isn't found; the item screen's HUD refresh left out; messages in French for the panel.
#include "gamedata.h"
#include "link.h"
#include "warp.h"

// Game 1.1.1 (sloperinaGZ's z64hd_builds.h)
#define VT_LINKOBJ          0x4B25E18
#define VT_PARAM            0x4B63438
#define VT_EQUIP            0x4B49698
#define VT_EQUIP_OWNER      0x4BDFE68
#define EQUIP_ALL_FN        0xF201B0
static const struct { uint32_t global, member; } link_paths[] = {
    { 0x4E371D0, 0x480 }, { 0x4E94858, 0x2E0 }, { 0x4E935B8, 0xEF8 }, { 0x4E94860, 0x328 }, { 0x4E94368, 0xD70 },
};

// Link's health: [[link+8]+0x6A0] holds parameter objects at +0x1C10 (current) and +0x1C18 (max);
// the value is at +8.
#define ACTOR_STATE_PTR     0x8
#define STATE_PARAMS_PTR    0x6A0
#define PARAMS_HEALTH_CUR   0x1C10
#define PARAMS_HEALTH_MAX   0x1C18
#define HEALTH_VALUE        0x8

// Equipment: the component's +0x18 is its owner actor (Link); the owner holds the component at +0x470.
// shortcut: in Fritchii on 1.1.1 the heap search finds no object with VT_EQUIP at all (19 Nov 2026),
// so sloperinaGZ's 1.1.1 vtable is likely wrong and gear is applied by reloading the scene. Find the
// right vtable (the object f_82e16c80 is given) to put gear on without a reload.
#define EQUIP_COMPONENT_LINK  0x18
#define EQUIP_OWNER_COMPONENT 0x470
#define LINK_NEARBY_BYTES     0x2000    // how much of Link's actor to look through for pointers
#define SEARCH_GAP            (10 * TICK_HZ)

// Age (age.c)
#define F_IS_CHILDHOOD      0xCEA220D3u   // Sequence_IsChildhood (Bool)
#define F_OTHER_WEAPON      0xA1F7EE2Bu   // PlayerStatus.AnotherEraEquipWeapon (String64)
#define F_OTHER_SHIELD      0x2342B619u   // PlayerStatus.AnotherEraEquipShield (String64)

const char *const gear_slots[EQ_COUNT] = { "Épée", "Bouclier", "Tunique", "Bottes" };
const gear_t gear[] = {
    { "", "Aucune", EQ_SWORD, FOR_CHILD | FOR_ADULT },
    { "Weapon_Sword_Kokiri", "Kokiri", EQ_SWORD, FOR_CHILD },
    { "Weapon_Sword_Master", "Légende", EQ_SWORD, FOR_ADULT },
    { "Weapon_Sword_Giant", "Géants", EQ_SWORD, FOR_ADULT },
    { "Weapon_Sword_Biggoron", "Biggoron", EQ_SWORD, FOR_ADULT },
    { "", "Aucun", EQ_SHIELD, FOR_CHILD | FOR_ADULT },
    { "Weapon_Shield_Deku", "Mojo", EQ_SHIELD, FOR_CHILD },
    { "Weapon_Shield_Hylian", "Hylien", EQ_SHIELD, FOR_CHILD | FOR_ADULT },
    { "Weapon_Shield_Mirror", "Miroir", EQ_SHIELD, FOR_ADULT },
    { "", "Kokiri", EQ_TUNIC, FOR_CHILD | FOR_ADULT },
    { "Armor_Adult_Goron", "Goron", EQ_TUNIC, FOR_ADULT },
    { "Armor_Adult_Zora", "Zora", EQ_TUNIC, FOR_ADULT },
    { "", "Kokiri", EQ_BOOTS, FOR_CHILD | FOR_ADULT },
    { "Armor_Adult_HeavyBoot", "Plomb", EQ_BOOTS, FOR_ADULT },
    { "Armor_Adult_HoverBoot", "Airs", EQ_BOOTS, FOR_ADULT },
};
const int gear_count = sizeof(gear) / sizeof(*gear);
static const uint32_t slot_fields[EQ_COUNT] = { GD_STR_SWORD, GD_STR_SHIELD, GD_STR_TUNIC, GD_STR_BOOTS };

static int starts_with(const char *s, const char *prefix) {
    while (*prefix && *s == *prefix)
        s++, prefix++;
    return !*prefix;
}

static void copy(char *out, int size, const char *s) {
    int i = 0;
    for (; s && s[i] && i < size - 1; i++)
        out[i] = s[i];
    out[i] = 0;
}

// ---- Link and his health (link.c) ----

static uintptr_t link_actor(void) {
    for (unsigned i = 0; i < sizeof(link_paths) / sizeof(*link_paths); i++) {
        uintptr_t holder = ptr_at(MAIN(link_paths[i].global));
        uintptr_t actor = holder ? ptr_at(holder + link_paths[i].member) : 0;
        if (actor && ptr_at(actor) == MAIN(VT_LINKOBJ))
            return actor;
    }
    return 0;
}

void health_refill(void) {
    uintptr_t actor = link_actor();
    uintptr_t st = actor ? ptr_at(actor + ACTOR_STATE_PTR) : 0;
    uintptr_t params = st ? ptr_at(st + STATE_PARAMS_PTR) : 0;
    if (!params)
        return;
    uintptr_t cur = ptr_at(params + PARAMS_HEALTH_CUR), max = ptr_at(params + PARAMS_HEALTH_MAX);
    if (!cur || !max || ptr_at(cur) != MAIN(VT_PARAM) || ptr_at(max) != MAIN(VT_PARAM))
        return;
    int32_t *c = (int32_t *)(cur + HEALTH_VALUE), *m = (int32_t *)(max + HEALTH_VALUE);
    if (*c < *m)
        *c = *m;
}

// ---- heap search (link.c, sys.c) ----

typedef struct {
    uint64_t addr, size;
    uint32_t type, attr, perm, ipc_refcount, device_refcount, padding;
} meminfo_t;

static uint32_t query_memory(meminfo_t *info, uint64_t addr) {
#if defined(__aarch64__)
    register uint64_t x0 __asm__("x0") = (uint64_t)info;
    register uint64_t x1 __asm__("x1");
    register uint64_t x2 __asm__("x2") = addr;
    __asm__ volatile("svc 0x6" : "+r"(x0), "=r"(x1), "+r"(x2) : : "memory", "x3", "x4", "x5", "x6", "x7", "x8",
                     "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18");
    return (uint32_t)x0;
#else
    (void)info, (void)addr;
    return 1;
#endif
}

// The first 8-byte word in the game's ordinary read-write heap equal to `value` that ok() accepts.
static uintptr_t heap_find(uint64_t value, int (*ok)(uintptr_t addr, uintptr_t ctx), uintptr_t ctx) {
    meminfo_t info;
    uint64_t addr = 0;
    for (int guard = 0; guard < 4096; guard++) {
        if (query_memory(&info, addr) != 0 || info.size == 0)
            break;
        if ((info.type & 0xFF) == 0x05 && (info.perm & 3) == 3 && !(info.attr & 1)) {   // normal, rw, not locked
            const volatile uint64_t *w = (const uint64_t *)info.addr;
            for (uint64_t i = 0; i < info.size / 8; i++)
                if (w[i] == value && ok((uintptr_t)&w[i], ctx))
                    return (uintptr_t)&w[i];
        }
        uint64_t next = info.addr + info.size;
        if (next <= addr)
            break;
        addr = next;
    }
    return 0;
}

// ---- equipment (link.c, pouch.c) ----

static uintptr_t cached_component, cached_owner;
static uint64_t component_missed_at, owner_missed_at;

// Who the component's +0x18 may point to: sloperinaGZ's Link object, and the two objects Core
// hands player_move (his player object and physics actor), in case the remake's 1.1.1 differs.
static uintptr_t seen_object, seen_actor;
void link_seen(void *object, void *actor) { seen_object = (uintptr_t)object, seen_actor = (uintptr_t)actor; }

static int is_link(uintptr_t v, uintptr_t link) { return v && (v == link || v == seen_object || v == seen_actor); }
static int is_link_component(uintptr_t component, uintptr_t link) {
    return ptr_at(component) == MAIN(VT_EQUIP) && is_link(ptr_at(component + EQUIP_COMPONENT_LINK), link);
}
// Counts the vtable's hits, and logs the first few owners they point to, for finding out why none fits.
static int vt_hits;
static int component_hit(uintptr_t addr, uintptr_t link) {
    uintptr_t owner = ptr_at(addr + EQUIP_COMPONENT_LINK);
    if (++vt_hits <= 3)
        KAE_LOG("cinema: equipment vtable at %lx, +0x18 = %lx (Link %lx, object %lx, actor %lx)",
                (unsigned long)addr, (unsigned long)owner, (unsigned long)link, (unsigned long)seen_object,
                (unsigned long)seen_actor);
    return is_link(owner, link);
}
static int is_owner_of(uintptr_t owner, uintptr_t component) {
    return ptr_at(owner) == MAIN(VT_EQUIP_OWNER) && ptr_at(owner + EQUIP_OWNER_COMPONENT) == component;
}
static int owner_hit(uintptr_t addr, uintptr_t component) {
    (void)component;
    return ptr_at(addr - EQUIP_OWNER_COMPONENT) == MAIN(VT_EQUIP_OWNER);
}
static int recent_miss(uint64_t at) { return at && now_tick() - at < SEARCH_GAP; }

// A pointer near Link (his actor's first bytes) that pick() accepts.
static uintptr_t near_link(uintptr_t link, int (*pick)(uintptr_t candidate, uintptr_t ctx), uintptr_t ctx) {
    if (!kae_readable((void *)link, LINK_NEARBY_BYTES))
        return 0;
    for (uintptr_t off = 0; off < LINK_NEARBY_BYTES; off += 8) {
        uintptr_t c = *(uintptr_t *)(link + off);
        if (c && !(c & 7) && pick(c, ctx))
            return c;
    }
    return 0;
}

static uintptr_t equip_component(uintptr_t link) {
    if (cached_component && is_link_component(cached_component, link))
        return cached_component;
    cached_component = near_link(link, is_link_component, link);
    if (!cached_component && seen_object)
        cached_component = near_link(seen_object, is_link_component, link);
    if (cached_component || recent_miss(component_missed_at))
        return cached_component;
    vt_hits = 0;
    cached_component = heap_find(MAIN(VT_EQUIP), component_hit, link);
    component_missed_at = cached_component ? 0 : now_tick();
    KAE_LOG("cinema: equipment component %s by heap search (%d with its vtable)", cached_component ? "found" : "not found", vt_hits);
    return cached_component;
}

static uintptr_t equip_owner(void) {
    uintptr_t link = link_actor();
    uintptr_t component = link ? equip_component(link) : 0;
    if (!component)
        return 0;
    if (cached_owner && is_owner_of(cached_owner, component))
        return cached_owner;
    cached_owner = near_link(link, is_owner_of, component);
    if (!cached_owner && seen_object)
        cached_owner = near_link(seen_object, is_owner_of, component);
    if (cached_owner || recent_miss(owner_missed_at))
        return cached_owner;
    uintptr_t hit = heap_find(component, owner_hit, component);
    cached_owner = hit ? hit - EQUIP_OWNER_COMPONENT : 0;
    owner_missed_at = cached_owner ? 0 : now_tick();
    KAE_LOG("cinema: equipment owner %s by heap search", cached_owner ? "found" : "not found");
    return cached_owner;
}

int gear_worn(int index) {
    const char *worn = gd_get_str(slot_fields[gear[index].slot]);
    return worn && str_equal(worn, gear[index].game);
}

typedef void (*equip_all_t)(uintptr_t owner, int32_t zero);
const char gear_reload_needed[] = "Rechargement du lieu pour l'appliquer...";

const char *gear_wear(int index) {
    if (index < 0 || index >= gear_count)
        return 0;
    if (!gd_set_str(slot_fields[gear[index].slot], gear[index].game))
        return "Impossible ici : données du jeu introuvables.";
    uintptr_t owner = equip_owner();
    if (!owner)
        return gear_reload_needed;   // cinema.c reloads the scene: the game puts the gear on as it loads
    MAIN_FN(equip_all_t, EQUIP_ALL_FN)(owner, 0);   // as pouch.c's unequip: every slot put on again
    KAE_LOG("cinema: wearing %s", gear[index].game[0] ? gear[index].game : "nothing / default");
    return "Équipé.";
}

// ---- age (age.c) ----

int age_current(void) {
    int child;
    if (!gd_get_bool(F_IS_CHILDHOOD, &child))
        return -1;
    return child ? AGE_CHILD : AGE_ADULT;
}

static const place_t *find_place(const char *name) {
    for (int i = 0; i < places_count; i++)
        if (str_equal(places[i].name, name))
            return &places[i];
    return 0;
}

// The version of scene `name` for `age`, into out: X_Child <-> X_Adult (X_Adult_Alternative -> X_Child).
// 0 if the scene has an age and its other version isn't known.
static int age_version(const char *name, int age, char *out, int size) {
    const char *suffix = 0;
    for (const char *p = name; *p && !suffix; p++)
        if (starts_with(p, "_Child") || starts_with(p, "_Adult"))
            suffix = p;
    if (!suffix || (age == AGE_CHILD) == starts_with(suffix, "_Child")) {   // shared, or already this age
        copy(out, size, name);
        return 1;
    }
    int n = (int)(suffix - name);
    if (n > size - 7)
        return 0;
    copy(out, n + 1, name);
    copy(out + n, size - n, age == AGE_CHILD ? "_Child" : "_Adult");
    return find_place(out) != 0;
}

// Where to arrive: the start point Link last arrived at, else an entrance an exit leads to, else the first.
static const spawn_t *reload_spawn(const place_t *place, const char *last_start) {
    const spawn_t *s = &spawns[place->first];
    if (last_start && last_start[0])
        for (int i = 0; i < place->n; i++)
            if (str_equal(s[i].pos, last_start))
                return &s[i];
    for (int i = 0; i < place->n; i++)
        if (s[i].starter)
            return &s[i];
    return place->n ? &s[0] : 0;
}

static void swap_strings(uint32_t a, uint32_t b) {
    char ca[GD_STR_LEN], cb[GD_STR_LEN];
    const char *va = gd_get_str(a), *vb = gd_get_str(b);
    if (!va || !vb)
        return;
    copy(ca, sizeof(ca), va);
    copy(cb, sizeof(cb), vb);
    gd_set_str(a, cb);
    gd_set_str(b, ca);
}

static void take_off_adult_only(uint32_t slot) {
    const char *item = gd_get_str(slot);
    if (item && starts_with(item, "Armor_Adult_"))
        gd_set_str(slot, "");
}

// The age before a reload changed it, with the adult-only clothes that came off; -1 once the reload
// has happened. The data is changed before the scene loads, so a reload that never happens puts it back.
static int undo_age = -1;
static char undo_tunic[GD_STR_LEN], undo_boots[GD_STR_LEN];

static void apply_age(int age) {
    swap_strings(GD_STR_SWORD, F_OTHER_WEAPON);   // the other age's sword and shield (the Temple of Time does it)
    swap_strings(GD_STR_SHIELD, F_OTHER_SHIELD);
    if (age == AGE_CHILD) {
        take_off_adult_only(GD_STR_TUNIC);
        take_off_adult_only(GD_STR_BOOTS);
    }
    gd_set_bool(F_IS_CHILDHOOD, age == AGE_CHILD);
}

// A scene the warp list doesn't know (houses, shops...). Static: the warp keeps the pointers.
static place_t unlisted_place;
static spawn_t unlisted_spawn;
static char unlisted_name[GD_STR_LEN], unlisted_pos[GD_STR_LEN];

const char *age_reload(int age) {
    const char *current = gd_get_str(GD_STR_MAP), *last_start = gd_get_str(GD_STR_SPAWN);
    int now = age_current();
    if (!current || !current[0] || now < 0)
        return "Impossible ici : données du jeu introuvables.";
    char name[GD_STR_LEN];
    if (!age_version(current, age, name, sizeof(name)))
        return age == AGE_CHILD ? "Ce lieu n'a pas de version enfant." : "Ce lieu n'a pas de version adulte.";
    KAE_LOG("cinema: reload %s -> %s, last start point \"%s\", age %d -> %d", current, name,
            last_start ? last_start : "?", now, age);
    const place_t *place = find_place(name);
    const spawn_t *spawn = place ? reload_spawn(place, last_start) : 0;
    if (!place && last_start && last_start[0]) {
        // its last start point, arriving as from a warp song (no starter actor needed)
        copy(unlisted_name, sizeof(unlisted_name), name);
        copy(unlisted_pos, sizeof(unlisted_pos), last_start);
        unlisted_place = (place_t){ unlisted_name, unlisted_name, PC_OTHER, 0, 0, 0 };
        unlisted_spawn = (spawn_t){ unlisted_pos, unlisted_pos, "Dm_SY_BancChangeArrive", 0 };
        place = &unlisted_place;
        spawn = &unlisted_spawn;
    }
    if (!place || !spawn)
        return "Impossible : aucun point d'arrivée connu ici.";
    if (age != now) {
        if (undo_age < 0) {
            undo_age = now;
            copy(undo_tunic, sizeof(undo_tunic), gd_get_str(GD_STR_TUNIC));
            copy(undo_boots, sizeof(undo_boots), gd_get_str(GD_STR_BOOTS));
        } else if (age == undo_age) {
            undo_age = -1;   // a second reload, back to the age the scene is loaded at
        }
        apply_age(age);
    }
    warp_go(place, spawn);
    return 0;
}

void age_frame(void) {
    if (undo_age < 0 || warp_pending())
        return;
    if (warp_abandoned() && age_current() >= 0 && age_current() != undo_age) {
        apply_age(undo_age);
        gd_set_str(GD_STR_TUNIC, undo_tunic);
        gd_set_str(GD_STR_BOOTS, undo_boots);
        KAE_LOG("cinema: age change undone: the scene didn't reload");
    }
    undo_age = -1;
}

const char *link_scene(void) { return gd_get_str(GD_STR_MAP); }
