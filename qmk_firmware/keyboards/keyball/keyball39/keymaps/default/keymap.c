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

// Rows 0-3 are the left half, rows 4-7 the right half. Thumb keys are
// exempt from the opposite-hands rule.
char chordal_hold_handedness(keypos_t key) {
    if (key.row == 3) {
        return key.col < 3 ? 'L' : '*';
    }
    if (key.row == 7) {
        return key.col < 3 ? 'R' : '*';
    }
    return key.row < 4 ? 'L' : 'R';
}

// Mod-taps settle as held on a nested tap; space rolls keep default timing.
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    return IS_QK_MOD_TAP(keycode);
}

// The right thumb activates the mouse layer as soon as another key is pressed.
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    return keycode == LT(1, KC_BSPC);
}

// Protect same-hand home-row rolls. Other chords use normal hold settlement.
// This must stay below TAPPING_TERM, when Chordal Hold stops applying.
#define SAME_HAND_HOLD_MS 120

_Static_assert(SAME_HAND_HOLD_MS < TAPPING_TERM, "SAME_HAND_HOLD_MS must stay below TAPPING_TERM");

bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t *tap_hold_record, uint16_t other_keycode, keyrecord_t *other_record) {
    if (tap_hold_record->event.type != KEY_EVENT || other_record->event.type != KEY_EVENT) {
        return true;
    }
    const char tap_hand   = chordal_hold_handedness(tap_hold_record->event.key);
    const char other_hand = chordal_hold_handedness(other_record->event.key);
    if (tap_hand == '*' || other_hand == '*' || tap_hand != other_hand) {
        return true;
    }
    const uint8_t home_row = tap_hand == 'L' ? 1 : 5;
    if (tap_hold_record->event.key.row != home_row || other_record->event.key.row != home_row) {
        return true;
    }
    const int16_t dt = (int16_t)(other_record->event.time - tap_hold_record->event.time);
    return dt < 0 || dt >= SAME_HAND_HOLD_MS;
}

#ifdef RGBLIGHT_ENABLE
// QMK 0.28 dropped the handler for the per-mode RGB keycodes. Map the
// contiguous keycode range back onto the effect enum. Effects this keymap
// does not compile in fall back to static light.
static const uint8_t PROGMEM rgb_mode_target[] = {
    RGBLIGHT_MODE_STATIC_LIGHT,
#ifdef RGBLIGHT_EFFECT_BREATHING
    RGBLIGHT_MODE_BREATHING,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_RAINBOW_MOOD
    RGBLIGHT_MODE_RAINBOW_MOOD,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_RAINBOW_SWIRL
    RGBLIGHT_MODE_RAINBOW_SWIRL,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_SNAKE
    RGBLIGHT_MODE_SNAKE,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_KNIGHT
    RGBLIGHT_MODE_KNIGHT,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_CHRISTMAS
    RGBLIGHT_MODE_CHRISTMAS,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_STATIC_GRADIENT
    RGBLIGHT_MODE_STATIC_GRADIENT,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_RGB_TEST
    RGBLIGHT_MODE_RGB_TEST,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
#ifdef RGBLIGHT_EFFECT_TWINKLE
    RGBLIGHT_MODE_TWINKLE,
#else
    RGBLIGHT_MODE_STATIC_LIGHT,
#endif
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (IS_RGB_KEYCODE(keycode)) {
        if (record->event.pressed) {
            // _noeeprom: the EEPROM write blocks the split link for ~10 ms.
            rgblight_mode_noeeprom(pgm_read_byte(&rgb_mode_target[keycode - RGB_MODE_PLAIN]));
        }
        return false;
    }
    return true;
}
#endif

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
    S(KC_1)     , S(KC_2)     , S(KC_3)     , S(KC_4)     , S(KC_5)     ,                                            S(KC_GRV)   , KC_BTN1     , KC_BTN3     , KC_BTN2     , KC_F11         ,
    S(KC_6)     , S(KC_7)     , S(KC_8)     , S(KC_9)     , S(KC_0)     ,                                            KC_LBRC     , KC_BTN4     , KC_BTN5     , S(KC_BSLS)  , KC_BSLS        ,
    KC_INT1     , KC_EQL      , S(KC_3)     , KC_TRNS     , KC_TRNS     , KC_TRNS      ,        TO(2)   , TO(0)        , KC_TRNS    , KC_TRNS     , KC_TRNS     , KC_F12
  ),

  // Layer 2 — navigation, numpad, media
  [2] = LAYOUT_universal(
    KC_VOLU     , KC_HOME     , KC_UP       , KC_END      , S(KC_LBRC)  ,                                            S(KC_RBRC)  , KC_P7       , KC_P8       , KC_P9       , KC_PPLS        ,
    KC_VOLD     , KC_LEFT     , KC_DOWN     , KC_RGHT     , KC_LBRC     ,                                            KC_RBRC     , KC_P4       , KC_P5       , KC_P6       , KC_MINS        ,
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

// Both halves render the same status screen. The slave never processes key
// records, so its key line stays at R0 C0 K00.
bool oled_task_user(void) {
    oledkit_render_info_user();
    return true;
}
#endif
