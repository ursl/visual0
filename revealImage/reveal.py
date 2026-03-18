#!/usr/bin/env python3
"""Minimal image reveal animation: blank → full image over time."""

import argparse
import numpy as np
import cv2


def build_pixel_order(h, w):
    """Random permutation of all pixel (y,x) indices."""
    indices = np.arange(h * w)
    np.random.shuffle(indices)
    return np.unravel_index(indices, (h, w))


def build_patch_order(h, w, patch_h, patch_w):
    """Random permutation of patch (iy,ix) indices."""
    ny = (h + patch_h - 1) // patch_h
    nx = (w + patch_w - 1) // patch_w
    indices = np.arange(ny * nx)
    np.random.shuffle(indices)
    return np.unravel_index(indices, (ny, nx))


def build_patch_order_bottom_up(h, w, patch_h, patch_w, spread=0.45):
    """
    Patch order weighted by y: bottom patches tend to appear first, with a smooth
    transition (no sharp zone boundaries). Random noise allows a few upper patches
    to appear in the initial phase.
    """
    ny = (h + patch_h - 1) // patch_h
    nx = (w + patch_w - 1) // patch_w
    patches = []
    for iy in range(ny):
        for ix in range(nx):
            # Base: 0 at top, 1 at bottom (smooth gradient)
            base = iy / max(1, ny - 1)
            # Noise creates transition overlap; positive noise lets upper patches appear early
            noise = np.random.uniform(-spread, spread)
            score = base + noise
            patches.append((score, iy, ix))
    patches.sort(key=lambda p: -p[0])  # high score first (bottom tendency)
    return [p[1] for p in patches], [p[2] for p in patches]


def reveal_random(img, out, total_frames, algorithm, patch_size, bg):
    """Standard random-order pixel or patch reveal."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)

    if algorithm == "pixels":
        ys, xs = build_pixel_order(h, w)
        total = h * w
    else:
        patch_iy, patch_ix = build_patch_order(h, w, patch_size, patch_size)
        total = len(patch_iy)

    idx = 0
    for frame_num in range(total_frames):
        end = min((frame_num + 1) * total // total_frames, total)
        if algorithm == "pixels":
            canvas[ys[idx:end], xs[idx:end]] = img[ys[idx:end], xs[idx:end]]
            idx = end
        else:
            for i in range(idx, end):
                iy, ix = patch_iy[i], patch_ix[i]
                y1 = iy * patch_size
                y2 = min(y1 + patch_size, h)
                x1 = ix * patch_size
                x2 = min(x1 + patch_size, w)
                canvas[y1:y2, x1:x2] = img[y1:y2, x1:x2]
            idx = end

        out.write(canvas.copy())
        if idx >= total:
            break

    # Hold final frame for remaining duration
    for _ in range(frame_num + 1, total_frames):
        out.write(img)


def reveal_bottom_up(img, out, total_frames, patch_size, bg, noise=0.45):
    """Reveal starts in lower third of image, then moves upward through middle and top."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)
    patch_iy, patch_ix = build_patch_order_bottom_up(h, w, patch_size, patch_size, spread=noise)
    total = len(patch_iy)

    idx = 0
    for frame_num in range(total_frames):
        end = min((frame_num + 1) * total // total_frames, total)
        for i in range(idx, end):
            iy, ix = patch_iy[i], patch_ix[i]
            y1 = iy * patch_size
            y2 = min(y1 + patch_size, h)
            x1 = ix * patch_size
            x2 = min(x1 + patch_size, w)
            canvas[y1:y2, x1:x2] = img[y1:y2, x1:x2]
        idx = end

        out.write(canvas.copy())
        if idx >= total:
            break

    for _ in range(frame_num + 1, total_frames):
        out.write(img)


def main():
    p = argparse.ArgumentParser(description="Image reveal animation")
    p.add_argument("image", help="Input JPG path")
    p.add_argument("-o", "--output", default="reveal.mp4", help="Output video path")
    p.add_argument("-d", "--duration", type=float, default=5.0, help="Seconds until complete")
    p.add_argument("-f", "--fps", type=int, default=30, help="Frames per second")
    p.add_argument(
        "-a", "--algorithm",
        choices=["pixels", "patches", "patches_bottom_up"],
        default="patches_bottom_up",
        help="Reveal algorithm (patches_bottom_up: lower third first, then moves up)",
    )
    p.add_argument("-p", "--patch-size", type=int, default=16, help="Patch size (for patches)")
    p.add_argument("-b", "--background", choices=["white", "black"], default="black")
    p.add_argument("-i", "--intro", type=float, default=10.0, help="Initial black screen duration (seconds)")
    p.add_argument("-s", "--seed", type=int, default=1234, help="Random seed for reproducible patch order")
    p.add_argument("--noise", type=float, default=0.45, help="Noise spread for patches_bottom_up (higher=more upper patches early, 0=strict bottom-up)")
    args = p.parse_args()

    if args.seed is not None:
        np.random.seed(args.seed)

    img = cv2.imread(args.image)
    if img is None:
        raise SystemExit(f"Cannot read image: {args.image}")

    h, w = img.shape[:2]
    bg = 255 if args.background == "white" else 0

    total_frames = int(args.duration * args.fps)
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    out = cv2.VideoWriter(args.output, fourcc, args.fps, (w, h))

    # Initial black screen
    intro_frames = int(args.intro * args.fps)
    black_frame = np.zeros_like(img)
    for _ in range(intro_frames):
        out.write(black_frame)

    if args.algorithm == "patches_bottom_up":
        reveal_bottom_up(img, out, total_frames, args.patch_size, bg, args.noise)
    else:
        reveal_random(img, out, total_frames, args.algorithm, args.patch_size, bg)

    out.release()
    print(f"Written {args.output} ({intro_frames + total_frames} frames, {args.fps} fps)")


if __name__ == "__main__":
    main()
