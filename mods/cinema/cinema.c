// Cinema 0.4: tools for shots on KaeGae Core. A free camera with speeds from very slow to fast,
// the world frozen or in slow motion, warps to any scene (warp.c), Link's age, gear, health and
// position (link.c), and Link carried along by the camera, all
// usable from a panel clicked with the mouse (panel.c); camera and time also from Cinema's page in
// the Mods menu (- and +) and from shortcuts.
//
// Copyright (C) seitanmeitoo. Cinema is free software under the GNU General Public License,
// version 3 or later (LICENSE), because its warps come from sloperinaGZ, which is.
//
// Time: the game's time manager multiplies every frame delta by a global time scale. Writing it
// slows or stops the whole world (Link included) while the game keeps drawing, and the free camera
// runs on the system tick, not on game time, so it still flies around a frozen scene. Found by the
// Lichii photo mode (zortos293/Lichii PR #618): **(main + 0x4c7e890) + 0x1c0, a float. Frozen is
// 1e-4, not 0: the game also takes the reciprocal of the scaled step.
//
// Camera: FPV Drone's free fly mode (mods/fpv-drone/core/fpv_drone.c, MIT, KaboWabo/Kabosune),
// without the drone modes, with a list of speeds and its own shortcuts. Events used:
//   input  one step per frame from player 1 / the handheld; while flying the game sees centred
//          sticks and no buttons but + and -, so Link waits
//   view   the gameplay view is replaced by the camera (priority 20: after FPV Drone and First
//          Person, so Cinema wins when they are on too)
//   frame  frames since the last gameplay view (menus, loading: the camera stands by), and time
//
// shortcut: raw offsets of game 1.1.1 (kae_game_base); the mod switches itself off on any other
// build. Move them into the symbol packs (a "TimeMgrPtr" global) once 1.0.0's address is known.
// shortcut: Core gives a mod switched off in the Mods menu no more events, so the scale it set
// stays; unfreeze before switching Cinema off. A stop hook in Core would fix it.
// shortcut: the Mods menu's list drops accented letters (kae_menu.c counts ASCII only), so the page
// labels have none; kae_draw_text has Latin-1 and the panel uses it. Accent the labels once the
// menu draws them.
#include "cinema.h"
#include "kmath.h"
#include "warp.h"
#include "link.h"
#include "inventory.h"

#define BUILD_111       "E0F3CCFAF50065AE4EA01EB5AEB13B1A45DA3348"
#define TIME_MGR_SLOT   0x4c7e890
#define TIME_SCALE_OFF  0x1c0
#define FROZEN          1e-4f

#define DT_MAX          0.05f
#define MAX_LOOK        0.97f
#define STANDBY_FRAMES  4          // frames without a gameplay view (menus, loading): stand by
#define TICK_HZ         19200000.0f
#define TOAST_S         2.0f       // how long the speed stays on screen after a change

const char *const SPEEDS[] = {
    "0,1 m/s", "0,25 m/s", "0,5 m/s", "1 m/s", "2 m/s", "4 m/s", "8 m/s", "16 m/s", "30 m/s", "60 m/s",
};
static const float SPEED_MS[] = { 0.1f, 0.25f, 0.5f, 1, 2, 4, 8, 16, 30, 60 };
#define N_SPEEDS ((int)(sizeof(SPEED_MS) / sizeof(*SPEED_MS)))

#define ACTION(label, help, fn) { KAE_ACTION, label, help, 0, 0, 0, 0, 0, 0, 0, 0, fn, 0 }
#define BIND(label, help, buttons) { KAE_BINDING, label, help, 0, 0, 0, 0, 0, 0, buttons, buttons, 0, 0 }
#define NUM(label, help, def, lo, hi, step) { KAE_NUMBER, label, help, def, def, lo, hi, step, 0, 0, 0, 0, 0 }
#define TOGGLE(label, help, def) { KAE_TOGGLE, label, help, def, def, 0, 1, 1, 0, 0, 0, 0, 0 }

static void act_panel(KaeItem *item, void *user);
static void act_camera(KaeItem *item, void *user);
static void act_freeze(KaeItem *item, void *user);
static void act_slow(KaeItem *item, void *user);

KaeItem settings[COUNT] = {
    // Actions first: what the page is for. They hold no value, so nothing is left on at the next boot.
    [A_PANEL]   = ACTION("Panneau souris: ouvrir / fermer", "Des boutons a cliquer avec la souris du PC, sur le jeu.", act_panel),
    [A_CAM]     = ACTION("Camera libre: activer / couper", "Part de la camera du jeu. Link attend sur place.", act_camera),
    [A_FREEZE]  = ACTION("Temps: figer / relancer le monde", "Tout s'arrete sauf la camera libre.", act_freeze),
    [A_SLOW]    = ACTION("Temps: ralenti on / off", "Le monde entier ralentit, Link compris.", act_slow),
    [CAM_SPEED] = { KAE_CHOICE, "Camera: vitesse", "Au stick pousse a fond. Lent pour les plans de coupe.", 4, 4, 0, N_SPEEDS - 1, 0, SPEEDS, 0, 0, 0, 0 },
    [CAM_TURN]  = NUM("Camera: vitesse de rotation (deg/s)", "Au stick pousse a fond.", 60, 5, 360, 5),
    [SLOW_PCT]  = NUM("Temps: vitesse du ralenti (%)", "100 = vitesse normale.", 25, 5, 95, 5),
    [SHOW]      = TOGGLE("Afficher l'indicateur", "PAUSE, RALENTI et la vitesse de la camera, en haut a gauche.", 1),
    [K_PANEL]   = BIND("Raccourci: panneau souris", "Donnez-lui une touche du clavier (Tab par exemple).", KAE_L | KAE_R),
    [K_CAM]     = BIND("Raccourci: camera libre", "Aucun par defaut. Stick gauche: avancer, stick droit: regarder.", 0),
    [K_RESET]   = BIND("Raccourci: replacer la camera", "La ramene a la camera du jeu.", KAE_A),
    [K_UP]      = BIND("Raccourci: camera monte", 0, KAE_ZR),
    [K_DOWN]    = BIND("Raccourci: camera descend", 0, KAE_ZL),
    [K_FASTER]  = BIND("Raccourci: camera plus vite", "Pendant le vol.", KAE_RIGHT),
    [K_SLOWER]  = BIND("Raccourci: camera moins vite", "Pendant le vol.", KAE_LEFT),
    [K_FREEZE]  = BIND("Raccourci: figer / relancer le monde", "Aucun par defaut.", 0),
    [K_SLOW]    = BIND("Raccourci: ralenti on / off", "Aucun par defaut.", 0),
};

static volatile unsigned requests;   // REQ_* from the panel, done in frame
static volatile int warp_place, warp_spawn, health_on, age_wanted, gear_wanted, carrying;
static volatile int inv_op, inv_a, inv_b;
static const char *link_status = "";

// A saved position: where Link stood, in which scene. A load puts him back over a few movement
// updates (the game may move him back once).
#define LOAD_UPDATES 3
static struct { int saved, loading, save_wanted; float at[3]; char scene[64]; } pos;

static struct {
    volatile int freeze, slow;
    int active;      // our scale is written into the game
    float saved;     // the game's scale before we changed it
} c;

static struct {
    volatile int on, reset;    // reset: start from the game camera on the next step
    int stepped;
    volatile int since_view;   // frames since the last gameplay view
    uint64_t tick, toast;      // last step; when the speed was last shown
    int have_game;             // eye / gback hold the game camera
    float eye[3], gback[3];    // the view as it was before the camera took over
    float mat[12];             // the camera's view matrix
    float p[3], r[9];          // position; right / up / back
    float carry_at[3];         // where a carried Link goes: behind the camera, at its height
} d;

static uint64_t now_tick(void) {
    uint64_t now = 0;
#if defined(__aarch64__)
    register uint64_t x0 __asm__("x0");
    __asm__ volatile("svc #0x1e" : "=r"(x0) : : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "memory");
    now = x0;   // svcGetSystemTick: 19.2 MHz
#endif
    return now;
}

static int same_build(const char *a, const char *b) {
    for (; *a && *b; a++, b++) {
        char x = *a, y = *b;
        if (x >= 'a' && x <= 'z') x -= 32;
        if (y >= 'a' && y <= 'z') y -= 32;
        if (x != y)
            return 0;
    }
    return *a == *b;
}

// ---- time ----

static void set_freeze(int on) { c.freeze = on; KAE_LOG("cinema: world %s", on ? "frozen" : "running"); }
static void set_slow(int on) { c.slow = on; KAE_LOG("cinema: slow motion %s", on ? "on" : "off"); }

// The time scale, or null if the manager isn't there yet (boot, loading). Read every frame: the
// manager may be rebuilt.
static float *time_scale(void) {
    uintptr_t slot = kae_game_base() + TIME_MGR_SLOT;
    if (!kae_readable((void *)slot, 8))
        return 0;
    uintptr_t p = *(uintptr_t *)slot;
    if (!p || !kae_readable((void *)p, 8))
        return 0;
    uintptr_t mgr = *(uintptr_t *)p;
    if (!mgr || !kae_readable((void *)(mgr + TIME_SCALE_OFF), 4))
        return 0;
    float *s = (float *)(mgr + TIME_SCALE_OFF);
    if (!(*s > 0.0f && *s <= 16.0f))   // not a time scale: wrong pointer, leave it alone
        return 0;
    return s;
}

static void apply_time(void) {
    float *s = time_scale();
    if (!s)
        return;
    float want = c.freeze ? FROZEN : c.slow ? (float)settings[SLOW_PCT].value / 100.0f : 1.0f;
    if (c.freeze || c.slow) {
        if (!c.active) {
            c.saved = *s == FROZEN ? 1.0f : *s;
            c.active = 1;
        }
        *s = want;   // every frame: the game may set its own scale (a scene's slow motion)
    } else if (c.active) {
        *s = c.saved;
        c.active = 0;
    }
}

// ---- camera ----

static void set_camera(int on) {
    d.on = on;
    if (!on)
        carrying = 0;
    d.reset = on;
    d.toast = on ? now_tick() : 0;
    KAE_LOG("cinema: free camera %s", on ? "on" : "off");
}

static void cross_n(float *out, const float *a, const float *b) {
    float x = a[1] * b[2] - a[2] * b[1], y = a[2] * b[0] - a[0] * b[2], z = a[0] * b[1] - a[1] * b[0];
    float k = 1.0f / ksqrt(x * x + y * y + z * z);
    out[0] = x * k, out[1] = y * k, out[2] = z * k;
}

// Point the frame along back = (bx, by, bz) with no roll: right stays level.
static void aim(float *r, float bx, float by, float bz) {
    static const float world_up[3] = { 0, 1, 0 };
    if (ksqrt(bx * bx + bz * bz) < 1e-3f)
        bx = 0, bz = 1;
    float m = 1.0f / ksqrt(bx * bx + by * by + bz * bz);
    r[6] = bx * m, r[7] = by * m, r[8] = bz * m;
    cross_n(r, world_up, r + 6);
    cross_n(r + 3, r + 6, r);
}

static float stick(int32_t raw) {
    float x = (float)raw * (1.0f / 32767.0f), dz = 0.06f;
    float a = __builtin_fabsf(x) - dz;
    a = a < 0 ? 0 : a / (1.0f - dz);
    return __builtin_copysignf(a > 1 ? 1 : a, x);
}

static void step(const KaeInput *in) {
    uint64_t now = now_tick();
    float dt = d.tick ? (float)(now - d.tick) / TICK_HZ : 0;
    d.tick = now;
    if (dt > DT_MAX)
        dt = DT_MAX;

    if (kae_binding_pressed(&settings[K_CAM]))
        set_camera(!d.on);
    if (!d.on)
        return;
    int faster = kae_binding_pressed(&settings[K_FASTER]), slower = kae_binding_pressed(&settings[K_SLOWER]);
    if (faster || slower) {
        int s = settings[CAM_SPEED].value + faster - slower;
        settings[CAM_SPEED].value = s < 0 ? 0 : s >= N_SPEEDS ? N_SPEEDS - 1 : s;
        d.toast = now;
    }
    float *right = d.r, *up = d.r + 3, *back = d.r + 6;
    if (kae_binding_pressed(&settings[K_RESET]))
        d.reset = 1;
    if (d.reset) {
        d.reset = 0;
        for (int i = 0; i < 3; i++)
            d.p[i] = d.eye[i];
        aim(d.r, d.gback[0], 0, d.gback[2]);
    }

    float lx = stick(in->lx), ly = stick(in->ly), rx = stick(in->rx), ry = stick(in->ry);
    float turn = krad((float)settings[CAM_TURN].value) * dt, yaw = -rx * turn, look = ry * turn;
    float bx = back[0] + yaw * back[2] - look * up[0];
    float by = back[1] - look * up[1];
    float bz = back[2] - yaw * back[0] - look * up[2];
    float m = 1.0f / ksqrt(bx * bx + by * by + bz * bz);
    bx *= m, by *= m, bz *= m;
    if (__builtin_fabsf(by) > MAX_LOOK) {   // stop short of straight up / down
        float h = ksqrt((1.0f - MAX_LOOK * MAX_LOOK) / (bx * bx + bz * bz));
        bx *= h, bz *= h;
        by = __builtin_copysignf(MAX_LOOK, by);
    }
    aim(d.r, bx, by, bz);
    float vert = (float)kae_binding_held(&settings[K_UP]) - (float)kae_binding_held(&settings[K_DOWN]);
    float speed = SPEED_MS[settings[CAM_SPEED].value];
    for (int i = 0; i < 3; i++)
        d.p[i] += (right[i] * lx - back[i] * ly + (i == 1 ? vert : 0)) * speed * dt;

    // Carried Link: 1 m behind the camera along the level part of its back axis, out of view.
    float h = ksqrt(back[0] * back[0] + back[2] * back[2]);
    h = h > 1e-3f ? 1.0f / h : 0;
    d.carry_at[0] = d.p[0] + back[0] * h;
    d.carry_at[1] = d.p[1];
    d.carry_at[2] = d.p[2] + back[2] * h;

    // The view matrix: rows right / up / back, translation = -(row . eye).
    for (int row = 0; row < 3; row++) {
        float *o = d.mat + 4 * row, *a = d.r + 3 * row;
        o[0] = a[0], o[1] = a[1], o[2] = a[2];
        o[3] = -(a[0] * d.p[0] + a[1] * d.p[1] + a[2] * d.p[2]);
    }
}

static int standing_by(void) { return d.since_view > STANDBY_FRAMES || !d.have_game || kae_menu_open(); }

static void input(KaeInput *seen, const KaeInput *real, void *user) {
    (void)user;
    if (standing_by()) {
        d.tick = 0;
        return;
    }
    if (!d.stepped && real->connected && (real->controller == 0 || real->controller == 0x20)) {
        d.stepped = 1;
        step(real);
    }
    if (d.on) {   // + and - stay with the game: its pause menu, and the Mods menu chord
        seen->buttons &= KAE_PLUS | KAE_MINUS;
        seen->lx = seen->ly = seen->rx = seen->ry = 0;
    }
}

static void view(KaeView *v, void *user) {
    (void)user;
    d.since_view = 0;
    if (!d.on || d.reset) {
        // Keep the game's view for the next take-off: eye = -(R^T t), back = row 2.
        const float *m = v->matrix;
        for (int i = 0; i < 3; i++) {
            d.eye[i] = -(m[i] * m[3] + m[4 + i] * m[7] + m[8 + i] * m[11]);
            d.gback[i] = m[8 + i];
        }
        d.have_game = 1;
        return;
    }
    for (int i = 0; i < 12; i++)
        v->matrix[i] = d.mat[i];
}

// ---- requests and menu actions ----

void cinema_request(unsigned what) { __atomic_fetch_or(&requests, what, __ATOMIC_SEQ_CST); }
int cinema_camera_on(void) { return d.on; }
int cinema_frozen(void) { return c.freeze; }
int cinema_slow(void) { return c.slow; }
int cinema_health(void) { return health_on; }
int cinema_carrying(void) { return carrying; }
void cinema_inventory(int op, int a, int b) { inv_op = op, inv_a = a, inv_b = b, cinema_request(REQ_INV); }
int cinema_has_position(void) { return pos.saved; }
const char *cinema_link_status(void) { return link_status; }
void cinema_age(int age) { age_wanted = age, cinema_request(REQ_AGE); }
void cinema_gear(int index) {
    if (index >= 0 && index < gear_count)
        gear_wanted = index, cinema_request(REQ_GEAR);
}

static int same_scene(const char *a, const char *b) {
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

// Before a scene change: the fade runs on game time, and the camera would stay in the old scene.
static void before_scene_change(void) {
    if (c.freeze)
        set_freeze(0);
    if (c.slow)
        set_slow(0);
    if (d.on)
        set_camera(0);
}

void cinema_warp(int place, int spawn) {
    if (place < 0 || place >= places_count || spawn < places[place].first || spawn >= places[place].first + places[place].n)
        return;
    warp_place = place, warp_spawn = spawn;
    cinema_request(REQ_WARP);
}

static void do_inventory(int op, int a, int b) {   // the game's thread
    switch (op) {
    case INV_ITEM:        if (a >= 0 && a < inv_n_items) inv_set_owned(&inv_items[a], b); break;
    case INV_GEAR:        if (a >= 0 && a < inv_n_gear) inv_set_owned(&inv_gear[a], b); break;
    case INV_MASK:        inv_one_choose(inv_masks, inv_n_masks, b); break;
    case INV_TRADE_CHILD: inv_one_choose(inv_trade_child, inv_n_trade_child, b); break;
    case INV_TRADE_ADULT: inv_one_choose(inv_trade_adult, inv_n_trade_adult, b); break;
    case INV_QUEST_ITEM:  if (a >= 0 && a < inv_n_quest_items) inv_set_flag(&inv_quest_items[a], b); break;
    case INV_SONG:        if (a >= 0 && a < inv_n_songs) inv_set_flag(&inv_songs[a], b); break;
    case INV_AMMO:        inv_fill_ammo(); break;
    case INV_STAGE:       quest_set_stage(a, b); break;
    case INV_ALL_QUESTS:  quests_set_all(b); break;
    }
}

static void do_requests(void) {   // the game's thread
    unsigned r = __atomic_exchange_n(&requests, 0, __ATOMIC_SEQ_CST);
    if (r & REQ_CAMERA)
        set_camera(!d.on);
    if ((r & REQ_RESET) && d.on)
        d.reset = 1;
    if (r & REQ_FREEZE)
        set_freeze(!c.freeze);
    if (r & REQ_SLOW)
        set_slow(!c.slow);
    if (r & REQ_HEALTH) {
        health_on = !health_on;
        KAE_LOG("cinema: infinite health %s", health_on ? "on" : "off");
    }
    if (r & REQ_WARP) {
        before_scene_change();
        warp_go(&places[warp_place], &spawns[warp_spawn]);
    }
    if (r & REQ_AGE) {
        before_scene_change();
        const char *error = age_reload(age_wanted);
        link_status = error ? error : "Rechargement du lieu...";
    }
    if (r & REQ_GEAR) {
        link_status = gear_wear(gear_wanted);
        if (link_status == gear_reload_needed && age_current() >= 0) {
            before_scene_change();
            const char *error = age_reload(age_current());
            if (error)
                link_status = error;
        }
    }
    if (r & REQ_POS_SAVE)
        pos.save_wanted = 1;   // done in player_move, where Link's position is known
    if (r & REQ_POS_LOAD) {
        const char *scene = link_scene();
        if (!pos.saved)
            link_status = "Aucune position sauvée.";
        else if (!scene || !same_scene(scene, pos.scene))
            link_status = "La position sauvée est dans un autre lieu.";
        else
            pos.loading = LOAD_UPDATES, link_status = "Position rechargée.";
    }
    if (r & REQ_INV)
        do_inventory(inv_op, inv_a, inv_b);
    if (r & REQ_CARRY) {
        carrying = d.on && !carrying;
        link_status = d.on ? "" : "Active d'abord la caméra libre.";
        KAE_LOG("cinema: carrying Link %s", carrying ? "on" : "off");
    }
}

// Link's movement update: the place to save, load or carry his position.
static void player_move(const KaePlayer *link, void *user) {
    (void)user;
    link_seen(link->object, link->actor);
    if (pos.save_wanted) {
        pos.save_wanted = 0;
        const char *scene = link_scene();
        int i = 0;
        for (; scene && scene[i] && i < (int)sizeof(pos.scene) - 1; i++)
            pos.scene[i] = scene[i];
        pos.scene[i] = 0;
        for (i = 0; i < 3; i++)
            pos.at[i] = link->position[i];
        pos.saved = 1;
        link_status = "Position sauvée.";
    }
    if (pos.loading > 0) {
        pos.loading--;
        kae_player_move(link, pos.at);
    }
    if (carrying && d.on)
        kae_player_move(link, d.carry_at);
}

// Menu actions run on the game's thread.
static void act_panel(KaeItem *item, void *user) { (void)item, (void)user; panel_toggle(); }

static void act_camera(KaeItem *item, void *user) { (void)item, (void)user; set_camera(!d.on); }
static void act_freeze(KaeItem *item, void *user) { (void)item, (void)user; set_freeze(!c.freeze); }
static void act_slow(KaeItem *item, void *user) { (void)item, (void)user; set_slow(!c.slow); }

// ---- frame and drawing ----

static void frame(void *user) {
    (void)user;
    if (d.since_view <= STANDBY_FRAMES)
        d.since_view++;
    d.stepped = 0;
    do_requests();
    warp_frame();
    if (health_on)
        health_refill();
    if (kae_binding_pressed(&settings[K_PANEL]))
        panel_toggle();
    if (kae_binding_pressed(&settings[K_FREEZE]))
        set_freeze(!c.freeze);
    if (kae_binding_pressed(&settings[K_SLOW]))
        set_slow(!c.slow);
    apply_time();
}

static void draw_indicator(void) {
    if (!settings[SHOW].value || kae_menu_open())
        return;
    int y = 20;
    if (c.freeze) {
        kae_draw_text(20, y, 2, 0xFFD040, "PAUSE");
        y += 36;
    } else if (c.slow) {
        char text[] = "RALENTI 00%";
        int pct = settings[SLOW_PCT].value;
        text[8] = (char)('0' + pct / 10 % 10);
        text[9] = (char)('0' + pct % 10);
        kae_draw_text(20, y, 2, 0x40C0FF, text);
        y += 36;
    }
    if (d.on && d.toast && (float)(now_tick() - d.toast) / TICK_HZ < TOAST_S) {
        char text[32] = "CAMERA ";
        const char *s = SPEEDS[settings[CAM_SPEED].value];
        int n = 7;
        while (*s && n < (int)sizeof(text) - 1)
            text[n++] = *s++;
        text[n] = 0;
        kae_draw_text(20, y, 2, 0xFFFFFF, text);
    }
}

static void draw(const KaeFrame *frame, void *user) {
    (void)user;
    draw_indicator();
    panel_draw(frame);
}

static int start(void) {
    const char *build = kae_game_build();
    if (!same_build(build, BUILD_111)) {
        KAE_LOG("cinema: made for game 1.1.1 only (this build: %s): doing nothing", build);
        return 2;
    }
    if (kae_on_frame(frame, 0, 0) != KAE_OK)
        return 2;
    if (kae_on_view(view, 0, 20) != KAE_OK || kae_on_input(input, 0, 20) != KAE_OK)
        KAE_LOG("cinema: no view or input event (Core older than 0.8?): no free camera");
    if (kae_on_player_move(player_move, 0, 0) != KAE_OK)
        KAE_LOG("cinema: no player_move event: Link's position can't be saved or carried");
    kae_on_draw(draw, 0, 0);
    kae_settings("Cinema", settings, COUNT);
    KAE_LOG("cinema 0.6.0 on Core");
    return 0;
}

KAE_MOD("seitanmeitoo.cinema", "0.6.0", start, 0);
