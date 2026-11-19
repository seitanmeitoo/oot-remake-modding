// Cinema's panel: buttons clicked with the PC mouse, drawn over the game.
//
// The mouse is the emulator's emulated mouse, read with nn::hid::GetMouseStates as Mouse Grab does
// (mods/mouse-grab, MIT, KaboWabo/Kabosune): a 1280 x 720 space whatever the picture size, left
// button in bit 0, attributes bit 1 = connected. In Fritchii it needs `mouse_enabled` on (the
// controls settings, Advanced tab). The arrow cursor is Mouse Grab's too.
//
// Immediate mode: every draw lays the panel out and a click (the left button going down) inside a
// button acts on it. Clicks only post requests (cinema_request) or change a setting's value; the
// state itself changes on the game's thread.
#include "cinema.h"
#include "warp.h"
#include "link.h"
#include "inventory.h"

typedef struct {
    int64_t  sample;
    int32_t  x, y;               // 0..1279, 0..719
    int32_t  dx, dy;
    int32_t  wheel_x, wheel_y;
    uint32_t buttons;            // bit 0 left, bit 1 right, bit 2 middle
    uint32_t attributes;         // bit 1: a mouse is connected
} MouseState;
_Static_assert(sizeof(MouseState) == 0x28, "nn::hid::MouseState");
int _ZN2nn3hid14GetMouseStatesEPNS0_10MouseStateEi(MouseState *states, int count);

enum { MOUSE_W = 1280, MOUSE_H = 720, MOUSE_CONNECTED = 2 };
enum {
    C_BACK = 0x18181E, C_TITLE = 0xFFD040, C_TEXT = 0xE8E8E8, C_DIM = 0x9090A0,
    C_BUTTON = 0x3A4050, C_HOVER = 0x56688C, C_ON = 0x2E7A44, C_BLACK = 0x000000, C_WHITE = 0xFFFFFF,
    C_LIST = 0x24242C,
};

static const char *const arrow[] = {   // '#' outline, '.' fill
    "#", "##", "#.#", "#..#", "#...#", "#....#", "#.....#", "#......#", "#.......#", "#........#",
    "#.........#", "#..........#", "#......#####", "#...#..#", "#..# #..#", "#.#  #..#", "##    #..#",
    "#     #..#", "       ##",
};

static volatile int open;
static uint32_t prev_buttons;
static int64_t prev_sample;

// What the panel shows: its page, and on the warp page the chosen category, place and spawn.
enum { PAGE_CAMERA, PAGE_WARP, PAGE_LINK, PAGE_INVENTORY, PAGE_STORY };
static int page, category, place = -1, spawn, scroll_places, scroll_spawns;
static int inv_tab, quest = -1, scroll_quests, scroll_stages;
#define LIST_ROWS 10

// This draw's layout: scale, mouse, whether the left button went down, wheel notches (+1 up).
static struct { int k, mx, my, click, wheel; } ui;

void panel_toggle(void) {
    open = !open;
    KAE_LOG("cinema: panel %s", open ? "open" : "closed");
}

static int inside(int x, int y, int w, int h) { return ui.mx >= x && ui.mx < x + w && ui.my >= y && ui.my < y + h; }

// A button at (*x, y); moves *x past it. 1 when clicked.
static int button(int *x, int y, const char *label, int on) {
    int k = ui.k, w = kae_text_width(k, label) + 12 * k, h = 22 * k;
    int hover = inside(*x, y, w, h);
    kae_draw_rect(*x, y, w, h, on ? C_ON : hover ? C_HOVER : C_BUTTON);
    kae_draw_text(*x + 6 * k, y + 3 * k, k, C_WHITE, label);
    *x += w + 6 * k;
    return hover && ui.click;
}

static void put_int(char **o, int v) {
    char tmp[12];
    int n = 0;
    if (v < 0)
        *(*o)++ = '-', v = -v;
    do tmp[n++] = (char)('0' + v % 10); while ((v /= 10) && n < 11);
    while (n)
        *(*o)++ = tmp[--n];
}

static void put_str(char **o, const char *s) {
    while (*s)
        *(*o)++ = *s++;
}

// [-] value [+] on a number setting, `text` drawn between them in a fixed width.
static void stepper(int *x, int y, KaeItem *it, const char *text) {
    int k = ui.k, delta = 0;
    if (button(x, y, "-", 0))
        delta = -1;
    kae_draw_text(*x + 4 * k, y + 3 * k, k, C_TEXT, text);
    *x += 11 * 8 * k;
    if (button(x, y, "+", 0))
        delta = 1;
    if (delta) {
        int v = it->value + delta * (it->step ? it->step : 1);   // a choice has no step: one at a time
        it->value = v < it->min ? it->min : v > it->max ? it->max : v;
    }
}

static void draw_arrow(int left, int top, int s, uint32_t fill) {
    for (int row = 0; row < (int)(sizeof arrow / sizeof *arrow); row++) {
        const char *a = arrow[row];
        for (int i = 0; a[i];) {
            int start = i;
            char kind = a[i];
            while (a[i] == kind)
                i++;
            if (kind != ' ')
                kae_draw_rect(left + start * s, top + row * s, (i - start) * s, s, kind == '#' ? C_BLACK : fill);
        }
    }
}

// At most `chars` characters of `s` (UTF-8) into `out`, ending in "..." when it is cut.
static const char *fit(char *out, int size, const char *s, int chars) {
    int total = 0;
    for (const char *t = s; *t; t++)
        total += ((unsigned char)*t & 0xC0) != 0x80;
    int keep = total <= chars ? total : chars - 3, n = 0, count = 0;
    for (; s[n] && n < size - 4; n++) {
        if (((unsigned char)s[n] & 0xC0) != 0x80 && count++ == keep)
            break;
        out[n] = s[n];
    }
    if (total > chars)
        out[n++] = '.', out[n++] = '.', out[n++] = '.';
    out[n] = 0;
    return out;
}

// A scrolling list of `count` labels; returns the row clicked, or -1. `selected` is highlighted.
static int list(int x, int y, int w, int count, int selected, int *scroll, const char *(*label)(int)) {
    int k = ui.k, lh = 18 * k, h = LIST_ROWS * lh, clicked = -1;
    kae_draw_rect(x, y, w, h, C_LIST);
    if (inside(x, y, w, h) && ui.wheel)
        *scroll -= ui.wheel;
    int most = count > LIST_ROWS ? count - LIST_ROWS : 0;
    *scroll = *scroll < 0 ? 0 : *scroll > most ? most : *scroll;
    char text[96];
    for (int r = 0; r < LIST_ROWS && *scroll + r < count; r++) {
        int i = *scroll + r, ry = y + r * lh, hover = inside(x, ry, w, lh);
        if (i == selected || hover)
            kae_draw_rect(x, ry, w, lh, i == selected ? C_ON : C_HOVER);
        kae_draw_text(x + 4 * k, ry + k, k, C_TEXT, fit(text, sizeof(text), label(i), (w - 12 * k) / (8 * k)));
        if (hover && ui.click)
            clicked = i;
    }
    if (count > LIST_ROWS) {   // where the view is in the list
        int bar = h * LIST_ROWS / count, at = (h - bar) * *scroll / most;
        kae_draw_rect(x + w - 3 * k, y + at, 3 * k, bar, C_DIM);
    }
    return clicked;
}

// The places of the chosen category, in places[] order.
static int in_category[96], n_in_category;
static const char *place_label(int i) { return places[in_category[i]].label; }
static const char *spawn_label(int i) { return spawns[places[place].first + i].label; }

static void page_camera(int x0, int y, int pad, int label_w, int row) {
    int k = ui.k, x;
    char text[24], *o;
    #define LABEL(s) kae_draw_text(x0 + pad, y + 3 * k, k, C_DIM, s), x = x0 + pad + label_w

    LABEL("Caméra libre");
    int cam = cinema_camera_on();
    if (button(&x, y, cam ? "Couper" : "Activer", cam))
        cinema_request(REQ_CAMERA);
    if (cam && button(&x, y, "Replacer", 0))
        cinema_request(REQ_RESET);
    y += row;

    LABEL("Vitesse caméra");
    stepper(&x, y, &settings[CAM_SPEED], SPEEDS[settings[CAM_SPEED].value]);
    y += row;

    LABEL("Rotation caméra");
    o = text;
    put_int(&o, settings[CAM_TURN].value);
    put_str(&o, " °/s");
    *o = 0;
    stepper(&x, y, &settings[CAM_TURN], text);
    y += row + row / 2;

    LABEL("Monde");
    int frozen = cinema_frozen();
    if (button(&x, y, frozen ? "Relancer" : "Figer", frozen))
        cinema_request(REQ_FREEZE);
    y += row;

    LABEL("Ralenti");
    int slow = cinema_slow();
    if (button(&x, y, slow ? "Couper" : "Activer", slow))
        cinema_request(REQ_SLOW);
    y += row;

    LABEL("Vitesse du ralenti");
    o = text;
    put_int(&o, settings[SLOW_PCT].value);
    put_str(&o, " %");
    *o = 0;
    stepper(&x, y, &settings[SLOW_PCT], text);
    y += row + row / 2;

    LABEL("Indicateur à l'écran");
    if (button(&x, y, settings[SHOW].value ? "Affiché" : "Masqué", settings[SHOW].value))
        settings[SHOW].value = !settings[SHOW].value;
    #undef LABEL
}

static void page_link(int x0, int y, int pad, int label_w, int row) {
    int k = ui.k, x;
    #define LABEL(s) kae_draw_text(x0 + pad, y + 3 * k, k, C_DIM, s), x = x0 + pad + label_w

    int age = age_current();
    LABEL("Âge");
    if (button(&x, y, "Enfant", age == AGE_CHILD))
        cinema_age(AGE_CHILD);
    if (button(&x, y, "Adulte", age == AGE_ADULT))
        cinema_age(AGE_ADULT);
    if (button(&x, y, "Recharger", 0) && age >= 0)
        cinema_age(age);
    y += row + row / 2;

    for (int slot = 0; slot < EQ_COUNT; slot++) {
        LABEL(gear_slots[slot]);
        for (int i = 0; i < gear_count; i++)
            if (gear[i].slot == slot && (age < 0 || (gear[i].ages & (age == AGE_CHILD ? FOR_CHILD : FOR_ADULT))))
                if (button(&x, y, gear[i].label, gear_worn(i)))
                    cinema_gear(i);
        y += row;
    }
    y += row / 2;

    LABEL("Position");
    if (button(&x, y, "Sauver", 0))
        cinema_request(REQ_POS_SAVE);
    if (button(&x, y, "Recharger", 0))
        cinema_request(REQ_POS_LOAD);
    y += row;

    LABEL("Déplacer Link");
    int carry = cinema_carrying();
    if (button(&x, y, carry ? "Lâcher" : "Suit la caméra", carry))
        cinema_request(REQ_CARRY);
    y += row;

    LABEL("Vie infinie");
    int health = cinema_health();
    if (button(&x, y, health ? "Oui" : "Non", health))
        cinema_request(REQ_HEALTH);
    y += row;

    kae_draw_text(x0 + pad, y + 3 * k, k, C_TEXT, cinema_link_status());
    #undef LABEL
}

// A button in a flowing row: it goes to the next line when it doesn't fit before xmax.
static int flow(int *x, int *y, int left, int xmax, int row, const char *label, int on) {
    if (*x + kae_text_width(ui.k, label) + 12 * ui.k > xmax)
        *x = left, *y += row;
    return button(x, *y, label, on);
}

// "None" and one button per item; the one held is lit. Returns the choice clicked, or -1.
static int one_of(int *x, int *y, int left, int xmax, int row, const pouch_one_t *items, int n) {
    int held = inv_one_chosen(items, n), clicked = -1;
    if (flow(x, y, left, xmax, row, "Aucun", held == 0))
        clicked = 0;
    for (int i = 0; i < n; i++)
        if (flow(x, y, left, xmax, row, items[i].label, held == i + 1))
            clicked = i + 1;
    return clicked;
}

static void page_inventory(int x0, int y, int pad, int w, int row) {
    static const char *const tabs[] = { "Objets", "Équipement", "Masques", "Échange", "Quête et chansons" };
    int k = ui.k, left = x0 + pad, xmax = x0 + w - pad, x = left;
    for (int t = 0; t < 5; t++)
        if (button(&x, y, tabs[t], t == inv_tab))
            inv_tab = t;
    y += row + row / 2;
    x = left;
    int c;
    switch (inv_tab) {
    case 0:
        for (int i = 0; i < inv_n_items; i++)
            if (inv_items[i].kind == IK_OWNED || inv_items[i].kind == IK_ACQUIRED) {
                int on = inv_owned(&inv_items[i]);
                if (flow(&x, &y, left, xmax, row, inv_items[i].label, on))
                    cinema_inventory(INV_ITEM, i, !on);
            }
        y += row + row / 2;
        x = left;
        if (button(&x, y, "Munitions au maximum", 0))
            cinema_inventory(INV_AMMO, 0, 0);
        break;
    case 1:
        for (int i = 0; i < inv_n_gear; i++)
            if (inv_gear[i].worn_slot) {   // the ocarinas need the game's pouch: left out
                int on = inv_owned(&inv_gear[i]);
                if (flow(&x, &y, left, xmax, row, inv_gear[i].label, on))
                    cinema_inventory(INV_GEAR, i, !on);
            }
        break;
    case 2:
        if ((c = one_of(&x, &y, left, xmax, row, inv_masks, inv_n_masks)) >= 0)
            cinema_inventory(INV_MASK, 0, c);
        break;
    case 3:
        kae_draw_text(left, y + 3 * k, k, C_DIM, "Enfant");
        y += row;
        if ((c = one_of(&x, &y, left, xmax, row, inv_trade_child, inv_n_trade_child)) >= 0)
            cinema_inventory(INV_TRADE_CHILD, 0, c);
        y += row + row / 2;
        x = left;
        kae_draw_text(left, y + 3 * k, k, C_DIM, "Adulte");
        y += row;
        if ((c = one_of(&x, &y, left, xmax, row, inv_trade_adult, inv_n_trade_adult)) >= 0)
            cinema_inventory(INV_TRADE_ADULT, 0, c);
        break;
    default:
        for (int i = 0; i < inv_n_quest_items; i++) {
            int on = inv_flag(&inv_quest_items[i]);
            if (flow(&x, &y, left, xmax, row, inv_quest_items[i].label, on))
                cinema_inventory(INV_QUEST_ITEM, i, !on);
        }
        y += row + row / 2;
        x = left;
        for (int i = 0; i < inv_n_songs; i++) {
            int on = inv_flag(&inv_songs[i]);
            if (flow(&x, &y, left, xmax, row, inv_songs[i].label, on))
                cinema_inventory(INV_SONG, i, !on);
        }
        break;
    }
}

static const char *quest_label(int i) { return inv_quests[i].label; }
static const char *stage_label(int i) { return inv_quests[quest].stages[i]; }

static void page_story(int x0, int y, int pad, int w, int row) {
    int k = ui.k, x = x0 + pad;
    if (button(&x, y, "Tout terminer", 0))
        cinema_inventory(INV_ALL_QUESTS, 0, 1);
    if (button(&x, y, "Tout au début", 0))
        cinema_inventory(INV_ALL_QUESTS, 0, 0);
    y += row + row / 2;

    int col = (w - 2 * pad - 8 * k) / 2, x2 = x0 + pad + col + 8 * k, lists_h = LIST_ROWS * 18 * k;
    kae_draw_text(x0 + pad, y, k, C_DIM, "Quête");
    kae_draw_text(x2, y, k, C_DIM, "Étape (nom du jeu)");
    y += 20 * k;
    int hit = list(x0 + pad, y, col, inv_n_quests, quest, &scroll_quests, quest_label);
    if (hit >= 0 && hit != quest)
        quest = hit, scroll_stages = 0;
    if (quest >= 0) {
        hit = list(x2, y, col, inv_quests[quest].n_stages, quest_stage(quest), &scroll_stages, stage_label);
        if (hit >= 0)
            cinema_inventory(INV_STAGE, quest, hit);
    } else {
        kae_draw_rect(x2, y, col, lists_h, C_LIST);
        kae_draw_text(x2 + 4 * k, y + k, k, C_DIM, "Choisis une quête.");
    }
    y += lists_h + 8 * k;
    kae_draw_text(x0 + pad, y + 3 * k, k, C_DIM,
                  "Change la sauvegarde : prends-en une à part. Souvent visible\naprès un rechargement du lieu (onglet Link).");
}

static void page_warp(int x0, int y, int pad, int w, int row) {
    int k = ui.k, x = x0 + pad;
    for (int c = 0; c < PC_COUNT; c++)
        if (button(&x, y, places_categories[c], c == category) && c != category)
            category = c, place = -1, scroll_places = 0;
    y += row;

    n_in_category = 0;
    for (int i = 0; i < places_count && n_in_category < (int)(sizeof(in_category) / sizeof(*in_category)); i++)
        if (places[i].category == category)
            in_category[n_in_category++] = i;
    int sel = -1;
    for (int i = 0; i < n_in_category; i++)
        if (in_category[i] == place)
            sel = i;

    int col = (w - 2 * pad - 8 * k) / 2, x2 = x0 + pad + col + 8 * k, lists_h = LIST_ROWS * 18 * k;
    kae_draw_text(x0 + pad, y, k, C_DIM, "Lieu");
    kae_draw_text(x2, y, k, C_DIM, "Point d'arrivée");
    y += 20 * k;
    int hit = list(x0 + pad, y, col, n_in_category, sel, &scroll_places, place_label);
    if (hit >= 0 && in_category[hit] != place)
        place = in_category[hit], spawn = 0, scroll_spawns = 0;
    if (place >= 0) {
        hit = list(x2, y, col, places[place].n, spawn, &scroll_spawns, spawn_label);
        if (hit >= 0)
            spawn = hit;
    } else {
        kae_draw_rect(x2, y, col, lists_h, C_LIST);
        kae_draw_text(x2 + 4 * k, y + k, k, C_DIM, "Choisis un lieu.");
    }
    y += lists_h + 8 * k;

    x = x0 + pad;
    if (place >= 0 && button(&x, y, "Y aller", 0))
        cinema_warp(place, places[place].first + spawn);
    kae_draw_text(x, y + 3 * k, k, C_TEXT, warp_status());
    y += row;
    kae_draw_text(x0 + pad, y + 3 * k, k, C_DIM, "Molette : défiler. En partant, le monde reprend et la caméra se coupe.");
}

void panel_draw(const KaeFrame *frame) {
    if (!open || kae_menu_open())
        return;
    MouseState m = { 0 };
    int got = _ZN2nn3hid14GetMouseStatesEPNS0_10MouseStateEi(&m, 1);
    int mouse = got > 0 && (m.attributes & MOUSE_CONNECTED);
    ui.k = frame->height >= 900 ? 2 : 1;
    ui.mx = mouse ? (int)((int64_t)m.x * frame->width / MOUSE_W) : -1;
    ui.my = mouse ? (int)((int64_t)m.y * frame->height / MOUSE_H) : -1;
    ui.click = mouse && (m.buttons & 1) && !(prev_buttons & 1);
    // The wheel's delta once per new mouse sample (several draws can see the same one), one row a notch.
    ui.wheel = mouse && m.sample != prev_sample ? (m.wheel_y > 0) - (m.wheel_y < 0) : 0;
    prev_buttons = mouse ? m.buttons : 0;
    prev_sample = m.sample;

    int k = ui.k, row = 26 * k, pad = 10 * k, label_w = 20 * 8 * k;
    int w = 70 * 8 * k + 2 * pad, h = 14 * row + 2 * pad;
    int x0 = frame->width - w - 24 * k, y0 = 24 * k, y = y0 + pad, x;
    if (x0 < 0)
        x0 = 0;
    kae_draw_rect(x0, y0, w, h, C_BACK);

    // Title, the pages, and a close button on the right.
    static const char *const pages[] = { "Caméra", "Téléportation", "Link", "Inventaire", "Histoire" };
    kae_draw_text(x0 + pad, y + 3 * k, k, C_TITLE, "Cinéma");
    x = x0 + pad + 8 * 8 * k;
    for (int p = 0; p < 5; p++)
        if (button(&x, y, pages[p], page == p))
            page = p;
    x = x0 + w - pad - kae_text_width(k, "Fermer") - 12 * k;
    if (button(&x, y, "Fermer", 0))
        open = 0;
    y += row + row / 2;
    if (!mouse) {
        kae_draw_text(x0 + pad, y + 3 * k, k, C_TEXT,
                      "Souris non détectée.\nDans Fritchii : réglages des contrôles,\nonglet Avancé, cocher Souris.");
        return;
    }
    if (page == PAGE_CAMERA)
        page_camera(x0, y, pad, label_w, row);
    else if (page == PAGE_LINK)
        page_link(x0, y, pad, label_w, row);
    else if (page == PAGE_INVENTORY)
        page_inventory(x0, y, pad, w, row);
    else if (page == PAGE_STORY)
        page_story(x0, y, pad, w, row);
    else
        page_warp(x0, y, pad, w, row);

    draw_arrow(ui.mx, ui.my, k, (m.buttons & 1) ? C_TITLE : C_WHITE);
}
