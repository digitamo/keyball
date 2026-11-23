/*
Copyright 2021 @Yowkees
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

#include "lib/keyball/keyball.h"

//////////////////////////////////////////////////////////////////////////////

// clang-format off
matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b00011111,
    0b00011111,
    0b00011111,
    0b00111111,
    0b00011111,
    0b00011111,
    0b00011111,
    0b00111111,
};
// clang-format on

void keyball_on_adjust_layout(keyball_adjust_t v) {
#ifdef RGBLIGHT_ENABLE
    // rgblight_led_index() subtracts clipping_start_pos from every effect
    // index, so the effect range must start at the clipping start. Otherwise
    // the first indices underflow and ws2812_set_color() writes past
    // ws2812_leds[] into whatever follows it in RAM.
    uint8_t lednum_this = keyball.this_have_ball ? 22 : 24;
    uint8_t lednum_that = !keyball.that_enable ? 0 : keyball.that_have_ball ? 22 : 24;
    uint8_t start = is_keyboard_left() ? 0 : lednum_that;
    if (start >= RGBLIGHT_LED_COUNT) {
        return;
    }
    uint8_t count = lednum_this;
    if (start + count > RGBLIGHT_LED_COUNT) {
        count = RGBLIGHT_LED_COUNT - start;
    }
    rgblight_set_clipping_range(start, count);
    rgblight_set_effect_range(start, count);
#endif
}
