#!/usr/bin/env python3
"""Expand image laterally with smooth transition at vertical borders."""

import argparse
import numpy as np
import cv2

# Good calls
# ----------
# python smoothEdge.py im1.jpg -m sparse -e 20 --sparse-base 0.9 --seam-blend 0 --sample-band 100 -o im1_edges.png



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


def _column_luma_bgr(col: np.ndarray) -> np.ndarray:
    """Luma 0..255 per row for one edge column (h, 3) BGR."""
    b, g, r = col.astype(np.float32).T
    return 0.114 * b + 0.587 * g + 0.299 * r


def _sample_row_in_band(
    y: int,
    band: int,
    h: int,
    rng: np.random.Generator,
    luma: np.ndarray | None = None,
    max_luma: float | None = None,
) -> int:
    """Random row in a vertical window of ``band`` pixels, centered on ``y``.

    If ``max_luma`` is set, only rows with edge luma <= ``max_luma`` are sampled;
    if none qualify, the darkest row in the band is used.
    """
    if h <= 1:
        return 0
    band = max(1, min(band, h))
    y_lo = y - band // 2
    if y_lo < 0:
        y_lo = 0
    if y_lo + band > h:
        y_lo = h - band
    y_hi = y_lo + band
    rows = np.arange(y_lo, y_hi, dtype=np.int64)
    if max_luma is not None and luma is not None:
        bright_ok = luma[rows] <= max_luma
        valid = rows[bright_ok]
        if valid.size > 0:
            return int(rng.choice(valid))
        return int(rows[np.argmin(luma[rows])])
    return int(rng.integers(y_lo, y_hi, endpoint=False))


def _blend_seam_toward_edge(
    out: np.ndarray,
    x_col: int,
    anchor: np.ndarray,
    background: tuple,
    weight: float,
) -> None:
    """Mix sparse samples in one column toward same-row edge color (in-place)."""
    if weight <= 0:
        return
    h = out.shape[0]
    bg = np.array(background, dtype=out.dtype)
    w = float(np.clip(weight, 0.0, 1.0))
    for y in range(h):
        if np.all(out[y, x_col] == bg):
            continue
        b = (1.0 - w) * out[y, x_col].astype(np.float32) + w * anchor[y].astype(np.float32)
        out[y, x_col] = np.clip(b, 0, 255).astype(out.dtype)


def smooth_edges_sparse(
    img: np.ndarray,
    edge_width: int,
    background: tuple = (0, 0, 0),
    sparse_base: float = 0.04,
    sparse_decay: float = 0.11,
    sparse_falloff: str = "linear",
    sparse_linear_span: float | None = None,
    sample_band: int = 5,
    max_sample_luma: float | None = None,
    seam_blend: float = 0.0,
    seed: int | None = None,
) -> np.ndarray:
    """
    Per side, add ``3 * edge_width`` columns. Each pixel is filled independently
    with a random sample from the adjacent vertical edge column, drawn only
    from rows within ``sample_band`` pixels of the output row (same vertical area).

    Distance ``d`` counts columns from the original image edge (0 = innermost).

    * linear (default): ``p(d) = sparse_base * max(0, 1 - d / span)`` with
      ``span`` = ``sparse_linear_span`` or full extension width ``3*X - 1``.
    * exp: ``p(d) = sparse_base * exp(-sparse_decay * d)``.

    If ``max_sample_luma`` is set, edge pixels brighter than that (luma) are never
    chosen; helps avoid speckles in dark regions.

    ``seam_blend`` (0..1): inner extension column is mixed toward the true edge
    at the same row to soften the boundary with the original image.
    """
    h, w = img.shape[:2]
    X = edge_width
    if X <= 0:
        return img.copy()

    ext = 3 * X
    rng = np.random.default_rng(seed)
    d_axis = np.arange(ext, dtype=np.float64)
    if sparse_falloff == "linear":
        span = sparse_linear_span if sparse_linear_span is not None else float(max(1, ext - 1))
        span = max(span, 1e-6)
        col_prob = sparse_base * np.maximum(0.0, 1.0 - d_axis / span)
        col_prob = np.clip(col_prob, 0.0, 1.0)
    else:
        col_prob = np.clip(sparse_base * np.exp(-sparse_decay * d_axis), 0.0, 1.0)

    new_w = w + 2 * ext
    out = np.empty((h, new_w, img.shape[2]), dtype=img.dtype)
    out[:] = background

    lx = ext
    out[:, lx : lx + w] = img

    left_col = img[:, 0]
    right_col = img[:, -1]
    left_luma = _column_luma_bgr(left_col) if max_sample_luma is not None else None
    right_luma = _column_luma_bgr(right_col) if max_sample_luma is not None else None

    # Left: x = ext-1 touches image (d=0), x = 0 is outermost (d = ext-1)
    for x in range(ext):
        d = ext - 1 - x
        p = col_prob[d]
        sel = rng.random(h) < p
        for y in np.flatnonzero(sel):
            y_pick = _sample_row_in_band(
                y, sample_band, h, rng, luma=left_luma, max_luma=max_sample_luma
            )
            out[y, x] = left_col[y_pick]

    # Right: xi = 0 touches image, xi = ext-1 outermost
    for xi in range(ext):
        p = col_prob[xi]
        xs = lx + w + xi
        sel = rng.random(h) < p
        for y in np.flatnonzero(sel):
            y_pick = _sample_row_in_band(
                y, sample_band, h, rng, luma=right_luma, max_luma=max_sample_luma
            )
            out[y, xs] = right_col[y_pick]

    if seam_blend > 0:
        _blend_seam_toward_edge(out, ext - 1, left_col, background, seam_blend)
        _blend_seam_toward_edge(out, lx + w, right_col, background, seam_blend)

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
        help="Edge width X (symmetric). blur: extension X each side. sparse: extension 3*X each side",
    )
    p.add_argument(
        "-m",
        "--mode",
        choices=["blur", "sparse"],
        default="blur",
        help="blur: Gaussian fade. sparse: random edge samples, density drops with distance from image",
    )
    p.add_argument(
        "--sparse-base",
        type=float,
        default=0.04,
        help="sparse: max fill probability at inner edge (d=0); linear default ~few pixels (default 0.04)",
    )
    p.add_argument(
        "--sparse-falloff",
        choices=["linear", "exp"],
        default="linear",
        help="sparse: how density drops with distance from image (default linear)",
    )
    p.add_argument(
        "--sparse-linear-span",
        type=float,
        default=None,
        metavar="D",
        help="sparse linear: density hits 0 at distance D columns from image; "
        "omit to use full extension (3*e-1). Larger D = slower / gentler decay",
    )
    p.add_argument(
        "--sparse-decay",
        type=float,
        default=0.11,
        help="sparse exp falloff only: multiplier in exp(-sparse_decay * d); larger = faster drop",
    )
    p.add_argument(
        "--sample-band",
        type=int,
        default=5,
        help="sparse: sample edge pixels only from this many rows centered on output row (default 5)",
    )
    p.add_argument(
        "--max-sample-luma",
        type=float,
        default=None,
        metavar="L",
        help="sparse: never sample edge pixels with luma above L (0-255, Rec.709-style on BGR); "
        "cuts bright outliers in dark areas. Omit to disable.",
    )
    p.add_argument(
        "--seam-blend",
        type=float,
        default=0.35,
        help="sparse: mix inner extension column toward same-row edge (0=off, ~0.35 default). "
        "Softens the line next to the original image.",
    )
    p.add_argument("--seed", type=int, default=None, help="sparse mode: RNG seed for reproducibility")
    p.add_argument("-o", "--output", help="Output image path (default: input with _smooth suffix)")
    p.add_argument(
        "-b",
        "--background",
        choices=["black", "white"],
        default="black",
        help="Background color for extended regions",
    )
    args = p.parse_args()

    img = cv2.imread(args.image)
    if img is None:
        raise SystemExit(f"Cannot read image: {args.image}")

    bg = (255, 255, 255) if args.background == "white" else (0, 0, 0)

    if args.mode == "blur":
        out = smooth_edges(img, args.edge, bg)
    else:
        out = smooth_edges_sparse(
            img,
            args.edge,
            bg,
            sparse_base=args.sparse_base,
            sparse_decay=args.sparse_decay,
            sparse_falloff=args.sparse_falloff,
            sparse_linear_span=args.sparse_linear_span,
            sample_band=args.sample_band,
            max_sample_luma=args.max_sample_luma,
            seam_blend=args.seam_blend,
            seed=args.seed,
        )

    if args.output:
        output_path = args.output
    else:
        stem = args.image.rsplit(".", 1)[0] if "." in args.image else args.image
        ext = args.image.rsplit(".", 1)[1] if "." in args.image else "png"
        output_path = f"{stem}_smooth.{ext}"

    cv2.imwrite(output_path, out)
    print(f"Written {output_path} ({out.shape[1]}x{out.shape[0]}, mode={args.mode})")


if __name__ == "__main__":
    main()
