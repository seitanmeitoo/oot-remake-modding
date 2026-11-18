// Cinema 0.1: time control for shots (freeze the world, slow motion), on KaeGae Core.
//
// The game's time manager multiplies every frame delta by a global time scale. Writing it slows
// or stops the whole world (Link included) while the game keeps drawing, so a free camera such as
// FPV Drone (which runs on the system tick, not on game time) still flies around a frozen scene.
// Found by the Lichii photo mode (zortos293/Lichii PR #618): **(main + 0x4c7e890) + 0x1c0, a float.
// Frozen is 1e-4, not 0: the game also takes the reciprocal of the scaled step.
//
// shortcut: raw offsets of game 1.1.1 (kae_game_base); the mod switches itself off on any other
// build. Move them into the symbol packs (a "TimeMgrPtr" global) once 1.0.0's address is known.
// shortcut: Core gives a mod switched off in the Mods menu no more events, so the scale it set
// stays; unfreeze before switching Cinema off. A stop hook in Core would fix it.
#include <kaegae.h>

#define BUILD_111       "E0F3CCFAF50065AE4EA01EB5AEB13B1A45DA3348"
#define TIME_MGR_SLOT   0x4c7e890
#define TIME_SCALE_OFF  0x1c0
#define FROZEN          1e-4f

#define BIND(label, help, buttons) { KAE_BINDING, label, help, 0, 0, 0, 0, 0, 0, buttons, buttons, 0, 0 }
#define NUM(label, help, def, lo, hi, step) { KAE_NUMBER, label, help, def, def, lo, hi, step, 0, 0, 0, 0, 0 }
#define TOGGLE(label, help, def) { KAE_TOGGLE, label, help, def, def, 0, 1, 1, 0, 0, 0, 0, 0 }

enum { FREEZE, SLOW, SLOW_PCT, SHOW, COUNT };
static KaeItem settings[] = {
    [FREEZE]   = BIND("Figer / relancer le monde", "Tout s'arrête sauf la caméra libre.", KAE_R3),
    [SLOW]     = BIND("Ralenti on / off", "Le monde entier ralentit, Link compris.", KAE_DOWN),
    [SLOW_PCT] = NUM("Vitesse du ralenti (%)", "100 = vitesse normale.", 25, 5, 95, 5),
    [SHOW]     = TOGGLE("Afficher l'indicateur", "PAUSE ou RALENTI en haut à gauche.", 1),
};

static struct {
    volatile int freeze, slow;
    int active;      // our scale is written into the game
    float saved;     // the game's scale before we changed it
} c;

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

static void frame(void *user) {
    (void)user;
    if (kae_binding_pressed(&settings[FREEZE])) {
        c.freeze = !c.freeze;
        KAE_LOG("cinema: world %s", c.freeze ? "frozen" : "running");
    }
    if (kae_binding_pressed(&settings[SLOW])) {
        c.slow = !c.slow;
        KAE_LOG("cinema: slow motion %s", c.slow ? "on" : "off");
    }
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

static void draw(const KaeFrame *frame, void *user) {
    (void)frame, (void)user;
    if (!settings[SHOW].value || kae_menu_open())
        return;
    if (c.freeze) {
        kae_draw_text(20, 20, 2, 0xFFD040, "PAUSE");
    } else if (c.slow) {
        char text[] = "RALENTI 00%";
        int pct = settings[SLOW_PCT].value;
        text[8] = (char)('0' + pct / 10 % 10);
        text[9] = (char)('0' + pct % 10);
        kae_draw_text(20, 20, 2, 0x40C0FF, text);
    }
}

static int start(void) {
    const char *build = kae_game_build();
    if (!same_build(build, BUILD_111)) {
        KAE_LOG("cinema: made for game 1.1.1 only (this build: %s): doing nothing", build);
        return 2;
    }
    if (kae_on_frame(frame, 0, 0) != KAE_OK)
        return 2;
    kae_on_draw(draw, 0, 0);
    kae_settings("Cinema", settings, COUNT);
    KAE_LOG("cinema 0.1.0 on Core");
    return 0;
}

KAE_MOD("seitanmeitoo.cinema", "0.1.0", start, 0);
