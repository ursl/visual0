#!/usr/bin/env python3
"""Minimal image reveal animation: blank → full image over time."""

import argparse
import os
import subprocess
import sys
import tempfile
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


def reveal_random(
    img, out, intro_frames, total_frames, algorithm, patch_size, bg, intro_slow_factor=10
):
    """Standard random-order pixel or patch reveal. Intro uses slower patch rate."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)

    if algorithm == "pixels":
        ys, xs = build_pixel_order(h, w)
        total = h * w
    else:
        patch_iy, patch_ix = build_patch_order(h, w, patch_size, patch_size)
        total = len(patch_iy)

    idx = 0
    main_denom = max(1, intro_slow_factor * max(1, total_frames))

    for frame_num in range(intro_frames):
        end = min((frame_num + 1) * total // main_denom, total)
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

    intro_end = idx

    frame_num = -1
    for frame_num in range(total_frames):
        end = min(
            intro_end + (frame_num + 1) * (total - intro_end) // max(1, total_frames),
            total,
        )
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

    for _ in range(frame_num + 1, total_frames):
        out.write(img)


def reveal_bottom_up(
    img, out, intro_frames, total_frames, patch_size, bg, noise=0.45, intro_slow_factor=10
):
    """Reveal starts bottom-weighted. Intro uses slower patch rate than main phase."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)
    patch_iy, patch_ix = build_patch_order_bottom_up(h, w, patch_size, patch_size, spread=noise)
    total = len(patch_iy)

    idx = 0
    main_denom = max(1, intro_slow_factor * max(1, total_frames))

    for frame_num in range(intro_frames):
        end = min((frame_num + 1) * total // main_denom, total)
        for i in range(idx, end):
            iy, ix = patch_iy[i], patch_ix[i]
            y1 = iy * patch_size
            y2 = min(y1 + patch_size, h)
            x1 = ix * patch_size
            x2 = min(x1 + patch_size, w)
            canvas[y1:y2, x1:x2] = img[y1:y2, x1:x2]
        idx = end
        out.write(canvas.copy())

    intro_end = idx

    frame_num = -1
    for frame_num in range(total_frames):
        end = min(
            intro_end + (frame_num + 1) * (total - intro_end) // max(1, total_frames),
            total,
        )
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
    p.add_argument(
        "-i",
        "--intro",
        type=float,
        default=10.0,
        help="Intro duration (seconds); patches appear at 1/intro_slow_factor of main rate",
    )
    p.add_argument(
        "--intro-slow-factor",
        type=int,
        default=10,
        help="Intro reveal rate = main rate / this (default 10)",
    )
    p.add_argument("-s", "--seed", type=int, default=1234, help="Random seed for reproducible patch order")
    p.add_argument("--noise", type=float, default=0.45, help="Noise spread for patches_bottom_up (higher=more upper patches early, 0=strict bottom-up)")
    p.add_argument(
        "--audio",
        metavar="PATH",
        default=os.path.expanduser("~/Downloads/Spiegel_spotdown.mp3"),
        help="MP3 to mux into the output (requires ffmpeg). Default: ~/Downloads/Spiegel_spotdown.mp3",
    )
    p.add_argument(
        "--no-audio",
        action="store_true",
        help="Do not mux audio (video only)",
    )
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

    audio_path = None
    if not args.no_audio and args.audio and str(args.audio).strip():
        audio_path = os.path.expanduser(os.path.expandvars(str(args.audio).strip()))
        if not os.path.isfile(audio_path):
            print(f"Warning: audio not found ({audio_path}), writing video without audio", file=sys.stderr)
            audio_path = None

    if audio_path:
        fd, video_tmp = tempfile.mkstemp(suffix=".mp4")
        os.close(fd)
        video_out_path = video_tmp
    else:
        video_out_path = args.output

    out = cv2.VideoWriter(video_out_path, fourcc, args.fps, (w, h))

    intro_frames = int(args.intro * args.fps)

    if args.algorithm == "patches_bottom_up":
        reveal_bottom_up(
            img,
            out,
            intro_frames,
            total_frames,
            args.patch_size,
            bg,
            args.noise,
            args.intro_slow_factor,
        )
    else:
        reveal_random(
            img,
            out,
            intro_frames,
            total_frames,
            args.algorithm,
            args.patch_size,
            bg,
            args.intro_slow_factor,
        )

    out.release()

    if audio_path:
        try:
            subprocess.run(
                [
                    "ffmpeg",
                    "-y",
                    "-nostdin",
                    "-loglevel",
                    "error",
                    "-i",
                    video_out_path,
                    "-i",
                    audio_path,
                    "-c:v",
                    "copy",
                    "-c:a",
                    "aac",
                    "-map",
                    "0:v:0",
                    "-map",
                    "1:a:0",
                    "-shortest",
                    args.output,
                ],
                check=True,
                capture_output=True,
            )
        except FileNotFoundError:
            os.unlink(video_out_path)
            raise SystemExit("ffmpeg not found; install ffmpeg to use --audio")
        except subprocess.CalledProcessError as e:
            os.unlink(video_out_path)
            err = (e.stderr or b"").decode(errors="replace")
            raise SystemExit(f"ffmpeg failed: {err or e}")
        os.unlink(video_out_path)
        print(
            f"Written {args.output} ({intro_frames + total_frames} frames, {args.fps} fps, muxed {audio_path})"
        )
    else:
        print(f"Written {args.output} ({intro_frames + total_frames} frames, {args.fps} fps)")


if __name__ == "__main__":
    main()
