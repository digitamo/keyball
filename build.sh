#!/bin/bash
# Build script for keyball39 firmware
# Usage: ./build.sh [default|via] [output-dir]

set -e

QMK_REPO="${QMK_REPO:-$HOME/.cache/qmk_firmware}"
KEYMAP="${1:-via}"
OUTPUT_DIR="${2:-$(pwd)}"
OUTPUT_FILE="keyball_keyball39_${KEYMAP}.hex"
SRC_DIR="$(pwd)"

# Keg-only Homebrew toolchains required by QMK
export PATH="/opt/homebrew/opt/avr-gcc@8/bin:/opt/homebrew/opt/arm-none-eabi-gcc@8/bin:$PATH"

echo "Building keyball39:$KEYMAP..."

# Check if QMK repo exists
if [ ! -d "$QMK_REPO" ]; then
	echo "Cloning QMK firmware..."
	git clone --depth 1 --recurse-submodules --shallow-submodules -b 0.24.0 \
		https://github.com/qmk/qmk_firmware.git "$QMK_REPO"

	# Link keyball keyboards
	ln -sf "$SRC_DIR/qmk_firmware/keyboards/keyball" "$QMK_REPO/keyboards/keyball"
fi

# Build
cd "$QMK_REPO"
make SKIP_GIT=yes keyball/keyball39:"$KEYMAP"

# Copy output to target directory
cp ".build/$OUTPUT_FILE" "$OUTPUT_DIR/"

echo ""
echo "Built: $OUTPUT_DIR/$OUTPUT_FILE"
