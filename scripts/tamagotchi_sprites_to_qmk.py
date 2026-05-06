#!/usr/bin/env python3
"""Convert Spriters-Resource Tamagotchi P1/P2 sprite sheets into QMK PROGMEM
byte arrays in vertical-LSB-first page-major (`oled_write_raw_P`) format.

Input: PNG sheets under $TMPDIR/tama_sprites/ (downloaded by hand from
https://www.spriters-resource.com/lcd_handhelds/tamagotchioriginalp1p2/asset/<id>/).

Output: a single C header `qmk_firmware/keyboards/keyball/keyball39/keymaps/
default/pet_sprites.h` containing one `static const char PROGMEM` array per
frame, plus an enum naming each sprite slot.

The Tamagotchi LCD pixels are typically 12-15 px wide on the source sheets;
we center each sprite in a 16x16 canvas. The renderer pixel-doubles to 32x32
at draw time.
"""
from __future__ import annotations

import os
import sys
from dataclasses import dataclass
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install pillow")


SPRITE_DIR = Path(os.environ.get("TMPDIR", "/tmp")) / "tama_sprites"
OUT_PATH = Path(__file__).resolve().parent.parent / (
    "qmk_firmware/keyboards/keyball/keyball39/keymaps/default/pet_sprites.h"
)
SPRITE_W = 16
SPRITE_H = 16
WHITE_THRESHOLD = 240  # pixels brighter than this in any channel are "background"


@dataclass
class FrameSpec:
    """One sprite frame: which sheet, which cell index in the top band."""
    name: str           # e.g. "MAMETCHI_A"
    sheet: str          # PNG basename in SPRITE_DIR
    cell_index: int     # 0-based cell index within the top horizontal band

    # Optional manual-crop fallback for sprites that don't auto-detect cleanly
    # (general-sprites overlays). If set, sheet/cell_index are ignored.
    manual_crop: tuple[int, int, int, int] | None = None  # (x, y, w, h)


# 4 stages × 2 variants = 8 character sprites, plus egg + state overlays.
# Frame ordering: each character contributes 2 idle frames (A, B) from the
# first two cells of its top band — that's the canonical Tamagotchi idle bob.
CHARACTERS: list[tuple[str, str]] = [
    # (name_prefix, source_png_id)
    ("BABY",      "144010.png"),  # Babytchi
    ("CHILD",     "144012.png"),  # Marutchi
    ("TEEN_GOOD", "144014.png"),  # Tamatchi
    ("TEEN_BAD",  "144015.png"),  # Tongaritchi
    ("ADULT_GOOD","144016.png"),  # Mametchi
    ("ADULT_BAD", "144052.png"),  # Pochitchi
]

# Manual crops for overlays + egg. Coordinates in pixels on 144401.png
# (General Sprites Simplified). These will be tuned by inspection.
OVERLAYS: list[FrameSpec] = [
    # Slot, sheet (unused for manual_crop), cell_index (unused), manual_crop
    FrameSpec("EGG_A",   "144401.png", 0, manual_crop=None),  # filled later
    FrameSpec("EGG_B",   "144401.png", 0, manual_crop=None),
    FrameSpec("DEAD",    "144401.png", 0, manual_crop=None),
    FrameSpec("SICK",    "144401.png", 0, manual_crop=None),
    FrameSpec("SLEEP",   "144401.png", 0, manual_crop=None),
    FrameSpec("EAT",     "144401.png", 0, manual_crop=None),
]


def find_top_band(img: Image.Image) -> tuple[int, int]:
    """Return (y0, y1) of the first non-white horizontal band."""
    px = img.load()
    h = img.height
    band: list[int] = []
    for y in range(h):
        has = False
        for x in range(img.width):
            r, g, b = px[x, y][:3]
            a = px[x, y][3] if img.mode == "RGBA" else 255
            if a > 0 and (r < WHITE_THRESHOLD or g < WHITE_THRESHOLD or b < WHITE_THRESHOLD):
                has = True
                break
        if has:
            band.append(y)
        elif band:
            break
    if not band:
        raise RuntimeError("no content found")
    return band[0], band[-1]


def find_cells_in_band(img: Image.Image, y0: int, y1: int) -> list[tuple[int, int]]:
    """Return (x0, x1) for each cell separated by at least one fully-white column."""
    px = img.load()
    w = img.width
    col_has: list[bool] = []
    for x in range(w):
        has = False
        for y in range(y0, y1 + 1):
            r, g, b = px[x, y][:3]
            a = px[x, y][3] if img.mode == "RGBA" else 255
            if a > 0 and (r < WHITE_THRESHOLD or g < WHITE_THRESHOLD or b < WHITE_THRESHOLD):
                has = True
                break
        col_has.append(has)
    cells = []
    in_run = False
    start = 0
    for x, h in enumerate(col_has):
        if h and not in_run:
            in_run = True
            start = x
        elif not h and in_run:
            in_run = False
            cells.append((start, x - 1))
    if in_run:
        cells.append((start, w - 1))
    return cells


def extract_cell_to_canvas(
    img: Image.Image, x0: int, y0: int, x1: int, y1: int
) -> list[list[int]]:
    """Extract pixels from rect, threshold to 1-bit, center in SPRITE_W×SPRITE_H grid.

    Returns grid[y][x] = 1 (lit) or 0 (background). LCD-dark pixels become lit.
    """
    px = img.load()
    src_w = x1 - x0 + 1
    src_h = y1 - y0 + 1
    if src_w > SPRITE_W or src_h > SPRITE_H:
        # Source is larger than canvas — scale down proportionally
        crop = img.crop((x0, y0, x1 + 1, y1 + 1))
        scale = min(SPRITE_W / src_w, SPRITE_H / src_h)
        new_w = max(1, int(src_w * scale))
        new_h = max(1, int(src_h * scale))
        crop = crop.resize((new_w, new_h), Image.LANCZOS)
        src_w, src_h = new_w, new_h
        sx0, sy0 = 0, 0
        sx1, sy1 = new_w - 1, new_h - 1
        spx = crop.load()
    else:
        spx = px
        sx0, sy0 = x0, y0
        sx1, sy1 = x1, y1

    pad_x = (SPRITE_W - src_w) // 2
    pad_y = (SPRITE_H - src_h) // 2

    grid = [[0] * SPRITE_W for _ in range(SPRITE_H)]
    for sy in range(src_h):
        for sx in range(src_w):
            r, g, b = spx[sx0 + sx, sy0 + sy][:3]
            mode = img.mode if spx is px else "RGB"
            a = spx[sx0 + sx, sy0 + sy][3] if mode == "RGBA" else 255
            if a == 0:
                continue
            # LCD-dark = lit pixel for OLED (we want the figure visible).
            brightness = (r + g + b) / 3
            if brightness < WHITE_THRESHOLD:
                ty = sy + pad_y
                tx = sx + pad_x
                if 0 <= ty < SPRITE_H and 0 <= tx < SPRITE_W:
                    grid[ty][tx] = 1
    return grid


def grid_to_qmk_bytes(grid: list[list[int]]) -> bytes:
    """Convert 16x16 bool grid to QMK page-major vertical-LSB-first bytes.

    Format: 2 pages × 16 columns = 32 bytes. Each byte represents 8 vertical
    pixels with LSB at top.
    """
    pages = SPRITE_H // 8  # 2
    out = bytearray(pages * SPRITE_W)
    for page in range(pages):
        for col in range(SPRITE_W):
            b = 0
            for bit in range(8):
                y = page * 8 + bit
                if grid[y][col]:
                    b |= 1 << bit
            out[page * SPRITE_W + col] = b
    return bytes(out)


def render_grid_ascii(grid: list[list[int]]) -> str:
    """For visual debugging — print the sprite as ASCII."""
    return "\n".join(
        "".join("#" if v else "." for v in row) for row in grid
    )


def format_c_array(name: str, data: bytes) -> str:
    lines = [f"static const char PROGMEM {name}[] = {{"]
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i:i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    return "\n".join(lines)


def main(verbose: bool = True) -> None:
    if not SPRITE_DIR.exists():
        sys.exit(f"sprite dir missing: {SPRITE_DIR}")

    arrays: list[tuple[str, bytes]] = []

    for prefix, png in CHARACTERS:
        path = SPRITE_DIR / png
        if not path.exists():
            sys.exit(f"missing sprite sheet: {path}")
        img = Image.open(path).convert("RGBA")
        y0, y1 = find_top_band(img)
        cells = find_cells_in_band(img, y0, y1)
        if len(cells) < 2:
            sys.exit(f"{png}: only {len(cells)} cells found, need >= 2")
        if verbose:
            print(f"{png}: top band y={y0}..{y1}, {len(cells)} cells", file=sys.stderr)
        for frame_i, suffix in enumerate("AB"):
            x0, x1 = cells[frame_i]
            grid = extract_cell_to_canvas(img, x0, y0, x1, y1)
            if verbose:
                print(f"\n{prefix}_{suffix} (cell {frame_i}, src {x1-x0+1}x{y1-y0+1}):", file=sys.stderr)
                print(render_grid_ascii(grid), file=sys.stderr)
            data = grid_to_qmk_bytes(grid)
            arrays.append((f"PET_SPRITE_{prefix}_{suffix}", data))

    # Header file
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    with OUT_PATH.open("w") as f:
        f.write("// Auto-generated by scripts/tamagotchi_sprites_to_qmk.py\n")
        f.write("// Source: https://www.spriters-resource.com/lcd_handhelds/tamagotchioriginalp1p2/\n")
        f.write("// 16x16 sprites in QMK page-major vertical-LSB-first format (32 bytes each)\n")
        f.write("#pragma once\n#include <avr/pgmspace.h>\n\n")
        for name, data in arrays:
            f.write(format_c_array(name, data))
            f.write("\n\n")
    if verbose:
        print(f"\nwrote {len(arrays)} sprites to {OUT_PATH}", file=sys.stderr)


if __name__ == "__main__":
    main()
