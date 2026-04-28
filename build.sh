#!/bin/bash
# Build script for keyball39 firmware
# Usage: ./build.sh [default|via]

set -e

QMK_REPO="${QMK_REPO:-/tmp/qmk}"
KEYMAP="${1:-via}"
OUTPUT_FILE="keyball_keyball39_${KEYMAP}.hex"

echo "Building keyball39:$KEYMAP..."

# Check if QMK repo exists
if [ ! -d "$QMK_REPO" ]; then
	echo "Cloning QMK firmware..."
	git clone --depth 1 --recurse-submodules --shallow-submodules -b 0.24.0 \
		https://github.com/qmk/qmk_firmware.git "$QMK_REPO"

	# Link keyball keyboards
	ln -sf "$(pwd)/qmk_firmware/keyboards/keyball" "$QMK_REPO/keyboards/keyball"
fi

# Build
cd "$QMK_REPO"
make SKIP_GIT=yes keyball/keyball39:"$KEYMAP"

# Copy output to local folder
cp .build/"$OUTPUT_FILE" "$(pwd)/"

echo ""
echo "Built: $(pwd)/$OUTPUT_FILE"
