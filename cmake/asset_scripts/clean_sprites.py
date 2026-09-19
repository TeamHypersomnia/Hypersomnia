#!/usr/bin/env python3
"""
Clean the RGB garbage hiding under fully transparent texels of PNG sprites.

Usage: clean_sprites.py [paths...] [--dry-run] [--radius N] [--quiet]

The engine blends with straight (non-premultiplied) alpha:

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE)

so with GL_LINEAR filtering the GPU interpolates RGB and A independently.
The RGB of a texel with alpha == 0 is therefore NOT invisible: it gets averaged
into every sample that straddles the sprite's edge. If that RGB is leftover
garbage from the painting process, the sprite grows a halo of that garbage
colour - and since the colour itself is wrong, stacking several such sprites
converges to the garbage colour instead of to the intended one.

This script rewrites the RGB of each fully transparent texel that borders a
visible one, setting it to the alpha-weighted average of its visible neighbours.
Alpha is never touched, so the sprite is pixel-identical in any editor and
identical under GL_NEAREST; only the bilinear edge changes.

Only a 1-texel ring can ever be sampled by bilinear filtering (a 2x2 fetch),
which is why --radius defaults to 1. Files that are already clean are left
untouched on disk.
"""

import argparse
import io
import os
import sys

import numpy as np
from PIL import Image

# "dls" holds arenas downloaded from servers - they are hash-verified against
# the host, so rewriting a single byte there would break joining that server.
EXCLUDED_DIRS = {"3rdparty", "build", "cache", ".git", "node_modules", "dls"}

DEFAULT_PATHS = ["hypersomnia/content"]

NEIGHBOUR_OFFSETS = [
    (-1, -1), (-1, 0), (-1, 1),
    (0, -1), (0, 1),
    (1, -1), (1, 0), (1, 1),
]


def collect_pngs(roots):
    result = []

    for root in roots:
        if os.path.isfile(root):
            result.append(root)
            continue

        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in EXCLUDED_DIRS]

            for name in sorted(filenames):
                if name.lower().endswith(".png"):
                    result.append(os.path.join(dirpath, name))

    return sorted(set(result))


def shift(array, dy, dx, fill=0):
    """Shift an array by (dy, dx), padding the exposed border with fill."""
    result = np.full_like(array, fill)

    ys_dst = slice(max(dy, 0), array.shape[0] + min(dy, 0))
    xs_dst = slice(max(dx, 0), array.shape[1] + min(dx, 0))
    ys_src = slice(max(-dy, 0), array.shape[0] + min(-dy, 0))
    xs_src = slice(max(-dx, 0), array.shape[1] + min(-dx, 0))

    result[ys_dst, xs_dst] = array[ys_src, xs_src]
    return result


def solidified_rgb(rgb, alpha, radius):
    """
    Return the RGB channels with transparent texels within `radius` of a visible
    texel replaced by the colour of their most opaque visible neighbour.

    The colour is copied verbatim rather than averaged. An averaged fill would
    invent colours the sprite never contained - enough of them to push a
    palettised sprite out of its palette - and it would buy nothing: the texel is
    invisible, and the bilinear filter only ever reads it at partial weight.

    Ties are broken by NEIGHBOUR_OFFSETS order, and a texel filled by an earlier
    ring always loses to a genuinely visible one, so the result is deterministic.

    Returns (rgb, filled), where `filled` marks the transparent texels that were
    actually given a neighbour's colour.
    """
    result = rgb.copy()
    visible = alpha > 0
    known = visible.copy()
    score = np.where(known, alpha.astype(np.int16), np.int16(-1))

    for _ in range(radius):
        if known.all():
            break

        best_score = np.full(alpha.shape, -1, dtype=np.int16)
        best_rgb = np.zeros_like(result)

        for dy, dx in NEIGHBOUR_OFFSETS:
            neighbour_score = shift(score, dy, dx, -1)
            better = neighbour_score > best_score

            best_rgb[better] = shift(result, dy, dx)[better]
            best_score = np.where(better, neighbour_score, best_score)

        fillable = (~known) & (best_score >= 0)

        if not fillable.any():
            break

        result[fillable] = best_rgb[fillable]
        score[fillable] = 0
        known |= fillable

    return (result, known & ~visible)


def encode_as_palette(array, save_args):
    """
    Losslessly re-encode an RGBA array as a palettised PNG, or return None if it
    needs more than 256 distinct RGBA values. Keeps the tiny sprites tiny:
    RGBA-ifying a palettised sprite costs roughly twice its size on disk.
    """
    flat = array.reshape(-1, 4).astype(np.uint32)
    keys = (flat[:, 0] << 24) | (flat[:, 1] << 16) | (flat[:, 2] << 8) | flat[:, 3]

    unique_keys, indices = np.unique(keys, return_inverse=True)

    if len(unique_keys) > 256:
        return None

    colours = np.stack([
        (unique_keys >> 24) & 0xFF,
        (unique_keys >> 16) & 0xFF,
        (unique_keys >> 8) & 0xFF,
        unique_keys & 0xFF,
    ], axis=-1).astype(np.uint8)

    image = Image.fromarray(indices.reshape(array.shape[:2]).astype(np.uint8), mode="P")
    image.putpalette(colours[:, :3].reshape(-1).tolist())

    encoded = io.BytesIO()
    image.save(encoded, format="PNG", optimize=True, transparency=bytes(colours[:, 3].tolist()), **save_args)

    return encoded.getvalue()


def encode_preserving_mode(array, save_args, original_mode):
    """
    Returns (bytes, mode). The original storage mode is kept whenever it still
    fits, so that a sprite painted in RGB does not silently turn Indexed in the
    artist's editor - and so that a palettised sprite does not double in size.
    """
    if original_mode in ("P", "PA"):
        as_palette = encode_as_palette(array, save_args)

        if as_palette is not None:
            return (as_palette, "P")

    if original_mode == "LA" and (array[..., 0] == array[..., 1]).all() and (array[..., 1] == array[..., 2]).all():
        encoded = io.BytesIO()
        Image.fromarray(array[..., [0, 3]], mode="LA").save(encoded, format="PNG", optimize=True, **save_args)

        return (encoded.getvalue(), "LA")

    encoded = io.BytesIO()
    Image.fromarray(array, mode="RGBA").save(encoded, format="PNG", optimize=True, **save_args)

    return (encoded.getvalue(), "RGBA")


# The engine decodes every PNG through stb_image with req_comp = 4
# (see src/augs/image/image.cpp), so a palettised sprite ends up as RGBA all the
# same, garbage palette entries included. Such a file therefore has to be
# rewritten as RGBA to be fixable per texel - but only if it is actually dirty.
ALPHA_MODES = ("RGBA", "LA", "PA", "P")
OPAQUE_MODES = ("RGB", "L", "1")


def clean_file(path, radius, dry_run):
    """
    Returns (status, n_dirty, max_garbage, size_before, size_after), where
    status is one of "cleaned...", "clean", "no-alpha" or "skipped: <reason>".

    The re-encoded bytes are always produced in memory first, so a dry run
    reports exactly the file size the real run would leave behind.
    """
    size_before = os.path.getsize(path)
    nothing = (0, 0, size_before, size_before)

    try:
        image = Image.open(path)
    except Exception as e:
        return (f"skipped: unreadable ({e})",) + nothing

    if image.mode in OPAQUE_MODES:
        return ("no-alpha",) + nothing

    if image.mode not in ALPHA_MODES:
        return (f"skipped: mode {image.mode}",) + nothing

    original_mode = image.mode
    array = np.array(image.convert("RGBA"))

    rgb = array[..., :3]
    alpha = array[..., 3]

    if not (alpha == 0).any():
        return ("clean",) + nothing

    fixed, filled = solidified_rgb(rgb, alpha, radius)

    # Nothing past the filled ring can ever be sampled, so flattening it to plain
    # black both drops the leftover garbage and compresses better than noise.
    fixed[(alpha == 0) & ~filled] = 0

    differs = (fixed != rgb).any(axis=-1)
    n_dirty = int(differs.sum())

    if n_dirty == 0:
        return ("clean",) + nothing

    max_garbage = int(np.abs(fixed[differs].astype(np.int16) - rgb[differs].astype(np.int16)).max())

    array[..., :3] = fixed

    save_args = {}

    if "dpi" in image.info:
        save_args["dpi"] = image.info["dpi"]

    payload, saved_mode = encode_preserving_mode(array, save_args, original_mode)

    # Re-encoding can be a no-op even when some texel differs, so compare the
    # bytes before touching the file: the report then only lists real changes.
    with open(path, "rb") as f:
        if f.read() == payload:
            return ("clean",) + nothing

    if not dry_run:
        with open(path, "wb") as f:
            f.write(payload)

    status = "cleaned" if saved_mode == original_mode else f"cleaned ({original_mode} -> {saved_mode})"

    return (status, n_dirty, max_garbage, size_before, len(payload))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("paths", nargs="*", default=DEFAULT_PATHS, help="files or directories to scan")
    parser.add_argument("--radius", type=int, default=1, help="how many texel rings to solidify (default: 1)")
    parser.add_argument("--dry-run", action="store_true", help="only report, never write")
    parser.add_argument("--check", action="store_true", help="imply --dry-run and exit 1 if any sprite is dirty")
    parser.add_argument("--quiet", action="store_true", help="only print the summary")
    parser.add_argument("--print-changed", action="store_true",
                        help="print nothing but the path of every rewritten sprite, one per line")
    args = parser.parse_args()

    args.dry_run = args.dry_run or args.check
    args.quiet = args.quiet or args.print_changed

    paths = collect_pngs(args.paths or DEFAULT_PATHS)

    if not paths:
        print("No PNGs found.")
        return 1

    cleaned = []
    skipped = []
    n_clean = 0
    n_no_alpha = 0

    for path in paths:
        status, n_dirty, max_garbage, size_before, size_after = clean_file(path, args.radius, args.dry_run)

        if status.startswith("cleaned"):
            note = status[len("cleaned"):].strip()
            cleaned.append((n_dirty, max_garbage, path, note, size_before, size_after))
        elif status == "clean":
            n_clean += 1
        elif status == "no-alpha":
            n_no_alpha += 1
        else:
            skipped.append((status, path))

    cleaned.sort(reverse=True)

    if args.print_changed:
        for entry in cleaned:
            print(entry[2])

        return 0

    if cleaned and not args.quiet:
        verb = "Would clean" if args.dry_run else "Cleaned"
        print(f"{verb} {len(cleaned)} sprite(s):\n")
        print(f"{'dirty texels':>13}  {'max garbage':>11}  {'bytes':>9}  {'delta':>9}  sprite")

        for n_dirty, max_garbage, path, note, size_before, size_after in cleaned:
            suffix = f" {note}" if note else ""
            delta = size_after - size_before
            print(f"{n_dirty:>13}  {max_garbage:>11}  {size_after:>9}  {delta:>+9}  {path}{suffix}")

        print()

    if skipped and not args.quiet:
        print(f"Skipped {len(skipped)} file(s):\n")

        for status, path in skipped:
            print(f"  {path}: {status}")

        print()

    total_dirty = sum(entry[0] for entry in cleaned)
    total_before = sum(entry[4] for entry in cleaned)
    total_after = sum(entry[5] for entry in cleaned)
    n_grew = sum(1 for entry in cleaned if entry[5] > entry[4])
    n_shrank = sum(1 for entry in cleaned if entry[5] < entry[4])

    print(f"Scanned {len(paths)} PNG(s): "
          f"{len(cleaned)} dirty ({total_dirty} texels), "
          f"{n_clean} already clean, "
          f"{n_no_alpha} without alpha, "
          f"{len(skipped)} skipped.")

    if cleaned:
        mb = 1024.0 * 1024.0
        delta = total_after - total_before
        ratio = total_after / total_before if total_before else 1.0

        print(f"Size of the rewritten sprites: "
              f"{total_before / mb:.2f} MB -> {total_after / mb:.2f} MB "
              f"({delta / mb:+.2f} MB, {ratio:.2f}x); "
              f"{n_grew} grew, {n_shrank} shrank.")

    if args.check:
        if cleaned:
            print(f"\nFAILED: {len(cleaned)} sprite(s) carry garbage under alpha == 0.")
            print("Run cmake/asset_scripts/clean_sprites.py to fix them.")
            return 1

        print("OK: every sprite has a clean transparent edge.")
        return 0

    if args.dry_run:
        print("Dry run: nothing was written.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
