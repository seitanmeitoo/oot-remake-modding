// Cinema: what cinema.c (camera, time, settings) shares with panel.c (the mouse panel).
#pragma once

#include <kaegae.h>

enum {
    A_PANEL, A_CAM, A_FREEZE, A_SLOW,
    CAM_SPEED, CAM_TURN, SLOW_PCT, SHOW,
    K_PANEL, K_CAM, K_RESET, K_UP, K_DOWN, K_FASTER, K_SLOWER, K_FREEZE, K_SLOW,
    COUNT
};
extern KaeItem settings[COUNT];
extern const char *const SPEEDS[];

// What the panel asks for, done by cinema.c on the game's thread at the next frame.
enum {
    REQ_CAMERA = 1, REQ_RESET = 2, REQ_FREEZE = 4, REQ_SLOW = 8, REQ_HEALTH = 16, REQ_WARP = 32,
    REQ_AGE = 64, REQ_GEAR = 128, REQ_POS_SAVE = 256, REQ_POS_LOAD = 512, REQ_CARRY = 1024,
};
void cinema_request(unsigned what);
void cinema_warp(int place, int spawn);   // indexes into places[] / spawns[] (warp.h); posts REQ_WARP
int cinema_camera_on(void);
int cinema_frozen(void);
int cinema_slow(void);
int cinema_health(void);                  // Link's health kept full
void cinema_age(int age);                 // reload the scene at this age (AGE_* in link.h); posts REQ_AGE
void cinema_gear(int index);              // put on gear[index] (link.h); posts REQ_GEAR
int cinema_carrying(void);                // Link follows the free camera
int cinema_has_position(void);            // a position is saved
const char *cinema_link_status(void);     // the last Link action's result, for the panel

void panel_toggle(void);
void panel_draw(const KaeFrame *frame);   // inside Core's draw
