#!/usr/bin/env python3
"""Generate SVG comparison charts from benchmark CSV.

Usage:
  python3 scripts/plot_benchmark.py results/benchmark.csv --outdir results

No third-party deps. Writes:
  results/time_by_size_random.svg
  results/comparisons_by_size_random.svg
  results/time_by_arrangement_n512.svg  (or largest size present)
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path


ALGOS = ("Bubble", "Merge", "Quick")
COLORS = {
    "Bubble": "#e45756",
    "Merge": "#4c78a8",
    "Quick": "#54a24b",
}


def read_rows(path: Path):
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        rows = []
        for row in reader:
            rows.append(
                {
                    "algorithm": row["algorithm"],
                    "arrangement": row["arrangement"],
                    "size": int(row["size"]),
                    "time_ms": float(row["time_ms"]),
                    "comparisons": int(row["comparisons"]),
                    "swaps": int(row["swaps"]),
                    "writes": int(row["writes"]),
                }
            )
        return rows


def svg_line_chart(
    title: str,
    series: dict[str, list[tuple[float, float]]],
    xlabel: str,
    ylabel: str,
    width: int = 900,
    height: int = 520,
) -> str:
    margin_l, margin_r, margin_t, margin_b = 72, 24, 48, 64
    plot_w = width - margin_l - margin_r
    plot_h = height - margin_t - margin_b

    all_x = [x for pts in series.values() for x, _ in pts]
    all_y = [y for pts in series.values() for _, y in pts]
    if not all_x or not all_y:
        return f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}"><text x="20" y="40">No data</text></svg>'

    xmin, xmax = min(all_x), max(all_x)
    ymin, ymax = 0.0, max(all_y) * 1.1 if max(all_y) > 0 else 1.0
    if xmax <= xmin:
        xmax = xmin + 1.0

    def sx(x: float) -> float:
        return margin_l + (x - xmin) / (xmax - xmin) * plot_w

    def sy(y: float) -> float:
        return margin_t + plot_h - (y - ymin) / (ymax - ymin) * plot_h

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">',
        f'<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{width/2:.1f}" y="28" text-anchor="middle" '
        f'font-family="Helvetica,Arial,sans-serif" font-size="18" fill="#222">{title}</text>',
        f'<line x1="{margin_l}" y1="{margin_t}" x2="{margin_l}" y2="{margin_t+plot_h}" stroke="#333"/>',
        f'<line x1="{margin_l}" y1="{margin_t+plot_h}" x2="{margin_l+plot_w}" y2="{margin_t+plot_h}" stroke="#333"/>',
        f'<text x="{width/2:.1f}" y="{height-18}" text-anchor="middle" '
        f'font-family="Helvetica,Arial,sans-serif" font-size="13" fill="#444">{xlabel}</text>',
        f'<text x="18" y="{height/2:.1f}" text-anchor="middle" '
        f'transform="rotate(-90 18 {height/2:.1f})" '
        f'font-family="Helvetica,Arial,sans-serif" font-size="13" fill="#444">{ylabel}</text>',
    ]

    # Y ticks
    for i in range(5):
        yv = ymin + (ymax - ymin) * i / 4
        y = sy(yv)
        label = f"{yv:.3g}"
        parts.append(
            f'<line x1="{margin_l-5}" y1="{y:.1f}" x2="{margin_l}" y2="{y:.1f}" stroke="#333"/>'
        )
        parts.append(
            f'<text x="{margin_l-10}" y="{y+4:.1f}" text-anchor="end" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="11" fill="#555">{label}</text>'
        )
        parts.append(
            f'<line x1="{margin_l}" y1="{y:.1f}" x2="{margin_l+plot_w}" y2="{y:.1f}" '
            f'stroke="#ddd" stroke-dasharray="3,3"/>'
        )

    # X ticks from first series keys
    xs = sorted({x for pts in series.values() for x, _ in pts})
    for x in xs:
        px = sx(x)
        parts.append(
            f'<line x1="{px:.1f}" y1="{margin_t+plot_h}" x2="{px:.1f}" '
            f'y2="{margin_t+plot_h+5}" stroke="#333"/>'
        )
        parts.append(
            f'<text x="{px:.1f}" y="{margin_t+plot_h+22}" text-anchor="middle" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="11" fill="#555">{int(x)}</text>'
        )

    legend_x = margin_l + 8
    legend_y = margin_t + 14
    for name, pts in series.items():
        color = COLORS.get(name, "#333")
        ordered = sorted(pts)
        if len(ordered) >= 2:
            d = "M " + " L ".join(f"{sx(x):.1f},{sy(y):.1f}" for x, y in ordered)
            parts.append(
                f'<path d="{d}" fill="none" stroke="{color}" stroke-width="2.5"/>'
            )
        for x, y in ordered:
            parts.append(
                f'<circle cx="{sx(x):.1f}" cy="{sy(y):.1f}" r="3.5" fill="{color}"/>'
            )
        parts.append(
            f'<rect x="{legend_x}" y="{legend_y-9}" width="14" height="14" fill="{color}"/>'
        )
        parts.append(
            f'<text x="{legend_x+20}" y="{legend_y+2}" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="13" fill="#222">{name}</text>'
        )
        legend_x += 110

    parts.append("</svg>")
    return "\n".join(parts)


def svg_grouped_bars(
    title: str,
    categories: list[str],
    series: dict[str, list[float]],
    ylabel: str,
    width: int = 960,
    height: int = 520,
) -> str:
    margin_l, margin_r, margin_t, margin_b = 72, 24, 48, 90
    plot_w = width - margin_l - margin_r
    plot_h = height - margin_t - margin_b

    all_y = [v for vals in series.values() for v in vals]
    ymax = max(all_y) * 1.15 if all_y and max(all_y) > 0 else 1.0

    n_cat = max(1, len(categories))
    n_series = max(1, len(series))
    group_w = plot_w / n_cat
    bar_w = group_w / (n_series + 1)

    def sy(y: float) -> float:
        return margin_t + plot_h - (y / ymax) * plot_h

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
        f'viewBox="0 0 {width} {height}">',
        f'<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{width/2:.1f}" y="28" text-anchor="middle" '
        f'font-family="Helvetica,Arial,sans-serif" font-size="18" fill="#222">{title}</text>',
        f'<line x1="{margin_l}" y1="{margin_t}" x2="{margin_l}" y2="{margin_t+plot_h}" stroke="#333"/>',
        f'<line x1="{margin_l}" y1="{margin_t+plot_h}" x2="{margin_l+plot_w}" y2="{margin_t+plot_h}" stroke="#333"/>',
        f'<text x="18" y="{height/2:.1f}" text-anchor="middle" '
        f'transform="rotate(-90 18 {height/2:.1f})" '
        f'font-family="Helvetica,Arial,sans-serif" font-size="13" fill="#444">{ylabel}</text>',
    ]

    for i in range(5):
        yv = ymax * i / 4
        y = sy(yv)
        parts.append(
            f'<line x1="{margin_l}" y1="{y:.1f}" x2="{margin_l+plot_w}" y2="{y:.1f}" '
            f'stroke="#ddd" stroke-dasharray="3,3"/>'
        )
        parts.append(
            f'<text x="{margin_l-10}" y="{y+4:.1f}" text-anchor="end" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="11" fill="#555">{yv:.3g}</text>'
        )

    for ci, cat in enumerate(categories):
        gx = margin_l + ci * group_w
        for si, (name, vals) in enumerate(series.items()):
            if ci >= len(vals):
                continue
            val = vals[ci]
            x = gx + bar_w * (si + 0.5)
            y = sy(val)
            h = margin_t + plot_h - y
            color = COLORS.get(name, "#333")
            parts.append(
                f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_w*0.9:.1f}" height="{h:.1f}" fill="{color}"/>'
            )
        parts.append(
            f'<text x="{gx + group_w/2:.1f}" y="{margin_t+plot_h+20}" text-anchor="middle" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="11" fill="#444">{cat}</text>'
        )

    legend_x = margin_l + 8
    legend_y = height - 28
    for name in series:
        color = COLORS.get(name, "#333")
        parts.append(
            f'<rect x="{legend_x}" y="{legend_y-9}" width="14" height="14" fill="{color}"/>'
        )
        parts.append(
            f'<text x="{legend_x+20}" y="{legend_y+2}" '
            f'font-family="Helvetica,Arial,sans-serif" font-size="13" fill="#222">{name}</text>'
        )
        legend_x += 110

    parts.append("</svg>")
    return "\n".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path, help="benchmark CSV path")
    parser.add_argument(
        "--outdir",
        type=Path,
        default=Path("results"),
        help="directory for SVG output",
    )
    args = parser.parse_args()

    rows = read_rows(args.csv)
    args.outdir.mkdir(parents=True, exist_ok=True)

    # Line charts for random arrangement
    time_series: dict[str, list[tuple[float, float]]] = defaultdict(list)
    cmp_series: dict[str, list[tuple[float, float]]] = defaultdict(list)
    for row in rows:
        if row["arrangement"] != "random":
            continue
        time_series[row["algorithm"]].append((row["size"], row["time_ms"]))
        cmp_series[row["algorithm"]].append((row["size"], float(row["comparisons"])))

    (args.outdir / "time_by_size_random.svg").write_text(
        svg_line_chart(
            "Median sort time vs size (random)",
            dict(time_series),
            "n",
            "time (ms)",
        ),
        encoding="utf-8",
    )
    (args.outdir / "comparisons_by_size_random.svg").write_text(
        svg_line_chart(
            "Comparisons vs size (random)",
            dict(cmp_series),
            "n",
            "comparisons",
        ),
        encoding="utf-8",
    )

    # Bar chart: time by arrangement at largest size present
    sizes = sorted({r["size"] for r in rows})
    focus_n = sizes[-1] if sizes else 0
    arrangements = []
    for r in rows:
        if r["size"] == focus_n and r["arrangement"] not in arrangements:
            arrangements.append(r["arrangement"])

    bar_series: dict[str, list[float]] = {a: [] for a in ALGOS}
    for arr in arrangements:
        for algo in ALGOS:
            match = next(
                (
                    r
                    for r in rows
                    if r["size"] == focus_n
                    and r["arrangement"] == arr
                    and r["algorithm"] == algo
                ),
                None,
            )
            bar_series[algo].append(match["time_ms"] if match else 0.0)

    (args.outdir / f"time_by_arrangement_n{focus_n}.svg").write_text(
        svg_grouped_bars(
            f"Median sort time by arrangement (n={focus_n})",
            arrangements,
            bar_series,
            "time (ms)",
        ),
        encoding="utf-8",
    )

    print(f"Wrote SVGs under {args.outdir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
