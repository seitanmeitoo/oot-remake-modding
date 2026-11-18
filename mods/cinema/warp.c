// Cinema: warps to any scene and arrival point.
//
// Copyright (C) PracticeROM and sloperinaGZ's contributors; changes for Cinema by seitanmeitoo.
// This file is part of Cinema and is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version. See LICENSE.
//
// From Slopped Up Sloperina 0.4.0 (mods/slopped-up-sloperina), the KaeGae Core port of sloperinaGZ
// 0.0.4.2: its warp.c and the 1.1.1 offsets of z64hd_builds.h. Changed for Cinema: game 1.1.1
// only (its offsets as constants, GameData through gamedata.c); timeouts on the system tick
// instead of the game's frame counter; a late reservation of our own map counted as done; status
// lines in French for Cinema's panel instead of gz notices; the arrival cutscene option left out
// (always the standard arrival); warp_pending / warp_abandoned for the age change (link.c).
//
// Instant (warp_go tries this first): request the same event an exit requests, with the same
// parameters. Exits are FieldAndDungeonEntranceEventRequestActor; they request "Dm_SY_BancChangeGo"
// with NextBanc / NextStartPos / NextStartPosAir / BootEventName / IsCheckAir, and that flow's
// GameSystemActor.EventReserveBancChange writes the Sequence_Reserved* GameData and sets
// Sequence_ControlInEvent = ChangeBancOnEvent, which the Event sequence turns into the map change.
// So the warp gets the game's own fade, sound and arrival.
//
// Fallback, if the request is refused: clear Sequence_ReservedNextBanc, wait for the game to fill
// it (Link entered an exit), then swap in our destination. Exits that lead into a dungeon are left
// alone: swapping the map under their door transition left the screen black for good.
#include "gamedata.h"
#include "warp.h"

// Game 1.1.1 (sloperinaGZ's z64hd_builds.h)
#define EVENT_MGR_PTR       0x4C7E090
#define EVENT_REQ_CTOR      0xD79F10
#define EVENT_REQ_SUBMIT    0xD7A820
#define EVENT_REQ_SUB_DTOR  0x20B6F0
#define EVENT_REQ_SUB_VT    0x4974B10
// Layouts shared by the game versions (z64hd.h)
#define EVENT_REQ_SIZE      0x1B20
#define EVENT_REQ_PARAMS    0x08      // param entry array
#define EVENT_REQ_NPARAMS   0x10      // u32 count; capacity (u32) at +0
#define EVENT_REQ_SUB_A     0x1950
#define EVENT_REQ_SUB_B     0x1968
#define EVENT_REQ_NAME      0x1A90    // char *event name; capacity (u32) at +0x1A98
#define EVENT_PARAM_SIZE    0xC8      // entry: u32 type, char *name (+8, cap u32 +0x10), value +0x98
#define EVENT_PARAM_STRING  1
#define EVENT_PARAM_BOOL    4
#define STRING_CAPACITY     0x8       // the game's strings: a char * buffer, then its u32 capacity
#define PARAM_TYPE          0x0
#define PARAM_NAME          0x8
#define PARAM_VALUE         0x98
#define VTABLE_FIRST_ENTRY  0x10      // an object's vtable pointer skips the offset-to-top and type info

#define STANDARD_ARRIVAL    "Dm_SY_BancChangeArrive"
// Spawns no exit leads to (warp-song pads, blue warps, boss rooms...) have no event starter for the
// standard arrival, which crashed. The game enters the warp-song pads with no starter and this
// arrival; it needs no starter.
#define NO_STARTER_ARRIVAL  "OcarinaWarp_Arrive"

// Timeouts on the system tick (19.2 MHz): Core's frame event runs several times per drawn frame
// (about 120 a second in Fritchii), so counting frames cut the 3 s wait to 0.75 s.
#define WARP_TIMEOUT        (300 * TICK_HZ)    // how long an armed fallback waits for an exit
#define REQUEST_TIMEOUT     (3 * TICK_HZ)      // the event writes the reserved map within this, or we fall back

static const place_t *target_place;
static const spawn_t *target_spawn;
static enum { W_NONE, W_ARMED, W_DONE, W_SKIPPED, W_TIMED_OUT, W_REQUESTED, W_NO_DATA } state;
static uint64_t deadline;

// ---- warps (warp.c) ----

static const char *arrival_event(const spawn_t *spawn) { return spawn->starter ? STANDARD_ARRIVAL : NO_STARTER_ARRIVAL; }

// After the game has reserved our map: what the event's own reservation got wrong for spawns
// without a starter (Dm_SY_BancChangeGo always names the spawn as the starter).
static void fix_reservation(const spawn_t *spawn) {
    if (spawn->starter)
        return;
    gd_set_str(GD_STR_STARTER, "");
    gd_set_str(GD_STR_EVENT, NO_STARTER_ARRIVAL);
}

static int is_dungeon(const char *name) {
    for (int i = 0; i < places_count; i++)
        if (places[i].dungeon && str_equal(places[i].name, name))
            return 1;
    return 0;
}

typedef void (*req_ctor_t)(void *req);
typedef _Bool (*req_submit_t)(void *mgr, void *req);
typedef void (*req_dtor_t)(void *sub);

static _Alignas(16) uint8_t request[EVENT_REQ_SIZE];   // the game's thread only
static char pos_air[GD_STR_LEN + 8];

static void copy_str(char *dst, uint32_t capacity, const char *src) {
    uint32_t i = 0;
    for (; capacity && i < capacity - 1 && src[i]; i++)
        dst[i] = src[i];
    if (capacity)
        dst[i] = 0;
}

static void set_game_string(uint8_t *string_field, const char *src) {
    copy_str(*(char **)string_field, *(uint32_t *)(string_field + STRING_CAPACITY), src);
}

static int add_param(const char *name, uint32_t type, const void *value) {
    uint32_t *count = (uint32_t *)(request + EVENT_REQ_NPARAMS);
    uint32_t capacity = *(uint32_t *)request;
    uint8_t *array = *(uint8_t **)(request + EVENT_REQ_PARAMS);
    if (!array || *count >= capacity)
        return 0;
    uint8_t *entry = array + (size_t)*count * EVENT_PARAM_SIZE;
    set_game_string(entry + PARAM_NAME, name);
    *(uint32_t *)(entry + PARAM_TYPE) = type;
    if (type == EVENT_PARAM_BOOL)
        *(uint8_t *)(entry + PARAM_VALUE) = *(const _Bool *)value;
    else
        *(const void **)(entry + PARAM_VALUE) = value;
    ++*count;
    return 1;
}

// Destroy the request as the game's node does: base vtable into both sub-objects, then their destructor.
static void destroy_request(void) {
    uintptr_t base_vtable = MAIN(EVENT_REQ_SUB_VT) + VTABLE_FIRST_ENTRY;
    *(uintptr_t *)(request + EVENT_REQ_SUB_B) = base_vtable;
    MAIN_FN(req_dtor_t, EVENT_REQ_SUB_DTOR)(request + EVENT_REQ_SUB_B);
    *(uintptr_t *)(request + EVENT_REQ_SUB_A) = base_vtable;
    MAIN_FN(req_dtor_t, EVENT_REQ_SUB_DTOR)(request + EVENT_REQ_SUB_A);
}

static int request_warp_event(const place_t *place, const spawn_t *spawn) {
    uintptr_t holder = ptr_at(MAIN(EVENT_MGR_PTR));
    void *manager = (void *)(holder ? ptr_at(holder) : 0);
    if (!manager)
        return 0;
    for (uint32_t i = 0; i < sizeof(request); i++)
        request[i] = 0;
    MAIN_FN(req_ctor_t, EVENT_REQ_CTOR)(request);
    set_game_string(request + EVENT_REQ_NAME, "Dm_SY_BancChangeGo");
    int n = 0;
    for (const char *s = spawn->pos; *s && n < GD_STR_LEN; s++)
        pos_air[n++] = *s;
    for (const char *s = "_Air"; *s; s++)
        pos_air[n++] = *s;
    pos_air[n] = 0;
    static const _Bool no = 0;
    int ok = add_param("NextBanc", EVENT_PARAM_STRING, place->name) &&
             add_param("NextStartPos", EVENT_PARAM_STRING, spawn->pos) &&
             add_param("NextStartPosAir", EVENT_PARAM_STRING, pos_air) &&
             add_param("BootEventName", EVENT_PARAM_STRING, arrival_event(spawn)) &&
             add_param("IsCheckAir", EVENT_PARAM_BOOL, &no);
    if (ok)
        ok = MAIN_FN(req_submit_t, EVENT_REQ_SUBMIT)(manager, request);
    destroy_request();
    return ok;
}

static void arm_exit_swap(void) {
    if (!gd_set_str(GD_STR_NEXT_MAP, "")) {
        state = W_NO_DATA;
        return;
    }
    state = W_ARMED;
    deadline = now_tick() + WARP_TIMEOUT;
    KAE_LOG("cinema: warp armed: %s / %s", target_place->name, target_spawn->pos);
}

void warp_go(const place_t *place, const spawn_t *spawn) {
    target_place = place;
    target_spawn = spawn;
    if (request_warp_event(place, spawn)) {
        state = W_REQUESTED;
        deadline = now_tick() + REQUEST_TIMEOUT;
        KAE_LOG("cinema: warp: requested Dm_SY_BancChangeGo to %s / %s", place->name, spawn->pos);
        return;
    }
    KAE_LOG("cinema: warp: the game refused the event request; falling back to the next exit");
    arm_exit_swap();
}

void warp_frame(void) {
    if (state == W_REQUESTED) {
        // Instant warp: wait for the event to write our map into Sequence_ReservedNextBanc.
        const char *next = gd_get_str(GD_STR_NEXT_MAP);
        if (next && str_equal(next, target_place->name)) {
            fix_reservation(target_spawn);
            state = W_DONE;
            KAE_LOG("cinema: warp: the game reserved %s", next);
        } else if (now_tick() > deadline) {
            KAE_LOG("cinema: warp: the event didn't start; falling back to the next exit");
            arm_exit_swap();
        }
    } else if (state == W_ARMED) {
        // Fallback: once Link takes an exit (the game fills the next map in), put our destination there.
        if (now_tick() > deadline) {
            state = W_TIMED_OUT;
            return;
        }
        const char *next = gd_get_str(GD_STR_NEXT_MAP);
        if (!next || !next[0])
            return;
        if (str_equal(next, target_place->name)) {   // our own request, late: it went through after all
            fix_reservation(target_spawn);
            state = W_DONE;
            KAE_LOG("cinema: warp: the game reserved %s (late)", next);
            return;
        }
        if (is_dungeon(next)) {
            state = W_SKIPPED;
            KAE_LOG("cinema: warp skipped: the exit leads into %s", next);
            return;
        }
        gd_set_str(GD_STR_NEXT_MAP, target_place->name);
        gd_set_str(GD_STR_SPAWN, target_spawn->pos);
        gd_set_str(GD_STR_STARTER, target_spawn->starter ? target_spawn->pos : "");
        gd_set_str(GD_STR_EVENT, arrival_event(target_spawn));
        state = W_DONE;
        KAE_LOG("cinema: warp: %s replaced by %s / %s", next, target_place->name, target_spawn->pos);
    }
}

const char *warp_status(void) {
    switch (state) {
    case W_REQUESTED: return "Téléportation en cours...";
    case W_ARMED:     return "Prends une sortie (porte, chemin) pour arriver.";
    case W_DONE:      return "Téléportation faite.";
    case W_SKIPPED:   return "Annulée : cette sortie mène à un donjon.";
    case W_TIMED_OUT: return "Annulée : aucune sortie prise à temps.";
    case W_NO_DATA:   return "Impossible ici : données du jeu introuvables.";
    default:          return "";
    }
}

int warp_pending(void) { return state == W_ARMED || state == W_REQUESTED; }
int warp_abandoned(void) { return state == W_TIMED_OUT || state == W_SKIPPED || state == W_NO_DATA; }
