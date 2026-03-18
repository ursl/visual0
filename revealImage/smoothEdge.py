#!/usr/bin/env python3
"""Expand image laterally with smooth transition at vertical borders."""

import argparse
import numpy as np
import cv2


def smooth_edges(img: np.ndarray, edge_width: int, background: tuple = (0, 0, 0)) -> np.ndarray:
    """
    Expand image by edge_width pixels on left and right. In the edge region,
    contrast gradually fades: we blend a progressively blurred version of the
    edge toward the background.
    """
    h, w = img.shape[:2]
    if edge_width <= 0:
        return img.copy()

    new_w = w + 2 * edge_width
    out = np.empty((h, new_w, img.shape[2]), dtype=img.dtype)
    out[:] = background

    # Original image in the center
    out[:, edge_width : edge_width + w] = img

    # Build edge strips; blur increases as we go outward so contrast fades
    strip_w = min(edge_width * 2, w // 2, 64)
    strip_w = max(strip_w, 4)
    left_strip = img[:, :strip_w].astype(np.float32)
    right_strip = img[:, -strip_w:].astype(np.float32)
    bg_arr = np.array(background, dtype=np.float32)

    # Precompute blur levels: kernel grows 1, 9, 17, 25, 33 so outer edge is heavily blurred
    n_levels = 5
    kernels = [max(1, 1 + i * 8) | 1 for i in range(n_levels)]  # odd: 1, 9, 17, 25, 33
    left_blurs = [cv2.GaussianBlur(left_strip, (k, k), 0) for k in kernels]
    right_blurs = [cv2.GaussianBlur(right_strip, (k, k), 0) for k in kernels]

    # Left edge
    for x in range(edge_width):
        t = (x + 1) / edge_width  # far left 0 -> at image 1
        # More blur as we go outward (contrast fades)
        level = (1 - t) * (n_levels - 1)
        i0, i1 = int(level), min(int(level) + 1, n_levels - 1)
        w1 = level - i0
        sample = (1 - w1) * left_blurs[i0][:, -1] + w1 * left_blurs[i1][:, -1]
        alpha = t * t  # smooth opacity falloff
        blended = alpha * sample + (1 - alpha) * bg_arr
        out[:, x] = np.clip(blended, 0, 255).astype(img.dtype)

    # Right edge
    for x in range(edge_width):
        t = (x + 1) / edge_width
        level = (1 - t) * (n_levels - 1)
        i0, i1 = int(level), min(int(level) + 1, n_levels - 1)
        w1 = level - i0
        sample = (1 - w1) * right_blurs[i0][:, 0] + w1 * right_blurs[i1][:, 0]
        alpha = t * t
        blended = alpha * sample + (1 - alpha) * bg_arr
        out[:, new_w - 1 - x] = np.clip(blended, 0, 255).astype(img.dtype)

    return out


def main():
    p = argparse.ArgumentParser(
        description="Expand image laterally with smooth transition at vertical borders"
    )
    p.add_argument("image", help="Input image path")
    p.add_argument(
        "-e",
        "--edge",
        type=int,
        default=100,
        help="Edge width in pixels (applied symmetrically left and right)",
    )
    p.add_argument("-o", "--output", help="Output image path (default: input with _smooth suffix)")
    p.add_argument(
        "-b",
        "--background",
        choices=["black", "white"],
        default="black",
        help="Background color for the smoothed transition",
    )
    args = p.parse_args()

    img = cv2.imread(args.image)
    if img is None:
        raise SystemExit(f"Cannot read image: {args.image}")

    bg = (255, 255, 255) if args.background == "white" else (0, 0, 0)
    out = smooth_edges(img, args.edge, bg)

    if args.output:
        output_path = args.output
    else:
        stem = args.image.rsplit(".", 1)[0] if "." in args.image else args.image
        ext = args.image.rsplit(".", 1)[1] if "." in args.image else "png"
        output_path = f"{stem}_smooth.{ext}"

    cv2.imwrite(output_path, out)
    print(f"Written {output_path} ({out.shape[1]}x{out.shape[0]})")


if __name__ == "__main__":
    main()
