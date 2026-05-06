/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"

// The Tamagotchi pet only fits on the default (non-VIA) build. The via build
// has VIA_ENABLE machinery that crowds out the ~3 KB needed for sprites + engine.
#ifndef VIA_ENABLE
#    define PET_ENABLE
#endif

#ifdef PET_ENABLE
#    include <avr/eeprom.h>
#    include <string.h>
#    include "pet_sprites.h"
#endif

enum custom_keycodes {
    PET_FEED = QK_USER_0,
    PET_PLAY,
    PET_CLEAN,
    PET_DISC,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // Layer 0 — alphas + home-row mods, layer-tap thumbs
  [0] = LAYOUT_universal(
    KC_Q        , KC_W        , KC_E        , KC_R        , KC_T        ,                                            KC_Y        , KC_U        , KC_I        , KC_O        , KC_P           ,
    LGUI_T(KC_A), LALT_T(KC_S), LCTL_T(KC_D), LSFT_T(KC_F), KC_G        ,                                            KC_H        , RSFT_T(KC_J), RCTL_T(KC_K), RALT_T(KC_L), RGUI_T(KC_SCLN),
    KC_Z        , KC_X        , KC_C        , KC_V        , KC_B        ,                                            KC_N        , KC_M        , KC_COMM     , KC_DOT      , KC_SLSH        ,
    KC_DEL      , KC_MINS     , KC_GRV      , LT(3,KC_ESC), LT(2,KC_TAB), LT(1,KC_SPC) ,        KC_ENT  , LT(1,KC_BSPC), KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_QUOT
  ),

  // Layer 1 — F-keys, shifted symbols, mouse buttons
  [1] = LAYOUT_universal(
    KC_F1       , KC_F2       , KC_F3       , KC_F4       , KC_F5       ,                                            KC_F6       , KC_F7       , KC_F8       , KC_F9       , KC_F10         ,
    S(KC_1)     , S(KC_2)     , S(KC_3)     , S(KC_4)     , S(KC_5)     ,                                            S(KC_GRV)   , KC_BTN1     , KC_PGUP     , KC_BTN2     , KC_F11         ,
    S(KC_6)     , KC_EQL      , S(KC_8)     , S(KC_9)     , S(KC_0)     ,                                            KC_LBRC     , S(KC_4)     , KC_PGDN     , KC_BTN3     , KC_BSLS        ,
    KC_INT1     , KC_EQL      , S(KC_3)     , KC_TRNS     , KC_TRNS     , KC_TRNS      ,        TO(2)   , TO(0)        , KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_F12
  ),

  // Layer 2 — navigation, numpad, media
  [2] = LAYOUT_universal(
    KC_VOLU     , KC_HOME     , KC_UP       , KC_END      , S(KC_LBRC)  ,                                            S(KC_RBRC)  , KC_P7       , KC_P8       , KC_P9       , KC_PPLS        ,
    KC_VOLD     , KC_LEFT     , KC_DOWN     , KC_RGHT     , KC_LBRC     ,                                            KC_RBRC     , KC_P4       , KC_P5       , KC_P6       , KC_PMNS        ,
    KC_MUTE     , KC_PGUP     , KC_CAPS     , KC_PGDN     , S(KC_9)     ,                                            S(KC_0)     , KC_P1       , KC_P2       , KC_P3       , KC_PAST        ,
    KC_MPRV     , KC_MPLY     , KC_MNXT     , KC_TRNS     , KC_TRNS     , KC_TRNS      ,        KC_NUM  , KC_P0        , KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_EQL
  ),

  // Layer 3 — keyboard config + RGB + Tamagotchi care
  // Right-side rows 0-1 col 1-2 are the pet care keys (FEED/PLAY on row 0, CLEAN/DISC on row 1).
  [3] = LAYOUT_universal(
    RGB_TOG     , AML_TO      , AML_I50     , AML_D50     , KC_TRNS     ,                                            KC_TRNS     , PET_FEED    , PET_PLAY    , KC_TRNS     , KC_TRNS        ,
    RGB_MOD     , RGB_HUI     , RGB_SAI     , RGB_VAI     , SCRL_DVD    ,                                            KC_TRNS     , PET_CLEAN   , PET_DISC    , KC_TRNS     , KC_TRNS        ,
    RGB_RMOD    , RGB_HUD     , RGB_SAD     , RGB_VAD     , SCRL_DVI    ,                                            CPI_D1K     , CPI_D100    , CPI_I100    , CPI_I1K     , KBC_SAVE       ,
    QK_BOOT     , KBC_RST     , KC_TRNS     , KC_TRNS     , SSNP_VRT    , SSNP_HOR     ,        SSNP_FRE, KC_TRNS      , KC_TRNS    , KC_TRNS     , KC_TRNS     , QK_BOOT
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    return state;
}

// ---------------------------------------------------------------------------
// Tamagotchi pet: state, EEPROM persistence, tick mechanics, care actions
// ---------------------------------------------------------------------------

#ifdef PET_ENABLE

#define PET_EEPROM_ADDR ((uint8_t *)525)  // safe zone after dynamic_macro
#define PET_MAGIC       0xCAFE

// Stage indices into PET_SPRITES[]
#define PET_STAGE_EGG        0
#define PET_STAGE_BABY       1
#define PET_STAGE_CHILD      2
#define PET_STAGE_TEEN_GOOD  3
#define PET_STAGE_TEEN_BAD   4
#define PET_STAGE_ADULT_GOOD 5
#define PET_STAGE_ADULT_BAD  6
#define PET_STAGE_DEAD       7

typedef struct {
    uint16_t magic;
    uint8_t  stage;
    uint8_t  hunger;
    uint8_t  happiness;
    uint8_t  discipline;
    uint8_t  hygiene;
    uint8_t  poop_count;
    uint8_t  graveyard;
    uint8_t  neglect_strikes;  // reset on stage transition
    uint8_t  pad;
    uint32_t lifetime_keys;
    uint32_t stage_keys;
} pet_state_t;

static pet_state_t pet;
static uint16_t    pet_persist_counter = 0;

static void pet_persist(void) {
    eeprom_update_block(&pet, PET_EEPROM_ADDR, sizeof(pet));
}

static void pet_hatch(void) {
    memset(&pet, 0, sizeof(pet));
    pet.magic      = PET_MAGIC;
    pet.stage      = PET_STAGE_EGG;
    pet.hunger     = 80;
    pet.happiness  = 80;
    pet.discipline = 80;
    pet.hygiene    = 80;
    pet_persist();
}

static void pet_init(void) {
    eeprom_read_block(&pet, PET_EEPROM_ADDR, sizeof(pet));
    if (pet.magic != PET_MAGIC) {
        pet_hatch();
    }
}

static void cap_add(uint8_t *m, uint8_t add) {
    uint16_t v = (uint16_t)*m + add;
    *m = (v > 100) ? 100 : (uint8_t)v;
}

static void pet_advance_stage(uint8_t next) {
    pet.stage           = next;
    pet.stage_keys      = 0;
    pet.neglect_strikes = 0;
    // Refill on growth as a "celebration"
    pet.hunger = pet.happiness = pet.discipline = pet.hygiene = 80;
    pet_persist();
}

static void pet_die(void) {
    if (pet.graveyard < 255) pet.graveyard++;
    uint8_t graveyard = pet.graveyard;
    memset(&pet, 0, sizeof(pet));
    pet.magic     = PET_MAGIC;
    pet.stage     = PET_STAGE_DEAD;
    pet.graveyard = graveyard;
    pet_persist();
}

static void pet_tick(void) {
    if (pet.stage == PET_STAGE_DEAD) {
        // Auto-rebirth after some keystrokes of mourning
        if (pet.lifetime_keys < 0xFFFFFFFFu) pet.lifetime_keys++;
        if (pet.lifetime_keys % 500 == 0) {
            uint8_t graveyard = pet.graveyard;
            pet_hatch();
            pet.graveyard = graveyard;
            pet_persist();
        }
        return;
    }

    if (pet.lifetime_keys < 0xFFFFFFFFu) pet.lifetime_keys++;
    if (pet.stage_keys < 0xFFFFFFFFu) pet.stage_keys++;

    // Decay schedule (each meter ticks down on its own cadence)
    if (pet.hunger     > 0 && (pet.stage_keys % 200) == 0) pet.hunger--;
    if (pet.happiness  > 0 && (pet.stage_keys % 300) == 0) pet.happiness--;
    if (pet.hygiene    > 0 && (pet.stage_keys % 500) == 0) pet.hygiene--;
    if (pet.discipline > 0 && (pet.stage_keys % 800) == 0) pet.discipline--;
    if (pet.poop_count < 3 && (pet.stage_keys % 1500) == 0) pet.poop_count++;

    // Neglect tracking — fires every 1000 keystrokes if any meter is at 0
    if ((pet.stage_keys % 1000) == 0 && pet.stage_keys > 0) {
        if (pet.hunger == 0 || pet.happiness == 0 || pet.hygiene == 0) {
            if (pet.neglect_strikes < 255) pet.neglect_strikes++;
        }
    }

    // Death from prolonged neglect
    if (pet.neglect_strikes >= 30) {
        pet_die();
        return;
    }

    // Stage transitions
    if (pet.stage == PET_STAGE_EGG && pet.stage_keys >= 1000) {
        pet_advance_stage(PET_STAGE_BABY);
    } else if (pet.stage == PET_STAGE_BABY && pet.stage_keys >= 5000) {
        pet_advance_stage(PET_STAGE_CHILD);
    } else if (pet.stage == PET_STAGE_CHILD && pet.stage_keys >= 15000) {
        pet_advance_stage(pet.neglect_strikes >= 5 ? PET_STAGE_TEEN_BAD : PET_STAGE_TEEN_GOOD);
    } else if ((pet.stage == PET_STAGE_TEEN_GOOD || pet.stage == PET_STAGE_TEEN_BAD)
               && pet.stage_keys >= 30000) {
        pet_advance_stage(pet.neglect_strikes >= 5 ? PET_STAGE_ADULT_BAD : PET_STAGE_ADULT_GOOD);
    }

    // Periodic flush so meter changes survive an unplug
    if (++pet_persist_counter >= 1000) {
        pet_persist_counter = 0;
        pet_persist();
    }
}

static void pet_feed(void)       { cap_add(&pet.hunger, 30);     pet_persist(); }
static void pet_play(void)       { cap_add(&pet.happiness, 30);  pet_persist(); }
static void pet_clean(void)      { pet.poop_count = 0; cap_add(&pet.hygiene, 30); pet_persist(); }
static void pet_discipline(void) { cap_add(&pet.discipline, 30); pet_persist(); }

void keyboard_post_init_user(void) {
    pet_init();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        pet_tick();
        switch (keycode) {
            case PET_FEED:  pet_feed();       return false;
            case PET_PLAY:  pet_play();       return false;
            case PET_CLEAN: pet_clean();      return false;
            case PET_DISC:  pet_discipline(); return false;
        }
    }
    return true;
}

#endif  // PET_ENABLE


// ---------------------------------------------------------------------------
// OLED rendering
// ---------------------------------------------------------------------------

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}

#    ifdef PET_ENABLE
// Rotate slave to match the vertical pet layout (32 wide × 128 tall).
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return is_keyboard_master() ? rotation : OLED_ROTATION_270;
}
#    endif

#    ifdef PET_ENABLE

// Pixel-double a 16x16 PROGMEM sprite to a 32x32 region at (x0, y0).
// Sprite format: 32 bytes, 2 pages × 16 cols, vertical-LSB-first.
static void draw_sprite_2x(const char *sprite, uint8_t x0, uint8_t y0) {
    for (uint8_t page = 0; page < 2; page++) {
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t b = pgm_read_byte(&sprite[page * 16 + col]);
            for (uint8_t bit = 0; bit < 8; bit++) {
                bool    on = (b >> bit) & 1;
                uint8_t dx = x0 + col * 2;
                uint8_t dy = y0 + (page * 8 + bit) * 2;
                oled_write_pixel(dx,     dy,     on);
                oled_write_pixel(dx + 1, dy,     on);
                oled_write_pixel(dx,     dy + 1, on);
                oled_write_pixel(dx + 1, dy + 1, on);
            }
        }
    }
}

// One 8-px-tall meter row: 1-char label + 24 px progress bar (0..100).
static void draw_meter(uint8_t row_y, char label, uint8_t level) {
    char s[2] = {label, 0};
    oled_set_cursor(0, row_y / 8);
    oled_write(s, false);

    // Bar body: 24 px wide × 4 px tall, x=8..31, y=row_y+2..row_y+5
    uint8_t fill = ((uint16_t)level * 24) / 100;
    if (fill > 24) fill = 24;
    for (uint8_t x = 0; x < 24; x++) {
        bool lit = x < fill;
        oled_write_pixel(8 + x, row_y + 3, lit);
        oled_write_pixel(8 + x, row_y + 4, lit);
    }
    // Top/bottom border
    for (uint8_t x = 0; x < 24; x++) {
        oled_write_pixel(8 + x, row_y + 1, true);
        oled_write_pixel(8 + x, row_y + 6, true);
    }
    // Left/right border
    oled_write_pixel(8,      row_y + 2, true);
    oled_write_pixel(8,      row_y + 5, true);
    oled_write_pixel(8 + 23, row_y + 2, true);
    oled_write_pixel(8 + 23, row_y + 5, true);
}

static const char PROGMEM stage_egg[]   = "EGG ";
static const char PROGMEM stage_baby[]  = "BABY";
static const char PROGMEM stage_chld[]  = "CHLD";
static const char PROGMEM stage_teng[]  = "TEEN";
static const char PROGMEM stage_tenb[]  = "T-X ";
static const char PROGMEM stage_adlg[]  = "ADLT";
static const char PROGMEM stage_adlb[]  = "A-X ";
static const char PROGMEM stage_dead[]  = "RIP ";

static const char *PROGMEM const stage_labels[] = {
    stage_egg, stage_baby, stage_chld, stage_teng,
    stage_tenb, stage_adlg, stage_adlb, stage_dead,
};

void oledkit_render_logo_user(void) {
    static uint16_t last_anim_change = 0;
    static bool     show_b           = false;
    if (timer_elapsed(last_anim_change) > 600) {
        last_anim_change = timer_read();
        show_b           = !show_b;
    }

    oled_clear();

    // 1) Sprite 32x32 at top
    pet_sprite_pair_t pair;
    uint8_t           stage = pet.stage > 7 ? 0 : pet.stage;
    memcpy_P(&pair, &PET_SPRITES[stage], sizeof(pair));
    draw_sprite_2x(show_b ? pair.b : pair.a, 0, 0);

    // 2) 4 meter bars stacked at rows 32, 40, 48, 56
    draw_meter(32, 'H', pet.hunger);
    draw_meter(40, 'F', pet.happiness);
    draw_meter(48, 'D', pet.discipline);
    draw_meter(56, 'C', pet.hygiene);

    // 3) Stage label at row 8 (pixel y=64)
    oled_set_cursor(0, 8);
    const char *label = (const char *)pgm_read_ptr(&stage_labels[stage]);
    oled_write_P(label, false);

    // 4) Poop counter and graveyard (avoid snprintf — pulls in ~1.5 KB of printf)
    if (pet.poop_count > 0) {
        char buf[4] = {'P', ':', (char)('0' + pet.poop_count), 0};
        oled_set_cursor(0, 9);
        oled_write(buf, false);
    }
    if (pet.graveyard > 0) {
        char    buf[5] = {'+', 0, 0, 0, 0};
        uint8_t v      = pet.graveyard;
        if (v >= 100) {
            buf[1] = '0' + v / 100;
            buf[2] = '0' + (v / 10) % 10;
            buf[3] = '0' + v % 10;
        } else if (v >= 10) {
            buf[1] = '0' + v / 10;
            buf[2] = '0' + v % 10;
        } else {
            buf[1] = '0' + v;
        }
        oled_set_cursor(0, 10);
        oled_write(buf, false);
    }
}

#    endif  // PET_ENABLE
#endif      // OLED_ENABLE
