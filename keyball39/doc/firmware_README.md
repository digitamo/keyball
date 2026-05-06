# Keyball39 firmwares README

## Description in English

This archive includes firmwares for Keyball39.

Included variations of firmware are below:

* `default` - base version for customization
* `test` - used for testing during assembly
* `via` - Recommended version that can use [Remap][remap]

The recommended usage of these firmwares is:

1.  Please use `test` version during assemble the keyboard.
    It is designed to help verifying that the soldering of diodes and switch
    sockets is correct, and easy to check the operation of RGB LED.
2.  After confirming the operation, rewrite it with `via` version.

## 日本語の説明

このアーカイブには Keyball39 のファームウェアが含まれています。

含まれているファームウェアのバリエーションは以下の通りです:

* `default` - カスタマイズの基礎となる版
* `test` - 組立時のテストに用いる版
* `via` - [Remap][remap] で利用できる、推奨版

推奨する使い方は次の通りです:

1.  キーボードを組み立てる際は `test` を使ってください。
    ダイオードやスイッチソケットのハンダ付けが正しいか確認できる他、
    RGB LEDの動作確認が容易になるように構成してあります。
2.  動作確認が取れたあとは `via` に書き換えて利用してください。

## Fixed keymap for `test`

This keymap does not use any layers or QMK function keys to make it easier to test.

```
KC_Q     KC_W     KC_E     KC_R     KC_T                            KC_Y     KC_U     KC_I     KC_O     KC_P
KC_A     KC_S     KC_D     KC_F     KC_G                            KC_H     KC_J     KC_K     KC_L     KC_SCLN
KC_Z     KC_X     KC_C     KC_V     KC_B                            KC_N     KC_M     KC_COMM  KC_DOT   KC_SLSH
KC_LCTL (KC_LGUI  KC_LALT  KC_ESC)  KC_SPC   KC_TAB        KC_BSPC  KC_ENT  (KC_ESC   KC_RALT  KC_RGUI) KC_RSFT
```

The area enclosed in parentheses may be replaced by a trackball.

<https://config.qmk.fm/#/test> is useful for testing keys.

[remap]:https://remap-keys.app/

## Build Guide

### Prerequisites

- QMK 0.24.0 (verified working)
- For other QMK versions, you may need to change `static const char BL` to `enum { BL }` in `keyball.c` for GCC 12+ compatibility

### Build Steps

```bash
# 1. Clone QMK firmware
git clone --depth 1 --recurse-submodules --shallow-submodules -b 0.24.0 \
  https://github.com/qmk/qmk_firmware.git /tmp/qmk

# 2. Link keyball keyboards
ln -sf /path/to/keyball/qmk_firmware/keyboards/keyball /tmp/qmk/keyboards/keyball

# 3. Build
cd /tmp/qmk && make SKIP_GIT=yes keyball/keyball39:default
```

### Troubleshooting

#### Reversed sides (left/right swapped)
If the keyboard halves are reversed (e.g., left half acts as right), add to `keyball39/config.h`:
```c
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_LEFT
```

This fixes the side detection when using `SPLIT_HAND_MATRIX_GRID` for handedness detection.

## Tamagotchi pet (default keymap only)

The `default` build ships with a small virtual pet that lives on the slave
half's OLED. Keep typing, take care of it, and watch it grow. The `via` build
doesn't include the pet — flash space is too tight when VIA is on.

### Where to look

The pet renders on whichever half you *don't* have plugged into USB (the
slave). Plug into the right side and the pet shows up on the left's OLED, and
vice versa. The plugged-in side keeps showing the usual key/ball/layer info.

You'll see, top to bottom:

```
 ┌────────┐
 │ sprite │   ← your pet, ~32 px tall
 ├────────┤
 │H ▓▓▓░░ │   Hunger
 │F ▓▓▓▓▓ │   Fun (happiness)
 │D ▓░░░░ │   Discipline
 │C ▓▓▓▓░ │   Cleanliness
 ├────────┤
 │ STAGE  │   Stage label
 │ P:n    │   Poop pile (only when needed)
 │ +n     │   Memorial count (only after misfortunes)
 └────────┘
```

### Caring for it

Hold the layer-3 thumb key (the one you use for keyboard config) and tap one
of the care keys on the right hand:

| Key on layer 3 (right hand) | What it does                              |
|-----------------------------|-------------------------------------------|
| `U` position — `PET_FEED`   | Feeds the pet — Hunger goes up            |
| `I` position — `PET_PLAY`   | Plays with the pet — Fun goes up          |
| `J` position — `PET_CLEAN`  | Tidies up — Cleanliness goes up           |
| `K` position — `PET_DISC`   | Disciplines — Discipline goes up          |

Each care action gives a generous boost; you don't need to mash the key.

### How time passes

There's no real-time clock on the keyboard, so the pet's life ticks forward
based on how much you type. Light use day = slow life. Heavy use day = the pet
grows up faster. The four meters drift downward as you keep typing, and care
keys top them back up.

A few rules of thumb without spoiling the surprises:
- A freshly hatched pet starts as an egg. Keep typing and it'll hatch.
- The shape of your pet at later stages depends on how well you've been
  caring for it along the way. Different care patterns produce different
  outcomes, so don't expect the same result twice.
- If a meter sits empty for a long stretch, the pet notices.

### Persistence

The pet's state is saved to the keyboard's EEPROM, so unplugging or rebooting
won't reset it. Stats pick up where you left off.

### Starting fresh

There's no dedicated "reset pet" key right now. If you really want a new egg,
the cleanest options are:
- Re-flash the firmware after wiping EEPROM (QMK's `EE_CLR` keycode, if you
  bind it temporarily, or a fresh flash with the magic byte changed).
- Wait it out. Every pet's story eventually ends, and the keyboard hatches a
  fresh egg shortly after.
