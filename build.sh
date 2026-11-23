#!/bin/bash
# Build script for keyball39 firmware
# Usage: ./build.sh [default|via] [output-dir]

set -e

QMK_REPO="${QMK_REPO:-$HOME/.cache/qmk_firmware_0.28.0}"
KEYMAP="${1:-via}"
OUTPUT_DIR="${2:-$(pwd)}"
# QMK lint treats a keymap literally named "via" as invalid, so the VIA build
# lives in keymaps/default_via. Keep "via" as the command-line name.
KEYMAP_DIR="$KEYMAP"
[ "$KEYMAP" = "via" ] && KEYMAP_DIR="default_via"
OUTPUT_FILE="keyball_keyball39_${KEYMAP_DIR}.hex"
SRC_DIR="$(pwd)"

# Toolchains: QMK prebuilt on Linux, keg-only Homebrew toolchains on macOS
if [ -d "$HOME/.cache/qmk_toolchains/qmk_toolchains_linuxX64/bin" ]; then
	export PATH="$HOME/.cache/qmk_toolchains/qmk_toolchains_linuxX64/bin:$PATH"
else
	export PATH="/opt/homebrew/opt/avr-gcc@8/bin:/opt/homebrew/opt/arm-none-eabi-gcc@8/bin:$PATH"
fi

echo "Building keyball39:$KEYMAP..."

# Check if QMK repo exists
if [ ! -d "$QMK_REPO" ]; then
	echo "Cloning QMK firmware..."
	git clone --depth 1 --recurse-submodules --shallow-submodules -b 0.28.0 \
		https://github.com/qmk/qmk_firmware.git "$QMK_REPO"
fi

if [ -d "$QMK_REPO/.git" ]; then
	QMK_VERSION="$(git -C "$QMK_REPO" describe --tags --always 2>/dev/null || true)"
	if [ "$QMK_VERSION" != "0.28.0" ]; then
		echo "warning: $QMK_REPO is $QMK_VERSION, expected 0.28.0" >&2
	fi
fi

# Link keyball keyboards. ln -sfn only replaces a symlink, so a real directory
# has to be refused instead of nesting the link inside it and building stale
# code from whatever was already there.
KEYBALL_LINK="$QMK_REPO/keyboards/keyball"
if [ -L "$KEYBALL_LINK" ]; then
	rm -f "$KEYBALL_LINK"
elif [ -e "$KEYBALL_LINK" ]; then
	echo "error: $KEYBALL_LINK exists and is not a symlink; remove it so this repo's keyboards are used" >&2
	exit 1
fi
ln -s "$SRC_DIR/qmk_firmware/keyboards/keyball" "$KEYBALL_LINK"

# Build
cd "$QMK_REPO"
make SKIP_GIT=yes keyball/keyball39:"$KEYMAP_DIR"

# Copy output to target directory
cp ".build/$OUTPUT_FILE" "$OUTPUT_DIR/"

echo ""
echo "Built: $OUTPUT_DIR/$OUTPUT_FILE"
