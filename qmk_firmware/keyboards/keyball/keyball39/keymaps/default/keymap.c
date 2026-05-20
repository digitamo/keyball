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
    S(KC_6)     , S(KC_7)      , S(KC_8)     , S(KC_9)     , S(KC_0)     ,                                            KC_LBRC     , S(KC_4)     , KC_PGDN     , KC_BTN3     , KC_BSLS        ,
    KC_INT1     , KC_EQL      , S(KC_3)     , KC_TRNS     , KC_TRNS     , KC_TRNS      ,        TO(2)   , TO(0)        , KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_F12
  ),

  // Layer 2 — navigation, numpad, media
  [2] = LAYOUT_universal(
    KC_VOLU     , KC_HOME     , KC_UP       , KC_END      , S(KC_LBRC)  ,                                            S(KC_RBRC)  , KC_P7       , KC_P8       , KC_P9       , KC_PPLS        ,
    KC_VOLD     , KC_LEFT     , KC_DOWN     , KC_RGHT     , KC_LBRC     ,                                            KC_RBRC     , KC_P4       , KC_P5       , KC_P6       , KC_PMNS        ,
    KC_MUTE     , KC_PGUP     , KC_CAPS     , KC_PGDN     , S(KC_9)     ,                                            S(KC_0)     , KC_P1       , KC_P2       , KC_P3       , KC_PAST        ,
    KC_MPRV     , KC_MPLY     , KC_MNXT     , KC_TRNS     , KC_TRNS     , KC_TRNS      ,        KC_NUM  , KC_P0        , KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_EQL
  ),

  // Layer 3 — keyboard config + RGB
  [3] = LAYOUT_universal(
    RGB_TOG     , AML_TO      , AML_I50     , AML_D50     , KC_TRNS     ,                                            RGB_M_P     , RGB_M_B     , RGB_M_R     , RGB_M_SW    , RGB_M_SN       ,
    RGB_MOD     , RGB_HUI     , RGB_SAI     , RGB_VAI     , SCRL_DVD    ,                                            RGB_M_K     , RGB_M_X     , RGB_M_G     , RGB_M_T     , RGB_M_TW       ,
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

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}

// Luna 2-frame idle sit, ported from filterpaper's QMK Luna animation
// (github.com/filterpaper/qmk_oled_animations/blob/main/oled_luna.c, GPL-2.0+),
// itself derived from HellSingCoder's original Luna for the Sofle.
// Frames are 32x24 (96 bytes) oriented for OLED_ROTATION_270 — see the
// oled_init_user override below that switches the slave to that rotation.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return is_keyboard_master() ? rotation : OLED_ROTATION_270;
}

void oledkit_render_logo_user(void) {
    static const char PROGMEM frame_a[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x1C,
        0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xA9, 0x1E, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x08, 0x68, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x82, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0C, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28,
        0x3E, 0x1C, 0x20, 0x20, 0x3E, 0x0F, 0x11, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    static const char PROGMEM frame_b[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x1C,
        0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xA9, 0x1E, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x90, 0x08, 0x18, 0x60, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0E, 0x82, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0C, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28,
        0x3E, 0x1C, 0x20, 0x20, 0x3E, 0x0F, 0x11, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    static uint16_t last_frame_change = 0;
    static bool show_b = false;
    if (timer_elapsed(last_frame_change) > 600) {
        last_frame_change = timer_read();
        show_b = !show_b;
    }
    // Slave OLED is rotated 270, so the buffer is page-major in display
    // coords: 32 cols × 16 pages = 512 bytes. The frame is 32×24 (3 pages),
    // so byte offset 13 × 32 = 416 places it at rows 104..127 (bottom).
    const char *frame = show_b ? frame_b : frame_a;
    for (uint16_t i = 0; i < sizeof(frame_a); i++) {
        oled_write_raw_byte(pgm_read_byte(&frame[i]), 416 + i);
    }
}
#endif
