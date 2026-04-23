# Keyball39

![Keyball39](../../../../keyball39/doc/rev1/images/kb39_001.jpg)

A split keyboard with 39 vertically staggered keys and 34mm track ball.

* Keyboard Maintainer: [@Yowkees](https://twitter.com/Yowkees)
* Hardware Supported: Keyball39 PCB, ProMicro
* Hardware Availability:
    * <https://shop.yushakobo.jp/products/5357>
    * <https://shirogane-lab.net/items/64b8f8693ee3fd0045280190>

## Build Firmware

### Prerequisites

Install the AVR toolchain:

```bash
sudo apt install gcc-avr avr-libc avrdude
```

Install QMK CLI:

```bash
pip install qmk
```

### Clone and Setup QMK Firmware

1. Clone the QMK firmware repository:

```bash
git clone --recurse-submodules https://github.com/qmk/qmk_firmware.git qmk_firmware
cd qmk_firmware
```

2. Checkout a compatible QMK version (0.25.x or later):

```bash
git checkout 0.25.22
```

3. Copy the keyball keyboard files:

```bash
cp -r /path/to/keyball/qmk_firmware/keyboards/keyball keyboards/
```

### Build

From the QMK firmware directory:

```bash
make keyball/keyball39:default
```

Available keymaps:
- `default` - Default keymap
- `test` - Test keymap
- `via` - VIA-enabled keymap
- `develop` - Development keymap

### Flash the Firmware

1. Put the ProMicro in bootloader mode (double-tap the reset button, or short RST and GND twice within 1 second)

2. Flash with avrdude:

```bash
avrdude -c atmel32u4 -p atmega32u4 -P /dev/ttyACM0 -U flash:w:keyball_keyball39_default.hex
```

Or use QMK's built-in flash command:

```bash
make keyball/keyball39:default:flash
```

### Troubleshooting

**Error: `avr-gcc: not found`**
- Install the AVR toolchain: `sudo apt install gcc-avr avr-libc avrdude`

**Error: `initializer element is not constant`**
- This is a known compatibility issue with newer GCC versions. Edit `keyboards/keyball/lib/keyball/keyball.c` and change:
  ```c
  .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
  ```
  to:
  ```c
  .pressing_keys = { '\xB0', '\xB0', '\xB0', '\xB0', '\xB0', '\xB0', 0 },
  ```

**Error: `No rule to make target`**
- Ensure you're in the QMK firmware directory
- Ensure the keyball keyboard files are in `keyboards/keyball/`
- Verify QMK version is compatible: `git checkout 0.25.22`

## Special keycodes

See [Special Keycode](../lib/keyball/keycodes.md) file.
