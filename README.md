# Keyball Series

![Keyball61](./keyball61/doc/rev1/images/kb61_001.jpg)

Keyball series is keyboard family which have 100% track ball.

Keyboards in the family are:

* Available
    * Keyball39: split + 39 keys + a track ball
    * Keyball44: split + 44 keys + a track ball
    * Keyball61: split + 61 keys + a track ball
* Unavailable
    * Keyball46 (first one!)
    * One47

## Where to Buy

|Keyboard   |Shirogane Lab / 白銀ラボ                                   |Yushakobo / 遊舎工房                       |
|-----------|-------------------------------------------|-----------------------------------------------------------|
|Keyball39  |<https://shiroganelab.com/products/keyball39> |<https://shop.yushakobo.jp/products/5357>  |
|Keyball44  |<https://shiroganelab.com/products/keyball44> |<https://shop.yushakobo.jp/products/8337>  |
|Keyball61  |<https://shiroganelab.com/products/keyball61> |<https://shop.yushakobo.jp/products/5358>  |

## Build Guide

*   Keyball39:
    [English/英語](/keyball39/doc/rev1/buildguide_en.md),
    [日本語/Japanese](./keyball39/doc/rev1/buildguide_jp.md)
*   Keyball44:
    [English/英語](./keyball44/doc/rev1/buildguide_en.md),
    [日本語/Japanese](./keyball44/doc/rev1/buildguide_jp.md)
*   Keyball61:
    [English/英語](./keyball61/doc/rev1/buildguide_en.md),
    [日本語/Japanese](./keyball61/doc/rev1/buildguide_jp.md)

## Firmware

See [document for firmware source code](./qmk_firmware/keyboards/keyball/readme.md).

### Pre-compiled Firmwares

(TO BE DOCUMENTED)

### Flashing (Keyball39, WSL + Windows host)

This machine is WSL2, so flashing happens on the Windows side with QMK Toolbox.

1. Build the firmware from WSL:

    ```sh
    ./build.sh via
    ```

    This produces `keyball_keyball39_via.hex` in the repo root.

2. Copy the hex to the Windows host, keeping the version suffix used in past flashes:

    ```sh
    cp keyball_keyball39_via.hex /mnt/c/Users/osama/Downloads/keyball_keyball39_via_v7.hex
    ```

3. On Windows, open QMK Toolbox (`C:\Program Files\QMK Toolbox\qmk_toolbox.exe`),
   open the hex, and set MCU to `atmega32u4` with flash method `AVR109 (Caterina)`.

4. Plug in one half and put its ProMicro in bootloader mode: double-tap the
   reset switch soldered on that half's PCB (or short RST and GND on the
   ProMicro twice). A new `Arduino Micro` COM port appears, then click Flash.

5. Repeat for the other half. Both halves need the same firmware.
