#!/usr/bin/env python3
"""Minimal image reveal animation: blank → full image over time."""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import numpy as np
import cv2


def normalize_image_for_video(img: np.ndarray) -> np.ndarray:
    """
    OpenCV VideoWriter expects uint8 BGR (h, w, 3), contiguous. Other dtypes or
    channel counts often produce green/corrupted output in the encoded file.
    """
    if img is None:
        return img
    if img.dtype != np.uint8:
        if np.issubdtype(img.dtype, np.floating):
            mx = float(np.max(img)) if img.size else 0.0
            if mx <= 1.0 + 1e-6:
                img = np.clip(img * 255.0, 0, 255).astype(np.uint8)
            else:
                img = np.clip(img, 0, 255).astype(np.uint8)
        else:
            img = cv2.convertScaleAbs(img)
    if img.ndim == 2:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    elif img.shape[2] == 4:
        img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)
    elif img.shape[2] != 3:
        raise ValueError(f"Expected 1, 3, or 4 channels, got shape {img.shape}")
    return np.ascontiguousarray(img)


def try_reencode_mp4_libx264(path: str, fps: int, crf: int = 20) -> bool:
    """
    Re-encode MP4 to H.264 yuv420p with explicit frame rate.

    OpenCV's mp4v output is sometimes broken at specific fps values (e.g. 8 on
    macOS): green stripes and wrong playback. A libx264 pass fixes the stream.
    Returns True if re-encoded, False if skipped or failed.
    """
    if shutil.which("ffmpeg") is None:
        return False
    out_dir = os.path.dirname(os.path.abspath(path)) or "."
    fd, tmp = tempfile.mkstemp(suffix=".mp4", dir=out_dir)
    os.close(fd)
    try:
        subprocess.run(
            [
                "ffmpeg",
                "-y",
                "-nostdin",
                "-loglevel",
                "error",
                "-i",
                path,
                "-c:v",
                "libx264",
                "-pix_fmt",
                "yuv420p",
                "-crf",
                str(crf),
                "-r",
                str(float(fps)),
                "-an",
                tmp,
            ],
            check=True,
            capture_output=True,
        )
        os.replace(tmp, path)
        return True
    except (subprocess.CalledProcessError, OSError) as e:
        try:
            os.unlink(tmp)
        except OSError:
            pass
        err = getattr(e, "stderr", None)
        if err is not None and isinstance(err, bytes):
            err = err.decode(errors="replace")
        print(f"Warning: ffmpeg re-encode failed ({e!s} {err or ''}); leaving OpenCV output", file=sys.stderr)
        return False


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
    img,
    out,
    intro_frames,
    total_frames,
    algorithm,
    patch_size,
    bg,
    intro_slow_factor=10,
    solid_black_intro=False,
):
    """Standard random-order pixel or patch reveal. Intro: slow patches or solid black."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)

    if algorithm == "pixels":
        ys, xs = build_pixel_order(h, w)
        total = h * w
    else:
        patch_iy, patch_ix = build_patch_order(h, w, patch_size, patch_size)
        total = len(patch_iy)

    idx = 0
    if solid_black_intro:
        for _ in range(intro_frames):
            out.write(np.ascontiguousarray(canvas))
    else:
        main_denom = max(1, intro_slow_factor * max(1, total_frames))
        for frame_num in range(intro_frames):
            proposed = min(total, int(round((frame_num + 1) * total / main_denom)))
            end = max(proposed, idx)
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
            out.write(np.ascontiguousarray(canvas))

    intro_end = idx

    frame_num = -1
    denom = max(1, total_frames)
    remaining = total - intro_end
    for frame_num in range(total_frames):
        proposed = intro_end + int(round((frame_num + 1) * remaining / denom))
        end = min(total, max(proposed, idx))
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

        out.write(np.ascontiguousarray(canvas))
        if idx >= total:
            break

    for _ in range(frame_num + 1, total_frames):
        out.write(np.ascontiguousarray(img))


def reveal_bottom_up(
    img,
    out,
    intro_frames,
    total_frames,
    patch_size,
    bg,
    noise=0.45,
    intro_slow_factor=10,
    solid_black_intro=False,
):
    """Reveal starts bottom-weighted. Intro: slow patches or solid black."""
    h, w = img.shape[:2]
    canvas = np.full_like(img, bg)
    patch_iy, patch_ix = build_patch_order_bottom_up(h, w, patch_size, patch_size, spread=noise)
    total = len(patch_iy)

    idx = 0
    if solid_black_intro:
        for _ in range(intro_frames):
            out.write(np.ascontiguousarray(canvas))
    else:
        main_denom = max(1, intro_slow_factor * max(1, total_frames))
        for frame_num in range(intro_frames):
            proposed = min(total, int(round((frame_num + 1) * total / main_denom)))
            end = max(proposed, idx)
            for i in range(idx, end):
                iy, ix = patch_iy[i], patch_ix[i]
                y1 = iy * patch_size
                y2 = min(y1 + patch_size, h)
                x1 = ix * patch_size
                x2 = min(x1 + patch_size, w)
                canvas[y1:y2, x1:x2] = img[y1:y2, x1:x2]
            idx = end
            out.write(np.ascontiguousarray(canvas))

    intro_end = idx

    frame_num = -1
    denom = max(1, total_frames)
    remaining = total - intro_end
    for frame_num in range(total_frames):
        proposed = intro_end + int(round((frame_num + 1) * remaining / denom))
        end = min(total, max(proposed, idx))
        for i in range(idx, end):
            iy, ix = patch_iy[i], patch_ix[i]
            y1 = iy * patch_size
            y2 = min(y1 + patch_size, h)
            x1 = ix * patch_size
            x2 = min(x1 + patch_size, w)
            canvas[y1:y2, x1:x2] = img[y1:y2, x1:x2]
        idx = end

        out.write(np.ascontiguousarray(canvas))
        if idx >= total:
            break

    for _ in range(frame_num + 1, total_frames):
        out.write(np.ascontiguousarray(img))


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
        help="Intro reveal rate = main rate / this (default 10); ignored with --solid-black-intro",
    )
    p.add_argument(
        "--solid-black-intro",
        action="store_true",
        help="During -i seconds write only solid background (no patch reveal); then main reveal from blank",
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
    p.add_argument(
        "--no-reencode",
        action="store_true",
        help="Skip ffmpeg libx264 pass (faster; OpenCV mp4v can be corrupt at some fps, e.g. 8 on macOS)",
    )
    args = p.parse_args()

    if args.seed is not None:
        np.random.seed(args.seed)

    img = cv2.imread(args.image, cv2.IMREAD_UNCHANGED)
    if img is None:
        raise SystemExit(f"Cannot read image: {args.image}")
    try:
        img = normalize_image_for_video(img)
    except ValueError as e:
        raise SystemExit(str(e)) from e

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
    if not out.isOpened():
        raise SystemExit(
            f"VideoWriter failed to open {video_out_path!r} (codec mp4v, size {w}x{h} @ {args.fps} fps)"
        )

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
            args.solid_black_intro,
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
            args.solid_black_intro,
        )

    out.release()

    if audio_path:
        try:
            # libx264 needs ffmpeg; fall back to copy if missing
            use_h264 = not args.no_reencode and shutil.which("ffmpeg") is not None
            vcodec = (
                [
                    "-c:v",
                    "libx264",
                    "-pix_fmt",
                    "yuv420p",
                    "-crf",
                    "20",
                    "-r",
                    str(float(args.fps)),
                ]
                if use_h264
                else ["-c:v", "copy"]
            )
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
                    *vcodec,
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
        if not args.no_reencode and try_reencode_mp4_libx264(video_out_path, args.fps):
            print(
                f"Written {args.output} ({intro_frames + total_frames} frames, {args.fps} fps, libx264 re-encode)"
            )
        else:
            print(f"Written {args.output} ({intro_frames + total_frames} frames, {args.fps} fps)")


if __name__ == "__main__":
    main()
